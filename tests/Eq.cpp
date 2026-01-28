
/*
 ==============================================================================
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

#include "helpers/TestHelpers.hpp"

#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>

TEST_CASE ("Diffuse/free field EQ: IR correctness", "[EQ]")
{
    using namespace juce;
    using namespace TestHelpers;

    constexpr auto sampleRate = 96000;
    constexpr auto blockSize = 1024;
    const auto dataPath = getTestDataPath();
    juce::File referencePath;

    AudioBuffer<float> buffer (2, blockSize);
    buffer.clear();
    AudioBuffer<float> reference (2, blockSize);
    MidiBuffer midiBuffer;

    auto proc = PolarDesignerAudioProcessor();
    auto& vts = proc.getValueTreeState();

    proc.prepareToPlay (sampleRate, blockSize);

    // configure processor
    SECTION ("Free field EQ, omni")
    {
        buffer.setSample (0, 0, 1.0f);

        auto* ffDfEqParam = vts.getParameter ("ffDfEq");
        ffDfEqParam->setValueNotifyingHost (ffDfEqParam->convertTo0to1 (1)); // free field

        vts.getParameter ("alpha1")->setValueNotifyingHost (0.0f); // omni
        vts.getParameter ("alpha2")->setValueNotifyingHost (0.0f);
        vts.getParameter ("alpha3")->setValueNotifyingHost (0.0f);
        vts.getParameter ("alpha4")->setValueNotifyingHost (0.0f);
        vts.getParameter ("alpha5")->setValueNotifyingHost (0.0f);

        referencePath = dataPath.getChildFile ("EQ_ff_omni_ir_96kHz.wav");
    }

    SECTION ("Free field EQ, eight")
    {
        buffer.setSample (0, 1, 1.0f);

        auto* ffDfEqParam = vts.getParameter ("ffDfEq");
        ffDfEqParam->setValueNotifyingHost (ffDfEqParam->convertTo0to1 (1)); // free field

        vts.getParameter ("alpha1")->setValueNotifyingHost (1.0f); // eight
        vts.getParameter ("alpha2")->setValueNotifyingHost (1.0f);
        vts.getParameter ("alpha3")->setValueNotifyingHost (1.0f);
        vts.getParameter ("alpha4")->setValueNotifyingHost (1.0f);
        vts.getParameter ("alpha5")->setValueNotifyingHost (1.0f);

        referencePath = dataPath.getChildFile ("EQ_ff_eight_ir_96kHz.wav");
    }

    SECTION ("Diffuse field EQ, omni")
    {
        buffer.setSample (0, 0, 1.0f);

        auto* ffDfEqParam = vts.getParameter ("ffDfEq");
        ffDfEqParam->setValueNotifyingHost (ffDfEqParam->convertTo0to1 (2)); // diffuse field

        vts.getParameter ("alpha1")->setValueNotifyingHost (0.0f); // omni
        vts.getParameter ("alpha2")->setValueNotifyingHost (0.0f);
        vts.getParameter ("alpha3")->setValueNotifyingHost (0.0f);
        vts.getParameter ("alpha4")->setValueNotifyingHost (0.0f);
        vts.getParameter ("alpha5")->setValueNotifyingHost (0.0f);

        referencePath = dataPath.getChildFile ("EQ_df_omni_ir_96kHz.wav");
    }

    SECTION ("Diffuse field EQ, eight")
    {
        buffer.setSample (0, 1, 1.0f);

        auto* ffDfEqParam = vts.getParameter ("ffDfEq");
        ffDfEqParam->setValueNotifyingHost (ffDfEqParam->convertTo0to1 (2)); // diffuse field

        vts.getParameter ("alpha1")->setValueNotifyingHost (1.0f); // eight
        vts.getParameter ("alpha2")->setValueNotifyingHost (1.0f);
        vts.getParameter ("alpha3")->setValueNotifyingHost (1.0f);
        vts.getParameter ("alpha4")->setValueNotifyingHost (1.0f);
        vts.getParameter ("alpha5")->setValueNotifyingHost (1.0f);

        referencePath = dataPath.getChildFile ("EQ_df_eight_ir_96kHz.wav");
    }

    proc.processBlock (buffer, midiBuffer);

    // writeWavFile (referencePath, buffer, sampleRate); // use this to update reference files
    loadWavFile (referencePath, reference);

    requireBuffersEqual (buffer, reference, 1e-6f);
    requireSignalPresent (buffer, 0.009f);
}

TEST_CASE ("Diffuse/free field EQ: correct latency", "[EQ]")
{
    PolarDesignerAudioProcessor proc;
    auto& vts = proc.getValueTreeState();

    auto* ffDfEqParam = vts.getParameter ("ffDfEq");
    ffDfEqParam->setValueNotifyingHost (ffDfEqParam->convertTo0to1 (1)); // free field

    SECTION ("44.1kHz EQ on")
    {
        proc.prepareToPlay (44100, 32);
        REQUIRE (proc.getLatencySamples() == 184 + 92);
    }

    SECTION ("48kHz EQ on")
    {
        proc.prepareToPlay (48000, 32);
        REQUIRE (proc.getLatencySamples() == 200);
    }

    SECTION ("96kHz EQ on")
    {
        proc.prepareToPlay (96000, 32);
        REQUIRE (proc.getLatencySamples() == 401 + 200);
    }
}
