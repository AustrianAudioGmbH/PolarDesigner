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
#pragma once

#include "../lookAndFeel/BinaryFonts.h"
#include "Colours.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{

enum class ButtonColor
{
    gray,
    red,
};

template <ButtonColor color>
class TextButton : public juce::TextButton
{
    static constexpr auto fillReduceMargin = 1.0f;
    static constexpr auto buttonFrameReduceMargin = 3.0f;
    static constexpr auto buttonFrameThickness = 2.0f;

public:
    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        using namespace juce;

        ignoreUnused (shouldDrawButtonAsDown);

        Colour backgroundColor;
        Colour frameColor;
        Colour hoverBackgroundColor;

        if constexpr (color == ButtonColor::gray)
        {
            backgroundColor = Colours::textButtonDefaultBackgroundColor;
            frameColor = Colours::textButtonHoverBackgroundColor;
            hoverBackgroundColor = Colours::textButtonHoverBackgroundColor;
        }
        else
        {
            backgroundColor = Colours::multiTextButtonBackgroundColor;
            frameColor = Colours::textButtonActiveRedFrameColor;
            hoverBackgroundColor = Colours::textButtonHoverRedBackgroundColor;
        }

        const auto buttonArea = getLocalBounds();

        g.setColour (Colours::textButtonFrameColor);
        g.drawRect (getLocalBounds());

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (hoverBackgroundColor);
            g.fillRect (buttonArea.reduced (fillReduceMargin));
        }
        else
        {
            g.setColour (backgroundColor);
            g.fillRect (buttonArea.reduced (fillReduceMargin));
        }

        const auto buttonText = getButtonText();

        g.setColour (Colours::mainTextColor);
        const auto font = Font (FontOptions (
            Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansSemiBold_ttf,
                                               BinaryFonts::NunitoSansSemiBold_ttfSize)));
        g.setFont (font);
        g.drawFittedText (buttonText, buttonArea, Justification::centred, 1);

        if (getToggleState())
        {
            g.setColour (frameColor);
            g.drawRect (buttonArea.reduced (buttonFrameReduceMargin), buttonFrameThickness);
        }
    }
};

template <ButtonColor color>
class MultiTextButtonComponent : public juce::TextButton
{
    static constexpr auto fillReduceMargin = 1.0f;
    static constexpr auto textReduceMargin = 6.0f;
    static constexpr auto buttonFrameReduceMargin = 3.0f;
    static constexpr auto buttonFrameThickness = 2.0f;

public:
    MultiTextButtonComponent()
    {
        setToggleable (true);
        setClickingTogglesState (true);
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        using namespace juce;

        ignoreUnused (shouldDrawButtonAsDown);

        Colour backgroundColor;
        Colour frameColor;
        Colour hoverBackgroundColor;

        if constexpr (color == ButtonColor::gray)
        {
            backgroundColor = Colours::textButtonDefaultBackgroundColor;
            frameColor = Colours::textButtonHoverBackgroundColor;
            hoverBackgroundColor = Colours::textButtonHoverBackgroundColor;
        }
        else
        {
            backgroundColor = Colours::multiTextButtonBackgroundColor;
            frameColor = Colours::textButtonActiveRedFrameColor;
            hoverBackgroundColor = Colours::textButtonHoverRedBackgroundColor;
        }

        const auto buttonArea = getLocalBounds();

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (hoverBackgroundColor);
            g.fillRect (buttonArea.reduced (fillReduceMargin));
        }
        else
        {
            g.setColour (backgroundColor);
            g.fillRect (buttonArea.reduced (fillReduceMargin));
        }

        const auto buttonText = getButtonText();

        g.setColour (getToggleState() ? Colours::mainTextColor : Colours::mainTextInactiveColor);
        const auto font = Font (FontOptions (normalFont));
        g.setFont (font);
        g.drawFittedText (buttonText,
                          buttonArea.reduced (textReduceMargin),
                          Justification::centred,
                          1);

        if (getToggleState())
        {
            g.setColour (frameColor);
            g.drawRect (buttonArea.reduced (buttonFrameReduceMargin), buttonFrameThickness);
        }
    }

private:
    juce::Typeface::Ptr normalFont =
        juce::Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansSemiBold_ttf,
                                                 BinaryFonts::NunitoSansSemiBold_ttfSize);
};

} // namespace AAGuiComponents
