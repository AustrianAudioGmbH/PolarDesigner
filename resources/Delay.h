 /*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include "BinaryData.h"

using namespace juce;

using namespace dsp;
class Delay : private ProcessorBase
{
public:

    Delay() {}

    ~Delay() override {}

    void setDelayTime (float delayTimeInSeconds)
    {
        if (delayTimeInSeconds <= 0.0f)
        {
            delay = 0.0f;
            bypassed = true;
        }
        else
        {
            delay = delayTimeInSeconds;
            bypassed = false;
        }

        prepare(spec);
    }

    unsigned int getDelayInSamples()
    {
        if (bypassed)
        {
            return 0;
        }
        else
        {
            return delayInSamples;
        }
    }

    void prepare (const ProcessSpec& specs) override
    {
        spec = specs;

        delayInSamples = static_cast<unsigned int> (roundToInt (delay * specs.sampleRate));

        buffer.setSize(static_cast<int> (specs.numChannels), static_cast<int> (specs.maximumBlockSize + delayInSamples));
        buffer.clear();
        writePosition = 0;
    }

    void process (const ProcessContextReplacing<float>& context) override
    {
        ScopedNoDenormals noDenormals;

        if (! bypassed)
        {
            auto abIn = context.getInputBlock();
            auto abOut = context.getOutputBlock();
            size_t L = abIn.getNumSamples();
            auto nCh = jmin((int) spec.numChannels, (int) abIn.getNumChannels());

            int startIndex, blockSize1, blockSize2;


            // write in delay line
            getReadWritePositions(false, (int) L, startIndex, blockSize1, blockSize2);

            for (int ch = 0; ch < nCh; ch++)
                buffer.copyFrom(ch, startIndex, abIn.getChannelPointer(static_cast<size_t> (ch)), blockSize1);

            if (blockSize2 > 0)
                for (int ch = 0; ch < nCh; ch++)
                    buffer.copyFrom(ch, 0, abIn.getChannelPointer(static_cast<size_t> (ch)) + blockSize1, blockSize2);


            // read from delay line
            getReadWritePositions(true, (int) L, startIndex, blockSize1, blockSize2);

            for (int ch = 0; ch < nCh; ch++)
                FloatVectorOperations::copy(abOut.getChannelPointer(static_cast<size_t> (ch)), buffer.getReadPointer(ch) + startIndex, blockSize1);

            if (blockSize2 > 0)
                for (int ch = 0; ch < nCh; ch++)
                    FloatVectorOperations::copy(abOut.getChannelPointer(static_cast<size_t> (ch)) + blockSize1, buffer.getReadPointer(ch), blockSize2);


            writePosition += L;
            writePosition = writePosition % static_cast<size_t>(buffer.getNumSamples());
        }
    }

    void reset() override
    {

    }

    void getReadWritePositions (bool read, int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
    {
        const size_t L = static_cast<size_t>(buffer.getNumSamples());
        size_t pos = writePosition;
        if (read)
        {
            pos = static_cast<size_t> (static_cast<unsigned int> (writePosition) - delayInSamples);
        }
        if (pos < 0)
            pos = pos + L;
        pos = pos % L;

        jassert(pos >= 0 && pos < L);

        if (numSamples <= 0)
        {
            startIndex = 0;
            blockSize1 = 0;
            blockSize2 = 0;
        }
        else
        {
            startIndex = static_cast<int>(pos);
            blockSize1 = jmin (static_cast<int>(L - pos), numSamples);
            numSamples -= blockSize1;
            blockSize2 = numSamples <= 0 ? 0 : numSamples;
        }
    }

private:
    //==============================================================================
    ProcessSpec spec = {-1, 0, 0};
    float delay;
    unsigned int delayInSamples = 0;
    bool bypassed = false;
    size_t writePosition = 0;
    AudioBuffer<float> buffer;
};
