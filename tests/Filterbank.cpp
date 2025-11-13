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

/* Make sure that all filters get updated when changing a crossover frequency to 
 * achieve perfect reconstruction
 */
TEST_CASE ("Crossover update", "[filterbank]")
{
    using namespace TestHelpers;

    constexpr auto sampleRate = 48000.0;
    constexpr auto bufferSize = 1024;
    constexpr auto numChannels = 2;

    const auto dataPath = getTestDataPath();
    juce::File referenceDataPath;

    juce::AudioBuffer<float> buffer (numChannels, bufferSize);
    juce::AudioBuffer<float> reference (numChannels, bufferSize);
    juce::MidiBuffer midiBuffer;

    // apply impulse to input
    buffer.clear();
    buffer.setSample (0, 0, 0.5f);
    buffer.setSample (1, 0, 0.5f);

    auto proc = PolarDesignerAudioProcessor();
    auto& vts = proc.getValueTreeState();

    proc.prepareToPlay (sampleRate, bufferSize);

    SECTION ("2-band filterbank with 200Hz crossover")
    {
        vts.getParameter ("nrBands")->setValueNotifyingHost (2.0f);
        vts.getParameter ("xOverF1")->setValueNotifyingHost (200.0f);
        referenceDataPath = dataPath.getChildFile ("Filterbank_2-band-200Hz.wav");
    }

    SECTION ("3-band filterbank with 300Hz and 2500Hz crossovers")
    {
        vts.getParameter ("nrBands")->setValueNotifyingHost (3.0f);
        vts.getParameter ("xOverF1")->setValueNotifyingHost (300.0f);
        vts.getParameter ("xOverF2")->setValueNotifyingHost (2500.0f);
        referenceDataPath = dataPath.getChildFile ("Filterbank_3-band-300Hz-2500Hz.wav");
    }

    SECTION ("4-band filterbank with 300Hz, 1000Hz and 5000Hz crossovers")
    {
        vts.getParameter ("nrBands")->setValueNotifyingHost (4.0f);
        vts.getParameter ("xOverF1")->setValueNotifyingHost (300.0f);
        vts.getParameter ("xOverF2")->setValueNotifyingHost (1000.0f);
        vts.getParameter ("xOverF3")->setValueNotifyingHost (5000.0f);
        referenceDataPath = dataPath.getChildFile ("Filterbank_4-band-300Hz-1000Hz-5000Hz.wav");
    }

    SECTION ("5-band filterbank with 200Hz, 600Hz, 2500Hz and 8000Hz crossovers")
    {
        vts.getParameter ("nrBands")->setValueNotifyingHost (5.0f);
        vts.getParameter ("xOverF1")->setValueNotifyingHost (200.0f);
        vts.getParameter ("xOverF2")->setValueNotifyingHost (600.0f);
        vts.getParameter ("xOverF3")->setValueNotifyingHost (2500.0f);
        vts.getParameter ("xOverF4")->setValueNotifyingHost (8000.0f);
        referenceDataPath =
            dataPath.getChildFile ("Filterbank_5-band-200Hz-600Hz-2500Hz-8000Hz.wav");
    }

    proc.processBlock (buffer, midiBuffer);

    // use this to update reference data
    // writeWavFile (referenceDataPath, buffer, sampleRate);

    loadWavFile (referenceDataPath, reference);
    requireBuffersEqual (buffer, reference, 1e-6f);
    requireSignalPresent (buffer, 0.03f);
}
