/*
  ==============================================================================

    GainSlider.h
    Created: 13 Jul 2023 1:09:17pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainSlider)
};
