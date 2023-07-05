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
    const Colour mainTextDisabledColor = mainTextColor.withAlpha(0.4f);
    const Colour mainTextInactiveColor = mainTextColor.withAlpha(0.7f);
    const Colour textButtonFrameColor = Colour(52, 54, 57);
    const Colour labelBackgroundColor = Colour(39, 39, 44);
    const Colour multiTextButtonBackgroundColor = Colour(31, 32, 38);
    const Colour groupComponentBackgroundColor = Colour(28, 30, 33);
    const Colour textButtonDefaultBackgroundColor = Colour(24, 25, 27);
    const Colour textButtonActiveFrameColor = Colour(255, 255, 255);
    const Colour textButtonActiveRedFrameColor = Colour(182, 22, 22);
    const Colour textButtonActiveBlue1FrameColor = Colour(0, 49, 222);
    const Colour textButtonActiveBlue2FrameColor = Colour(0, 127, 255);
    const Colour textButtonActiveBlue3FrameColor = Colour(0, 87, 255);
    const Colour textButtonActiveBlue4FrameColor = Colour(108, 180, 238);
    const Colour sliderHoverFrameColor = Colour(93, 94, 95);
    const Colour polarVisualizerRed = Colour(182, 22, 22);
    const Colour polarVisualizerOrange = Colour(255, 107, 0);
    const Colour polarVisualizerYellow = Colour(214, 193, 9);
    const Colour polarVisualizerGreen = Colour(96, 154, 0);
    const Colour polarVisualizerGreenDark = Colour(0, 157, 25);
    const Colour polarVisualizerGrid = Colour(52, 54, 57);
    const Colour textButtonHoverBackgroundColor = textButtonActiveFrameColor.withAlpha(0.3f);
    const Colour textButtonHoverRedBackgroundColor = textButtonActiveRedFrameColor.withAlpha(0.3f);
    const Colour textButtonPressedBackgroundColor = textButtonActiveFrameColor.withAlpha(0.1f);
    const Colour textButtonPressedRedBackgroundColor = textButtonActiveRedFrameColor.withAlpha(0.1f);
    const Colour toggleButtonActiveRedBackgroundColor = Colour(182, 22, 22).withAlpha(0.7f);

    Typeface::Ptr normalFont;
    Typeface::Ptr terminatorRegularFont, terminatorBoldFont;

    MainLookAndFeel()
    {
        normalFont = Typeface::createSystemTypefaceFor(BinaryFonts::NunitoSansSemiBold_ttf, BinaryFonts::NunitoSansSemiBold_ttfSize);
        terminatorRegularFont = Typeface::createSystemTypefaceFor(BinaryFonts::InterRegular_ttf, BinaryFonts::InterRegular_ttfSize);
        terminatorBoldFont = Typeface::createSystemTypefaceFor(BinaryFonts::InterBold_ttf, BinaryFonts::InterBold_ttfSize);

        setColour(ListBox::backgroundColourId, groupComponentBackgroundColor);
    }

    ~MainLookAndFeel() {}

    Typeface::Ptr getTypefaceForFont(const Font& f) override
    {
        switch (f.getStyleFlags()) {
        case 1: return normalFont;
        default: return normalFont;
        }
    }

    void drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown) override
    {
        Rectangle<float> buttonArea(0.0f, 0.0f, button.getWidth(), button.getHeight());

        auto loadArrowImg = juce::Drawable::createFromImageData(BinaryData::loadArrow_svg, BinaryData::loadArrow_svgSize);
        auto freeFieldImg = juce::Drawable::createFromImageData(BinaryData::freeField_svg, BinaryData::freeField_svgSize);
        auto diffuseFieldImg = juce::Drawable::createFromImageData(BinaryData::diffuseField_svg, BinaryData::diffuseField_svgSize);
        auto eqFieldCheckSign = juce::Drawable::createFromImageData(BinaryData::eqFieldCheckSign_svg, BinaryData::eqFieldCheckSign_svgSize);
        auto terminateSpillIconImg = juce::Drawable::createFromImageData(BinaryData::terminateSpillIcon_svg, BinaryData::terminateSpillIcon_svgSize);
        auto maximizeTargetIconImg = juce::Drawable::createFromImageData(BinaryData::maximizeTargetIcon_svg, BinaryData::maximizeTargetIcon_svgSize);
        auto maxTargetToSpillIconImg = juce::Drawable::createFromImageData(BinaryData::maxTargetToSpillIcon_svg, BinaryData::maximizeTargetIcon_svgSize);
        auto closePresetListIconImg = juce::Drawable::createFromImageData(BinaryData::closePresetListIcon_svg, BinaryData::closePresetListIcon_svgSize);

        if (button.getButtonText() == "Zero latency")
        {
            g.setColour(button.isEnabled() ? textButtonActiveFrameColor : textButtonHoverBackgroundColor);
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

            if (!button.isEnabled())
            {
                bool result = loadArrowImg->replaceColour(Colours::white, mainTextDisabledColor);
                if (!result)
                    return;
            }

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
        else if (button.getComponentID() == "5621" || button.getComponentID() == "5721")
        {
            auto imageArea = buttonArea.reduced(5, 5);

            closePresetListIconImg->drawWithin(g, imageArea, juce::RectanglePlacement::centred, 1.f);

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
        else if (button.getButtonText() == "Open from file")
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
        else if (button.getButtonText() == "Free Field")
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

            if (!button.isEnabled())
            {
                bool resultMainImg = freeFieldImg->replaceColour(Colours::white, mainTextDisabledColor);
                if (!resultMainImg)
                    return;
                bool resultSignOutImg = eqFieldCheckSign->replaceColour(Colours::white, mainTextDisabledColor);
                if (!resultSignOutImg)
                    return;
                bool resultSignInImg = eqFieldCheckSign->replaceColour(textButtonActiveRedFrameColor, textButtonHoverRedBackgroundColor);
                if (!resultSignInImg)
                    return;
            }

            auto freeFieldImageArea = buttonArea;
            freeFieldImageArea.removeFromTop(button.proportionOfHeight(0.11f));
            freeFieldImageArea.removeFromBottom(button.proportionOfHeight(0.33f));
            freeFieldImg->drawWithin(g, freeFieldImageArea, juce::RectanglePlacement::centred, 1.f);

            if (button.getToggleState() == true)
            {
                auto eqFieldCheckSignArea = juce::Rectangle<float>(button.getWidth() * 0.77f,
                    button.getHeight() * 0.05f, button.getWidth() * 0.18f, button.getWidth() * 0.18f);
                eqFieldCheckSign->drawWithin(g, eqFieldCheckSignArea, juce::RectanglePlacement::centred, 1.f);
            }
        }
        else if (button.getButtonText() == "Diffuse Field")
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
            
            if (!button.isEnabled())
            {
                bool resultMainImg = diffuseFieldImg->replaceColour(Colours::white, mainTextDisabledColor);
                if (!resultMainImg)
                    return;
                bool resultSignOutImg = eqFieldCheckSign->replaceColour(Colours::white, mainTextDisabledColor);
                if (!resultSignOutImg)
                    return;
                bool resultSignInImg = eqFieldCheckSign->replaceColour(textButtonActiveRedFrameColor, textButtonHoverRedBackgroundColor);
                if (!resultSignInImg)
                    return;
            }

            auto diffuseFieldImageArea = buttonArea;
            diffuseFieldImageArea.removeFromTop(button.proportionOfHeight(0.11f));
            diffuseFieldImageArea.removeFromBottom(button.proportionOfHeight(0.33f));
            diffuseFieldImg->drawWithin(g, diffuseFieldImageArea, juce::RectanglePlacement::centred, 1.f);
            
            if (button.getToggleState() == true)
            {
                auto eqFieldCheckSignArea = juce::Rectangle<float>(button.getWidth()*0.77f,
                    button.getHeight() * 0.05f, button.getWidth()*0.18f, button.getWidth() * 0.18f);
                eqFieldCheckSign->drawWithin(g, eqFieldCheckSignArea, juce::RectanglePlacement::centred, 1.f);
            }
        }
        else if (button.getButtonText() == "Terminate Spill")
        {
            g.setColour(textButtonFrameColor);
            auto reducedRect = buttonArea.reduced(4);
            g.drawRect(reducedRect, 1);

            auto iconArea = buttonArea.reduced(button.proportionOfWidth(0.45f), button.proportionOfHeight(0.33f)).translated(button.proportionOfWidth(0.38f), 0);

            terminateSpillIconImg->drawWithin(g, iconArea, juce::RectanglePlacement::centred, 1.f);

            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(reducedRect.reduced(1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(reducedRect.reduced(1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Maximize Target")
        {
            g.setColour(textButtonFrameColor);
            auto reducedRect = buttonArea.reduced(4);
            g.drawRect(reducedRect, 1);

            auto iconArea = buttonArea.reduced(button.proportionOfWidth(0.45f), button.proportionOfHeight(0.33f)).translated(button.proportionOfWidth(0.38f), 0);
             
            maximizeTargetIconImg->drawWithin(g, iconArea, juce::RectanglePlacement::centred, 1.f);
             
            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(reducedRect.reduced(1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(reducedRect.reduced(1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Max Target-to-spill")
        {
            g.setColour(textButtonFrameColor);
            auto reducedRect = buttonArea.reduced(4);
            g.drawRect(reducedRect, 1);

            auto iconArea = buttonArea.reduced(button.proportionOfWidth(0.45f), button.proportionOfHeight(0.33f)).translated(button.proportionOfWidth(0.38f), 0);

            maxTargetToSpillIconImg->drawWithin(g, iconArea, juce::RectanglePlacement::centred, 1.f);

            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverBackgroundColor);
                g.fillRect(reducedRect.reduced(1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedBackgroundColor);
                g.fillRect(reducedRect.reduced(1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Terminate spill" ||
                 button.getButtonText() == "Maximize target" ||
                 button.getButtonText() == "Max Target-to-Spill")
        {
        }
        else if (button.getButtonText().contains("TermLine"))
        {
            Line<float> line(buttonArea.getCentreX(), 0, buttonArea.getCentreX(), buttonArea.getHeight());
            Path linePath;
            linePath.addLineSegment(line, buttonArea.proportionOfWidth(0.06f));
            g.setColour(textButtonPressedBackgroundColor);
            g.fillPath(linePath);
        }
        else if (button.getButtonText() == "01" ||
        button.getButtonText() == "02" ||
        button.getButtonText() == "03")
        {
            if (button.isEnabled())
            {
                Path path;
                auto reducedButtonArea = buttonArea.reduced(1);
                auto ellipseArea = reducedButtonArea;
                ellipseArea.setWidth(jmin(reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setHeight(jmin(reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setCentre(buttonArea.getCentre());
                path.addEllipse(ellipseArea);
                g.setColour(toggleButtonActiveRedBackgroundColor);
                g.fillPath(path);
                g.setColour(textButtonActiveRedFrameColor);
                g.strokePath(path, PathStrokeType(ellipseArea.proportionOfWidth(0.06f)));
            }
            else
            {
                Path path;
                auto reducedButtonArea = buttonArea.reduced(1);
                auto ellipseArea = reducedButtonArea;
                ellipseArea.setWidth(jmin(reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setHeight(jmin(reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setCentre(buttonArea.getCentre());
                path.addEllipse(ellipseArea);
                
                auto color = textButtonPressedBackgroundColor;
                if (button.getToggleState())
                {
                    //Draw check sign when terminator stage completed
                    Line<float> line1(Point(ellipseArea.getX() + ellipseArea.proportionOfWidth(0.24f), 
                        ellipseArea.getY() + ellipseArea.proportionOfHeight(0.47f)), 
                        Point(ellipseArea.getX() + ellipseArea.proportionOfWidth(0.41f), 
                            ellipseArea.getY() + ellipseArea.proportionOfHeight(0.67f)));

                    Line<float> line2(line1.getEnd(),
                        Point(ellipseArea.getX() + ellipseArea.proportionOfWidth(0.72f),
                            ellipseArea.getY() + ellipseArea.proportionOfHeight(0.26f)));

                    Path signPath;
                    signPath.addLineSegment(line1, ellipseArea.proportionOfWidth(0.06f));
                    signPath.addLineSegment(line2, ellipseArea.proportionOfWidth(0.06f));

                    color = textButtonActiveRedFrameColor.withAlpha(0.15f);
                    g.setColour(color);
                    g.fillPath(path);;
                    g.setColour(textButtonActiveRedFrameColor);
                    g.strokePath(path, PathStrokeType(ellipseArea.proportionOfWidth(0.06f)));

                    g.setColour(mainTextColor);
                    g.fillPath(signPath);
                }
                else
                {
                    g.setColour(color);
                    g.fillPath(path);;
                }
            }
        }
        else if (button.getButtonText() == "Click on the button below to apply polar\npatterns with minimum spill energy"
              || button.getButtonText() == "Click on the button below to apply polar\npatterns with maximum signal energy"
              || button.getButtonText() == "Find best compromise between reduction\nof spill and maximizing target signal")
        {
        }
        else if (button.getButtonText() == "Begin Terminate"
            || button.getButtonText() == "Begin Maximize"
            || button.getButtonText() == "Apply Max Target-to-Spill"
            )
        {
            g.setColour(mainTextColor);
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
        //Sync channel buttons
        else if (button.getComponentID() == "5521")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue1FrameColor : textButtonActiveBlue1FrameColor.withAlpha(0.5f);
            if (isMouseOverButton)
            {
                g.setColour(mainColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(mainColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        else if(button.getComponentID() == "5522")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue2FrameColor : textButtonActiveBlue2FrameColor.withAlpha(0.5f);
            if (isMouseOverButton)
            {
                g.setColour(mainColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(mainColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5523")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue3FrameColor : textButtonActiveBlue3FrameColor.withAlpha(0.5f);
            if (isMouseOverButton)
            {
                g.setColour(mainColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(mainColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5524")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue4FrameColor : textButtonActiveBlue4FrameColor.withAlpha(0.5f);
            if (isMouseOverButton)
            {
                g.setColour(mainColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(mainColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(mainColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        //Close preset and close terminator buttons
        else if (button.getComponentID() == "5621" || button.getComponentID() == "5721")
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
        }
        //Nr of bands buttons
        else
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue4FrameColor : textButtonActiveBlue4FrameColor.withAlpha(0.5f);
            if (isMouseOverButton)
            {
                g.setColour(textButtonHoverRedBackgroundColor);
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonPressedRedBackgroundColor);
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(textButtonPressedRedBackgroundColor);
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(button.isEnabled() ? textButtonActiveRedFrameColor : textButtonActiveRedFrameColor.withAlpha(0.5f));
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
    }

    void drawButtonText(Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        Rectangle<int> buttonArea(0, 0, button.getWidth(), button.getHeight());

        g.setColour(button.isEnabled() ? mainTextColor : mainTextDisabledColor);

        Font font(normalFont);

        String text = button.getButtonText();

        int x = buttonArea.getX();
        int w = buttonArea.getWidth();
        int h = button.getTopLevelComponent()->getHeight() * 0.023f;
        int y = (buttonArea.getHeight() - h) / 2;

        auto justification = Justification::centred;

        if (button.getButtonText() == "Load")
        {
            x = buttonArea.proportionOfWidth(0.18f);
            w = buttonArea.proportionOfWidth(0.47f);
        }
        else if (button.getButtonText() == "Free Field")
        {
            y = buttonArea.proportionOfHeight(0.7f);
            h = button.getTopLevelComponent()->getHeight() * 0.02f;
        }
        else if (button.getButtonText() == "Diffuse Field")
        {
            y = buttonArea.proportionOfHeight(0.7f);
            h = button.getTopLevelComponent()->getHeight() * 0.018f;
        }
        else if (button.getButtonText() == "Terminate Spill"
            || button.getButtonText() == "Maximize Target"
            || button.getButtonText() == "Max Target-to-spill"
            )
        {
            justification = Justification::centredLeft;
            x = buttonArea.proportionOfWidth(0.11f);
            w = buttonArea.proportionOfWidth(0.78f);
        }
        else if (button.getButtonText() == "Begin Terminate"
            || button.getButtonText() == "Begin Maximize"
            || button.getButtonText() == "Apply Max Target-to-Spill"
            )
        {
            justification = Justification::centredLeft;
            x = buttonArea.proportionOfWidth(0.11f);
            w = buttonArea.proportionOfWidth(0.78f);
        }
        else if (button.getButtonText() == "Terminate spill" ||
            button.getButtonText() == "Maximize target" ||
            button.getButtonText() == "Max Target-to-Spill")
        {
            g.setColour(button.isEnabled() ? mainTextColor : mainTextInactiveColor);
            font = button.isEnabled() ? terminatorBoldFont : terminatorRegularFont;
            justification = Justification::centredLeft;
            h = button.getTopLevelComponent()->getHeight() * 0.015f;
            y = (buttonArea.getHeight() - h) / 2;
        }
        else if (button.getButtonText() == "01" ||
            button.getButtonText() == "02" ||
            button.getButtonText() == "03"
            )
        {
            if (button.getToggleState())
                return;

            g.setColour(button.isEnabled() ? mainTextColor : mainTextInactiveColor);
            String tmpText("");
            tmpText += button.getButtonText().getCharPointer()[1];
            text = tmpText;
            font = button.isEnabled() ? terminatorBoldFont : terminatorRegularFont;
            justification = Justification::centred;
            h = button.getTopLevelComponent()->getHeight() * 0.014f;
            y = (buttonArea.getHeight() - h) / 2;
        }
        else if (button.getButtonText().contains("TermLine"))
        {
            return;
        }
        else if (button.getButtonText() == "Click on the button below to apply polar\npatterns with minimum spill energy"
            || button.getButtonText() == "Click on the button below to apply polar\npatterns with maximum signal energy"
            || button.getButtonText() == "Find best compromise between reduction\nof spill and maximizing target signal"
            )
        {
            g.setColour(button.isEnabled() ? mainTextColor : mainTextInactiveColor);
            font = terminatorRegularFont;
            justification = Justification::centredLeft;
            h = button.getTopLevelComponent()->getHeight() * 0.014f;
            w = buttonArea.proportionOfWidth(1.f);
        }

        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(text, x, y, w, h, justification, 1);
    }

    void drawGroupComponentOutline(Graphics& g, int width, int height,
        const String& text, const Justification& position,
        GroupComponent& group) override
    {
        Rectangle<float> groupArea(0, 0, group.getWidth(), group.getHeight());
        g.setColour(groupComponentBackgroundColor);

        juce::Path path;
        if (text == "Preset")
        {
            path.addRectangle(groupArea);
        }
        else
        {
            path.addRoundedRectangle(groupArea, 10.f, 10.f);
        }

        path.closeSubPath();
        g.fillPath(path);

        g.setColour(group.isEnabled() ? mainTextColor : mainTextDisabledColor);

        int x = group.proportionOfWidth(0.06f);
        int y = 10.f;
        int w = group.proportionOfWidth(0.87f);
        int h = group.getTopLevelComponent()->getHeight() * 0.025f;

        if (text == "Preset")
        {
            y = group.proportionOfHeight(0.04f);
        }

        Font font(normalFont);
        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(text, x, y, w, h, Justification::left, 1);
    }

    Slider::SliderLayout getSliderLayout(Slider& slider) override
    {
        Rectangle<int> localBounds(0, 0, slider.getWidth(), slider.getHeight());
        Slider::SliderLayout layout;

        if (slider.getSliderStyle() == Slider::SliderStyle::LinearHorizontal)
        {
            layout.sliderBounds.setBounds(localBounds.getWidth() * 0.07f, localBounds.getY(), localBounds.getWidth() * 0.42f, localBounds.getHeight());
            layout.textBoxBounds.setBounds(localBounds.getWidth() * 0.71f, localBounds.getY(), localBounds.getWidth() * 0.29f, localBounds.getHeight());
            layout.textBoxBounds.reduce(10, 10);
        }
        else if (slider.getSliderStyle() == Slider::SliderStyle::LinearVertical)
        {
            layout.sliderBounds.setBounds(localBounds.getX(), localBounds.getHeight() * 0.1f, localBounds.getWidth(), localBounds.getHeight()*0.8f);
            layout.textBoxBounds.setBounds(localBounds.getWidth() * 0.52f, localBounds.getHeight()*0.3f, slider.getTopLevelComponent()->getWidth() * 0.065f, slider.getTopLevelComponent()->getHeight() * 0.05f);
            layout.textBoxBounds.reduce(10, 10);
        }

        return layout;
    }

    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider) override
    {
        drawLinearSliderBackground(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }

    void drawLinearSliderBackground(Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider) override
    {
        const float h = slider.getTopLevelComponent()->getHeight() * 0.005f;

        Path pathBgr;
        Path pathFrg;
        auto pathBgrColor = slider.isEnabled() ? textButtonFrameColor : textButtonFrameColor.withAlpha(0.4f);
        auto pathFrgColor = slider.isEnabled() ? textButtonActiveRedFrameColor : textButtonActiveRedFrameColor.withAlpha(0.2f);

        if (style == Slider::SliderStyle::LinearHorizontal)
        {
            Rectangle<float> backgroundRect(x, 0.5f * height - h / 2, width, h);
            pathBgr.addRoundedRectangle(backgroundRect, h);
            Rectangle<float> foregroundRect(width / 2 + x, 0.5f * height - h / 2, sliderPos - (width / 2 + x), h);
            pathFrg.addRectangle(foregroundRect);
        }
        else if (style == Slider::SliderStyle::LinearVertical)
        {
            Rectangle<float> backgroundRect(0.5f * width - h/2, y, h, height);
            pathBgr.addRoundedRectangle(backgroundRect, h);
            Rectangle<float> foregroundRect(0.5f * width - h / 2, y + height * 0.43f, h, sliderPos - (y + height * 0.43f));
            pathFrg.addRectangle(foregroundRect);
        }
        g.setColour(pathBgrColor);
        g.fillPath(pathBgr);
        g.setColour(pathFrgColor);
        g.fillPath(pathFrg);
    }

    void drawLinearSliderThumb(Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider) override
    {
        const float newDiameter = slider.getTopLevelComponent()->getHeight() * 0.024f;

        Path p;
        if (style == Slider::SliderStyle::LinearHorizontal)
        {
            p.addEllipse(sliderPos - newDiameter / 2, height / 2.f - newDiameter / 2.f, newDiameter, newDiameter);
        }
        else if (style == Slider::SliderStyle::LinearVertical)
        {
            p.addEllipse(width / 2.f - newDiameter / 2.f, sliderPos - newDiameter / 2, newDiameter, newDiameter);
        }
        auto pathColor = slider.isEnabled() ? mainTextColor : mainTextColor.withAlpha(0.4f);
        g.setColour(pathColor);

        g.fillPath(p);

        if (slider.isMouseOver() && slider.isEnabled())
        {
            g.setColour(sliderHoverFrameColor);
            g.strokePath(p, PathStrokeType(1.0f));
        }
    }

    void drawLabel(Graphics& g, Label& label) override
    {
        Rectangle<float> labelArea(0, 0, label.getWidth(), label.getHeight());

        g.setColour(labelBackgroundColor);
        auto labelInnerArea = labelArea.reduced(labelArea.getWidth() * 0.07f, labelArea.getHeight() * 0.08f);
        g.fillRect(labelInnerArea);

        auto textColor = label.isEnabled() ? mainTextColor : textButtonHoverBackgroundColor;
        g.setColour(textColor);

        int x = labelArea.getX();
        int w = labelArea.getWidth();
        int h = label.getTopLevelComponent()->getHeight() * 0.018f;
        int y = (labelArea.getHeight() - h) / 2;

        Font font(normalFont);
        font.setHeight(h);
        g.setFont(font);

        if (!label.isBeingEdited()) {
          g.drawFittedText(label.getText(), x, y, w, h, Justification::centred, 1);
       }
    }

    void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& textEditor) override
    {
        Rectangle<float> textEditorArea(0, 0, textEditor.getWidth(), textEditor.getHeight());
        g.setColour(textButtonHoverBackgroundColor);
        g.fillRect(textEditorArea);
    }

    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& textEditor) override
    {
        Rectangle<float> textEditorArea(0, 0, textEditor.getWidth(), textEditor.getHeight());
        if (textEditor.isEnabled())
        {
            g.setColour(mainTextColor);
            g.drawRect(textEditorArea, 1.f);
        }
    }

    void drawToggleButton(Graphics& g, ToggleButton& button,
        bool isMouseOverButton, bool isButtonDown) override
    {
        Rectangle<int> toggleButtonBounds(0, 0, button.getWidth(), button.getHeight());

        Font font(normalFont);

        int x = toggleButtonBounds.getX();
        int w = toggleButtonBounds.getWidth();
        int h = button.getTopLevelComponent()->getHeight() * 0.023f;
        int y = (button.getHeight() - h) / 2;

        font.setHeight(h);
        g.setFont(font);

        if (button.getButtonText() == "S" || button.getButtonText() == "M")
        { 
            toggleButtonBounds.reduce(4, 4);
            if (button.getToggleState())
            {
                Colour mainColour = button.isEnabled() ? textButtonActiveRedFrameColor : textButtonActiveRedFrameColor.withBrightness(0.5f);
                Colour textColour = button.isEnabled() ? mainTextColor : mainTextColor.withBrightness(0.5f);

                if (isMouseOverButton)
                {
                    g.setColour(mainColour.withAlpha(0.3f));
                    g.fillRect(toggleButtonBounds);
                }
                if (isButtonDown)
                {
                    g.setColour(mainColour.withAlpha(0.1f));
                    g.fillRect(toggleButtonBounds);
                }
                g.setColour(mainColour.withAlpha(0.2f));
                g.fillRect(toggleButtonBounds);

                g.setColour(mainColour.withAlpha(0.5f));
                g.drawRect(toggleButtonBounds, 1.f);
                g.setColour(textColour);
                g.drawFittedText(button.getButtonText(), x, y, w, h, Justification::centred, 1);
            }
            else
            {
                Colour mainColour = button.isEnabled() ? mainTextColor.withBrightness(0.5f) : textButtonFrameColor;
                Colour textColour = button.isEnabled() ? mainTextColor : mainTextColor.withBrightness(0.3f);

                if (isMouseOverButton)
                {
                    g.setColour(mainColour.withAlpha(0.3f));
                    g.fillRect(toggleButtonBounds);
                }
                if (isButtonDown)
                {
                    g.setColour(mainColour.withAlpha(0.1f));
                    g.fillRect(toggleButtonBounds);
                }
                g.setColour(mainColour.withAlpha(0.2f));
                g.fillRect(toggleButtonBounds);

                g.setColour(mainColour.withAlpha(0.5f));
                g.drawRect(toggleButtonBounds, 1.f);
                g.setColour(textColour);
                g.drawFittedText(button.getButtonText(), x, y, w, h, Justification::centred, 1);
            }
        }
        else
        {
            g.setColour(button.isEnabled() ? textButtonActiveRedFrameColor : textButtonActiveRedFrameColor.withAlpha(0.5f));
            Path outline;
            outline.addRoundedRectangle(toggleButtonBounds.reduced(button.getWidth() * 0.19f, button.getHeight() * 0.28f), button.getHeight() * 0.23f, button.getHeight() * 0.23f);

            g.strokePath(outline, PathStrokeType(2.0f));

            if (button.getToggleState() != true)
            {
                if (isMouseOverButton)
                {
                    g.setColour(textButtonHoverRedBackgroundColor);
                }
                else
                {
                    g.setColour(textButtonPressedRedBackgroundColor);
                }
            }
            else
            {
                if (isMouseOverButton)
                {
                    g.setColour(textButtonHoverRedBackgroundColor);
                }
                else
                {
                    g.setColour(textButtonPressedRedBackgroundColor);
                }
            }
            g.fillPath(outline);

            drawTickBox(g, button, toggleButtonBounds.getX(), toggleButtonBounds.getY(),
                toggleButtonBounds.getWidth(), toggleButtonBounds.getHeight(),
                button.getToggleState(), false, false, false);
        }
    }

    void drawTickBox(Graphics& g, Component& component,
        float x, float y, float w, float h,
        bool ticked,
        bool isEnabled,
        bool isMouseOverButton,
        bool isButtonDown) override
    {
        const float newDiameter = h * 0.34f;

        Path p;

        if (ticked)
        {
            p.addEllipse(w*0.77f - newDiameter, h / 2 - newDiameter / 2, newDiameter, newDiameter);
        }
        else
        {
            p.addEllipse(w*0.23f, h / 2 - newDiameter / 2, newDiameter, newDiameter);
        }
        
        g.setColour(component.isEnabled() ? mainTextColor : mainTextDisabledColor);
        g.fillPath(p);
    }

    void drawScrollbar(Graphics& g,
        ScrollBar& scrollbar,
        int  	x,
        int  	y,
        int  	width,
        int  	height,
        bool  	isScrollbarVertical,
        int  	thumbStartPosition,
        int  	thumbSize,
        bool  	isMouseOver,
        bool  	isMouseDown
    ) override
    {
        Path pathBgr;
        pathBgr.addRoundedRectangle(x, y, width, height, width / 2);
        g.setColour(textButtonFrameColor);
        g.fillPath(pathBgr);
        Path pathFgr;
        pathFgr.addRoundedRectangle(0, thumbStartPosition, width, thumbSize, width/2.f);
        g.setColour(sliderHoverFrameColor);
        g.fillPath(pathFgr);
    }
};