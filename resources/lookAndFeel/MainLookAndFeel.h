/*
  ==============================================================================

    MainLookAndFeel.h
    Created: 30 May 2023 4:54:48pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/

#pragma once

class MainLookAndFeel : public LookAndFeel_V4
{
public:
    const Colour mainBackground = Colour(24, 25, 27);
    const Colour mainTextColor = Colour(255, 255, 255);
    const Colour textButtonBackgroundColor = Colour(31, 32, 38);
    const Colour textButtonFrameColor = Colour(52, 54, 57);
    const Colour textButtonActiveFrameColor = Colour(255, 255, 255);

    Typeface::Ptr normalFont;

    MainLookAndFeel()
    {
        normalFont = Typeface::createSystemTypefaceFor(BinaryFonts::NunitoSansSemiBold_ttf, BinaryFonts::NunitoSansSemiBold_ttfSize);
    }

    ~MainLookAndFeel() {}

    void drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown) override
    {
        Rectangle<float> buttonArea(0.0f, 0.0f, button.getWidth(), button.getHeight());
        buttonArea.reduce(3.0f, 3.0f);
        if (button.getToggleState() == true)
        {
            g.setColour(textButtonActiveFrameColor);
            g.drawRect(buttonArea, 1);
        }
    }
};