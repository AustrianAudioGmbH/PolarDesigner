/*
  ==============================================================================

    GainSlider.h
    Created: 13 Jul 2023 1:09:17pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
class GainSlider : public juce::Slider
{
public:
    GainSlider() {}

    ~GainSlider() override {}

    bool hitTest (int x, int y) override
    {
        auto rect = getLookAndFeel().getSliderLayout (*this).sliderBounds;
        return rect.contains (x, y);
    }

    void mouseMove (const juce::MouseEvent& event) override
    {
        (void) event;
        setTextBoxStyle (Slider::TextBoxLeft, true, -1, -1);
        repaint();
    }

    void mouseExit (const juce::MouseEvent& event) override { (void) event; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainSlider)
};
