/*
 ==============================================================================
 TestHelpers.hpp
 Author: Sebastian Grill

 Copyright (c) 2025 - Austrian Audio GmbH
 www.austrian.audio

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <memory>

namespace TestHelpers
{
static inline juce::File getTestDataPath()
{
    return juce::File (POLARDESIGNER_ROOT_PATH).getChildFile ("tests/data");
}

static inline void loadWavFile (const juce::File& file, juce::AudioBuffer<float>& buffer)
{
    using namespace juce;

    if (! (file.existsAsFile() && file.hasFileExtension (".wav")))
    {
        throw std::runtime_error ("File does not exist or is not a valid WAV file: "
                                  + file.getFullPathName().toStdString());
    }

    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (reader == nullptr)
    {
        throw std::runtime_error ("Could not create reader for file: "
                                  + file.getFullPathName().toStdString());
    }

    buffer.setSize (static_cast<int> (reader->numChannels),
                    static_cast<int> (reader->lengthInSamples));
    reader->read (&buffer, 0, static_cast<int> (reader->lengthInSamples), 0, true, true);
}

static inline void
    writeWavFile (const juce::File& file, const juce::AudioBuffer<float>& buffer, float sampleRate)
{
    using namespace juce;

    WavAudioFormat wavFormat;

    std::unique_ptr<AudioFormatWriter> writer;
    std::unique_ptr<OutputStream> stream (file.createOutputStream());

    if (! stream)
    {
        throw std::runtime_error ("Could not open file for writing: "
                                  + file.getFullPathName().toStdString());
    }

    stream->setPosition (0);

    auto options = AudioFormatWriterOptions()
                       .withSampleRate (sampleRate)
                       .withNumChannels (buffer.getNumChannels())
                       .withBitsPerSample (24)
                       .withMetadataValues ({})
                       .withQualityOptionIndex (0);

    auto newWriter = wavFormat.createWriterFor (stream, options);

    writer = std::move (newWriter);

    if (writer == nullptr)
    {
        throw std::runtime_error ("Could not create writer for file: "
                                  + file.getFullPathName().toStdString());
    }

    writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
}

static inline void requireBuffersEqual (const juce::AudioBuffer<float>& buffer,
                                        const juce::AudioBuffer<float>& other,
                                        float absTol = 1e-9f)
{
    using namespace Catch::Matchers;

    REQUIRE (buffer.getNumChannels() == other.getNumChannels());
    REQUIRE (buffer.getNumSamples() == other.getNumSamples());

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto val1 = buffer.getSample (ch, i);
            const auto val2 = other.getSample (ch, i);
            REQUIRE_THAT (val1, WithinAbs (val2, absTol) || WithinRel (val2));
        }
}

static inline void requireSignalPresent (const juce::AudioBuffer<float>& buffer,
                                         float rmsThreshold = 0.1f)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const auto rms = buffer.getRMSLevel (ch, 0, buffer.getNumSamples());
        REQUIRE (rms >= rmsThreshold);
    }
}

static inline void
    copySamples (float* const* target, const float* const* source, int numChannels, int numSamples)
{
    for (auto ch = 0; ch < numChannels; ++ch)
        for (auto i = 0; i < numSamples; ++i)
            target[ch][i] = source[ch][i];
}

static inline void
    addSamples (float* const* target, const float* const* source, int numChannels, int numSamples)
{
    for (auto ch = 0; ch < numChannels; ++ch)
        for (auto i = 0; i < numSamples; ++i)
            target[ch][i] += source[ch][i];
}
} // namespace TestHelpers
