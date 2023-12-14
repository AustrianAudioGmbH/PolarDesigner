/*
  ==============================================================================

    GainSlider.h
    Created: 13 Jul 2023 1:09:17pm
    Author:  Mikolaj Cikuj

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


//==============================================================================
/*
*/
class GainSlider  : public Slider
{
public:
    GainSlider()
    {
    }

    ~GainSlider() override
    {
    }

    bool hitTest(int x, int y) override
    {
        auto rect = getLookAndFeel().getSliderLayout(*this).sliderBounds;
        return rect.contains(x, y);
    }

    void mouseMove(const MouseEvent& event) override
    {
        (void)event;
        setTextBoxStyle(Slider::TextBoxLeft, true, -1, -1);
        repaint();
    }

    void mouseExit(const MouseEvent& event) override
    {
        (void)event;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainSlider)
};
