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
    const Colour multiTextButtonBackgroundColor = Colour(31, 32, 38);
    const Colour groupComponentBackgroundColor = Colour(28, 30, 33);
    const Colour textButtonDefaultBackgroundColor = Colour(24, 25, 27);
    const Colour textButtonHoverBackgroundColor = Colour(Colours::white.withAlpha(0.3f));
    const Colour textButtonPressedBackgroundColor = Colour(Colours::white.withAlpha(0.1f));
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

        auto loadArrowImg = juce::Drawable::createFromImageData(BinaryData::loadArrow_svg, BinaryData::loadArrow_svgSize);

        if (button.getButtonText() == "Zero latency")
        {
            g.setColour(textButtonActiveFrameColor);
            g.drawRect(buttonArea, 1);
            
            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(buttonArea.reduced(1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(buttonArea.reduced(1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Load")
        {
            g.setColour(textButtonFrameColor);
            g.drawRect(buttonArea, 1);

            auto arrowArea = buttonArea.reduced(button.proportionOfWidth(0.45f), button.proportionOfHeight(0.33f)).translated(button.proportionOfWidth(0.36f), 0);

            loadArrowImg->drawWithin(g, arrowArea, juce::RectanglePlacement::centred, 1.f);

            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(buttonArea.reduced(1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(buttonArea.reduced(1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Save")
        {
            g.setColour(textButtonFrameColor);
            g.drawRect(buttonArea, 1);

            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(buttonArea.reduced(1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(buttonArea.reduced(1.0f, 1.0f));
            }
        }
        else
        {
            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(textButtonActiveFrameColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 1);
            }
        }
    }

    void drawButtonText(Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        Rectangle<int> buttonArea(0, 0, button.getWidth(), button.getHeight());

        g.setColour(mainTextColor);

        int x = button.proportionOfWidth(0.18f);
        int y = button.proportionOfHeight(0.24f);
        int w = button.proportionOfWidth(0.47f);
        int h = button.proportionOfHeight(0.55f);

        Font font(normalFont);
        font.setHeight(h);
        g.setFont(font);

        if (button.getButtonText() == "Load")
        {
           g.drawFittedText(button.getButtonText(), x, y, w, h, Justification::centred, 1);
        }
        else
        {
           g.drawFittedText(button.getButtonText(), buttonArea, Justification::centred, 1);
        }
    }

    void drawGroupComponentOutline(Graphics& g, int width, int height,
        const String& text, const Justification& position,
        GroupComponent& group) override
    {
        Rectangle<float> groupArea(0, 0, group.getWidth(), group.getHeight());
        g.setColour(groupComponentBackgroundColor);

        juce::Path path;
        path.addRoundedRectangle(groupArea, 10.f, 10.f);
        path.closeSubPath();
        g.fillPath(path);

        g.setColour(mainTextColor);

        int x = group.proportionOfWidth(0.06f);
        int y = group.proportionOfHeight(0.136f);
        int w = group.proportionOfWidth(0.87f);
        int h = group.proportionOfHeight(0.2f);

        Font font(normalFont);
        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(text, x, y, w, h, Justification::left, 1);
    }
};