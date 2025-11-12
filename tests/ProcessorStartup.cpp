/*
 ==============================================================================
 ProcessorStartup.cpp
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

#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>

/* This is a simple smoke test to make sure nothing segfaults when things happen before
 * prepareToPlay is called
 */

TEST_CASE ("Processor startup", "[startup]")
{
    juce::AudioBuffer<float> buffer (2, 1024);
    juce::MidiBuffer midiBuffer;

    auto proc = PolarDesignerAudioProcessor();
    auto& vts = proc.getValueTreeState();

    // FIX:uncommenting this causes a deadlock on atmoky macrunner1 when run through
    // ci but not when run manually, suggesting this is a very specific race condition.
    // Needs further investigation
    // [[maybe_unused]] auto editor = proc.createEditor();

    vts.getParameter ("zeroLatencyMode")->setValueNotifyingHost (0.0f);
    proc.changeABLayerState (COMPARE_LAYER_B);
    vts.getParameter ("zeroLatencyMode")->setValueNotifyingHost (1.0f);
    proc.changeABLayerState (COMPARE_LAYER_A);
    vts.getParameter ("zeroLatencyMode")->setValueNotifyingHost (0.0f);
    proc.changeABLayerState (COMPARE_LAYER_B);
    vts.getParameter ("zeroLatencyMode")->setValueNotifyingHost (0.0f);

    proc.prepareToPlay (96000.0, 64);

    vts.getParameter ("zeroLatencyMode")->setValueNotifyingHost (0.0f);

    proc.prepareToPlay (44100.0, 256);

    vts.getParameter ("zeroLatencyMode")->setValueNotifyingHost (0.0f);

    proc.prepareToPlay (48000.0, 1024);

    proc.processBlock (buffer, midiBuffer);
}
