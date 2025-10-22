/*
  ==============================================================================

    MainLookAndFeel.h
    Created: 30 May 2023 4:54:48pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/
#pragma once

#include "BinaryFonts.h"

#include <JuceHeader.h>

using namespace juce;

class MainLookAndFeel : public LookAndFeel_V4
{
public:
    const Colour mainBackground = Colour (17, 18, 19);
    const Colour mainTextColor = Colour (255, 255, 255);
    const Colour mainTextDisabledColor = mainTextColor.withAlpha (0.4f);
    const Colour mainTextInactiveColor = mainTextColor.withAlpha (0.7f);
    const Colour textButtonFrameColor = Colour (52, 54, 57);
    const Colour labelBackgroundColor = Colour (39, 39, 44);
    const Colour multiTextButtonBackgroundColor = Colour (31, 32, 38);
    const Colour groupComponentBackgroundColor = Colour (24, 25, 28);
    const Colour textButtonDefaultBackgroundColor = Colour (24, 25, 27);
    const Colour textButtonActiveFrameColor = Colour (255, 255, 255);
    const Colour textButtonActiveRedFrameColor = Colour (182, 22, 22);
    const Colour textButtonActiveBlue1FrameColor = Colour (0, 49, 222);
    const Colour textButtonActiveBlue2FrameColor = Colour (0, 127, 255);
    const Colour textButtonActiveBlue3FrameColor = Colour (0, 87, 255);
    const Colour textButtonActiveBlue4FrameColor = Colour (108, 180, 238);
    const Colour sliderHoverFrameColor = Colour (93, 94, 95);
    const Colour polarVisualizerRed = Colour (182, 22, 22);
    const Colour polarVisualizerOrange = Colour (255, 107, 0);
    const Colour polarVisualizerYellow = Colour (214, 193, 9);
    const Colour polarVisualizerGreen = Colour (96, 154, 0);
    const Colour polarVisualizerGreenDark = Colour (0, 157, 25);
    const Colour polarVisualizerGrid = Colour (52, 54, 57);
    const Colour trimSliderMainColor = Colour (92, 92, 92);
    const Colour textButtonHoverBackgroundColor = textButtonActiveFrameColor.withAlpha (0.3f);
    const Colour textButtonHoverRedBackgroundColor = textButtonActiveRedFrameColor.withAlpha (0.3f);
    const Colour textButtonPressedBackgroundColor = textButtonActiveFrameColor.withAlpha (0.1f);
    const Colour textButtonPressedRedBackgroundColor =
        textButtonActiveRedFrameColor.withAlpha (0.1f);
    const Colour toggleButtonActiveRedBackgroundColor = Colour (182, 22, 22).withAlpha (0.7f);

    Typeface::Ptr normalFont;
    Typeface::Ptr terminatorRegularFont, terminatorBoldFont;

    MainLookAndFeel()
    {
        normalFont = Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansSemiBold_ttf,
                                                        BinaryFonts::NunitoSansSemiBold_ttfSize);
        terminatorRegularFont =
            Typeface::createSystemTypefaceFor (BinaryFonts::InterRegular_ttf,
                                               BinaryFonts::InterRegular_ttfSize);
        terminatorBoldFont = Typeface::createSystemTypefaceFor (BinaryFonts::InterBold_ttf,
                                                                BinaryFonts::InterBold_ttfSize);

        setColour (ListBox::backgroundColourId, groupComponentBackgroundColor);

        //        DBG("MainLookAndFeel Scale Factor: " << Desktop::getInstance().getGlobalScaleFactor());
    }

    ~MainLookAndFeel() override {}

    Typeface::Ptr getTypefaceForFont (const Font& f) override
    {
        switch (f.getStyleFlags())
        {
            case 1:
                return normalFont;
            default:
                return normalFont;
        }
    }

    // Helper function to get the scale factor
    float getScaleFactor (Component* component) const
    {
        return component ? Component::getApproximateScaleFactorForComponent (component)
                         : Desktop::getInstance().getGlobalScaleFactor();
    }

    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        juce::ignoreUnused (backgroundColour);

        float scale = getScaleFactor (&button);
        Rectangle<float> buttonArea (0.0f,
                                     0.0f,
                                     button.getWidth() * 1.0f,
                                     button.getHeight() * 1.0f);

        static auto austrianAudioLogoImg =
            juce::Drawable::createFromImageData (BinaryData::austrianAudioLogo_svg,
                                                 BinaryData::austrianAudioLogo_svgSize);
        static auto loadArrowImg =
            juce::Drawable::createFromImageData (BinaryData::loadArrow_svg,
                                                 BinaryData::loadArrow_svgSize);
        static auto freeFieldImg =
            juce::Drawable::createFromImageData (BinaryData::freeField_svg,
                                                 BinaryData::freeField_svgSize);
        static auto diffuseFieldImg =
            juce::Drawable::createFromImageData (BinaryData::diffuseField_svg,
                                                 BinaryData::diffuseField_svgSize);
        static auto eqFieldCheckSign =
            juce::Drawable::createFromImageData (BinaryData::eqFieldCheckSign_svg,
                                                 BinaryData::eqFieldCheckSign_svgSize);
        static auto terminateSpillIconImg =
            juce::Drawable::createFromImageData (BinaryData::terminateSpillIcon_svg,
                                                 BinaryData::terminateSpillIcon_svgSize);
        static auto maximizeTargetIconImg =
            juce::Drawable::createFromImageData (BinaryData::maximizeTargetIcon_svg,
                                                 BinaryData::maximizeTargetIcon_svgSize);
        static auto maxTargetToSpillIconImg =
            juce::Drawable::createFromImageData (BinaryData::maxTargetToSpillIcon_svg,
                                                 BinaryData::maximizeTargetIcon_svgSize);
        static auto closePresetListIconImg =
            juce::Drawable::createFromImageData (BinaryData::closePresetListIcon_svg,
                                                 BinaryData::closePresetListIcon_svgSize);
        static auto undoPresetIconImg =
            juce::Drawable::createFromImageData (BinaryData::undoPresetIcon_svg,
                                                 BinaryData::undoPresetIcon_svgSize);

        int cornerDirButtons =
            static_cast<int> (button.getTopLevelComponent()->proportionOfHeight (0.004f) / scale);
        int reduceYDirButtons =
            static_cast<int> (button.getTopLevelComponent()->proportionOfHeight (0.005f) / scale);

        if (button.getButtonText() == "Austrian Audio Logo")
        {
            auto austrianAudioLogoImgArea = buttonArea;
            austrianAudioLogoImg->drawWithin (g,
                                              austrianAudioLogoImgArea,
                                              juce::RectanglePlacement::xLeft,
                                              1.f);
        }
        else if (button.getButtonText() == "Zero latency")
        {
            g.setColour (button.isEnabled() ? textButtonActiveFrameColor
                                            : textButtonHoverBackgroundColor);
            g.drawRect (buttonArea, 1);

            auto toggleState = button.getToggleState();

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }

            if (toggleState)
            {
                g.setColour (polarVisualizerRed);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Load")
        {
            g.setColour (textButtonFrameColor);
            g.drawRect (buttonArea, 1);

            auto arrowArea = buttonArea
                                 .reduced (button.proportionOfWidth (0.45f) * 1.0f,
                                           button.proportionOfHeight (0.33f) * 1.0f)
                                 .translated (button.proportionOfWidth (0.36f) * 1.0f, 0.0f);

            if (! button.isEnabled())
            {
                bool result = loadArrowImg->replaceColour (Colours::white, mainTextDisabledColor);
                if (! result)
                    return;
            }

            loadArrowImg->drawWithin (g, arrowArea, juce::RectanglePlacement::centred, 1.f);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getComponentID() == "5621" || button.getComponentID() == "5721")
        {
            auto imageArea = buttonArea.reduced (5, 5);

            closePresetListIconImg->drawWithin (g,
                                                imageArea,
                                                juce::RectanglePlacement::centred,
                                                1.f);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Save")
        {
            g.setColour (textButtonFrameColor);
            g.drawRect (buttonArea, 1);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Open from file")
        {
            g.setColour (textButtonFrameColor);
            g.drawRect (buttonArea, 1);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Free Field")
        {
            g.setColour (textButtonFrameColor);
            g.drawRect (buttonArea, 1);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }

            if (! button.isEnabled())
            {
                bool resultMainImg =
                    freeFieldImg->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
                bool resultSignOutImg =
                    eqFieldCheckSign->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultSignOutImg)
                    return;
                bool resultSignInImg =
                    eqFieldCheckSign->replaceColour (textButtonActiveRedFrameColor,
                                                     textButtonHoverRedBackgroundColor);
                if (! resultSignInImg)
                    return;
            }

            auto freeFieldImageArea = buttonArea;
            freeFieldImageArea.removeFromTop (button.proportionOfHeight (0.11f) * 1.0f);
            freeFieldImageArea.removeFromBottom (button.proportionOfHeight (0.33f) * 1.0f);
            freeFieldImg->drawWithin (g,
                                      freeFieldImageArea,
                                      juce::RectanglePlacement::centred,
                                      1.f);

            if (button.getToggleState() == true)
            {
                auto eqFieldCheckSignArea = juce::Rectangle<float> (button.getWidth() * 0.77f,
                                                                    button.getHeight() * 0.05f,
                                                                    button.getWidth() * 0.18f,
                                                                    button.getWidth() * 0.18f);
                eqFieldCheckSign->drawWithin (g,
                                              eqFieldCheckSignArea,
                                              juce::RectanglePlacement::centred,
                                              1.f);
            }
        }
        else if (button.getButtonText() == "Diffuse Field")
        {
            g.setColour (textButtonFrameColor);
            g.drawRect (buttonArea, 1);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }

            if (! button.isEnabled())
            {
                bool resultMainImg =
                    diffuseFieldImg->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
                bool resultSignOutImg =
                    eqFieldCheckSign->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultSignOutImg)
                    return;
                bool resultSignInImg =
                    eqFieldCheckSign->replaceColour (textButtonActiveRedFrameColor,
                                                     textButtonHoverRedBackgroundColor);
                if (! resultSignInImg)
                    return;
            }

            auto diffuseFieldImageArea = buttonArea;
            diffuseFieldImageArea.removeFromTop (button.proportionOfHeight (0.11f) * 1.0f);
            diffuseFieldImageArea.removeFromBottom (button.proportionOfHeight (0.33f) * 1.0f);
            diffuseFieldImg->drawWithin (g,
                                         diffuseFieldImageArea,
                                         juce::RectanglePlacement::centred,
                                         1.f);

            if (button.getToggleState() == true)
            {
                auto eqFieldCheckSignArea = juce::Rectangle<float> (button.getWidth() * 0.77f,
                                                                    button.getHeight() * 0.05f,
                                                                    button.getWidth() * 0.18f,
                                                                    button.getWidth() * 0.18f);
                eqFieldCheckSign->drawWithin (g,
                                              eqFieldCheckSignArea,
                                              juce::RectanglePlacement::centred,
                                              1.f);
            }
        }
        else if (button.getButtonText() == "Terminate Spill")
        {
            g.setColour (textButtonFrameColor);
            auto reducedRect = buttonArea.reduced (4);
            g.drawRect (reducedRect, 1);

            Rectangle<float> iconArea =
                buttonArea
                    .reduced (button.proportionOfWidth (0.45f) * 1.0f,
                              button.proportionOfHeight (0.33f) * 1.0f)
                    .translated (button.proportionOfWidth (0.38f) * 1.0f, 0.0f);

            if (! button.isEnabled())
            {
                bool resultMainImg =
                    terminateSpillIconImg->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
            }
            terminateSpillIconImg->drawWithin (g, iconArea, juce::RectanglePlacement::centred, 1.f);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (reducedRect.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (reducedRect.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Maximize Target")
        {
            g.setColour (textButtonFrameColor);
            auto reducedRect = buttonArea.reduced (4);
            g.drawRect (reducedRect, 1);

            auto iconArea = buttonArea
                                .reduced (button.proportionOfWidth (0.45f) * 1.0f,
                                          button.proportionOfHeight (0.33f) * 1.0f)
                                .translated (button.proportionOfWidth (0.38f) * 1.0f, 0.0f);

            if (! button.isEnabled())
            {
                bool resultMainImg =
                    maximizeTargetIconImg->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
            }
            maximizeTargetIconImg->drawWithin (g, iconArea, juce::RectanglePlacement::centred, 1.f);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (reducedRect.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (reducedRect.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Max Target-to-spill")
        {
            g.setColour (textButtonFrameColor);
            auto reducedRect = buttonArea.reduced (4);
            g.drawRect (reducedRect, 1);

            auto iconArea = buttonArea
                                .reduced (button.proportionOfWidth (0.45f) * 1.0f,
                                          button.proportionOfHeight (0.33f) * 1.0f)
                                .translated (button.proportionOfWidth (0.38f) * 1.0f, 0.0f);

            if (! button.isEnabled())
            {
                bool resultMainImg =
                    maxTargetToSpillIconImg->replaceColour (Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
            }
            maxTargetToSpillIconImg->drawWithin (g,
                                                 iconArea,
                                                 juce::RectanglePlacement::centred,
                                                 1.f);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (reducedRect.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (reducedRect.reduced (1.0f, 1.0f));
            }
        }
        else if (button.getButtonText() == "Terminate spill"
                 || button.getButtonText() == "Maximize target"
                 || button.getButtonText() == "Max Target-to-Spill")
        {
        }
        else if (button.getButtonText().contains ("TermLine"))
        {
            Line<float> line (buttonArea.getCentreX(),
                              0,
                              buttonArea.getCentreX(),
                              buttonArea.getHeight());
            Path linePath;
            linePath.addLineSegment (line, buttonArea.proportionOfWidth (0.06f));
            g.setColour (textButtonPressedBackgroundColor);
            g.fillPath (linePath);
        }
        else if (button.getButtonText() == "01" || button.getButtonText() == "02"
                 || button.getButtonText() == "03")
        {
            if (button.isEnabled())
            {
                Path path;
                auto reducedButtonArea = buttonArea.reduced (1);
                auto ellipseArea = reducedButtonArea;
                ellipseArea.setWidth (
                    jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setHeight (
                    jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setCentre (buttonArea.getCentre());
                path.addEllipse (ellipseArea);
                g.setColour (toggleButtonActiveRedBackgroundColor);
                g.fillPath (path);
                g.setColour (textButtonActiveRedFrameColor);
                g.strokePath (path, PathStrokeType (ellipseArea.proportionOfWidth (0.06f)));
            }
            else
            {
                Path path;
                auto reducedButtonArea = buttonArea.reduced (1);
                auto ellipseArea = reducedButtonArea;
                ellipseArea.setWidth (
                    jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setHeight (
                    jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setCentre (buttonArea.getCentre());
                path.addEllipse (ellipseArea);

                auto color = textButtonPressedBackgroundColor;
                if (button.getToggleState())
                {
                    //Draw check sign when terminator stage completed
                    Line<float> line1 (
                        Point (ellipseArea.getX() + ellipseArea.proportionOfWidth (0.24f),
                               ellipseArea.getY() + ellipseArea.proportionOfHeight (0.47f)),
                        Point (ellipseArea.getX() + ellipseArea.proportionOfWidth (0.41f),
                               ellipseArea.getY() + ellipseArea.proportionOfHeight (0.67f)));

                    Line<float> line2 (
                        line1.getEnd(),
                        Point (ellipseArea.getX() + ellipseArea.proportionOfWidth (0.72f),
                               ellipseArea.getY() + ellipseArea.proportionOfHeight (0.26f)));

                    Path signPath;
                    signPath.addLineSegment (line1, ellipseArea.proportionOfWidth (0.06f));
                    signPath.addLineSegment (line2, ellipseArea.proportionOfWidth (0.06f));

                    color = textButtonActiveRedFrameColor.withAlpha (0.15f);
                    g.setColour (color);
                    g.fillPath (path);
                    ;
                    g.setColour (textButtonActiveRedFrameColor);
                    g.strokePath (path, PathStrokeType (ellipseArea.proportionOfWidth (0.06f)));

                    g.setColour (mainTextColor);
                    g.fillPath (signPath);
                }
                else
                {
                    g.setColour (color);
                    g.fillPath (path);
                    ;
                }
            }
        }
        else if (
            button.getButtonText()
                == "Click on the button below to apply polar\npatterns with minimum spill energy"
            || button.getButtonText()
                   == "Click on the button below to apply polar\npatterns with maximum signal energy"
            || button.getButtonText()
                   == "Find best compromise between reduction\nof spill and maximizing target signal")
        {
        }
        else if (button.getButtonText() == "Begin Terminate"
                 || button.getButtonText() == "Begin Maximize"
                 || button.getButtonText() == "Apply Max Target-to-Spill")
        {
            g.setColour (mainTextColor);
            g.drawRect (buttonArea, 1);

            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (1.0f, 1.0f));
            }
        }
        //Sync channel buttons
        else if (button.getComponentID() == "5521")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue1FrameColor
                                                : textButtonActiveBlue1FrameColor.withAlpha (0.5f);
            if (isMouseOverButton)
            {
                g.setColour (mainColor.withAlpha (0.3f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (4.0f, 4.0f));
                g.setColour (mainColor);
                g.drawRect (buttonArea.reduced (3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5522")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue2FrameColor
                                                : textButtonActiveBlue2FrameColor.withAlpha (0.5f);
            if (isMouseOverButton)
            {
                g.setColour (mainColor.withAlpha (0.3f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (4.0f, 4.0f));
                g.setColour (mainColor);
                g.drawRect (buttonArea.reduced (3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5523")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue3FrameColor
                                                : textButtonActiveBlue3FrameColor.withAlpha (0.5f);
            if (isMouseOverButton)
            {
                g.setColour (mainColor.withAlpha (0.3f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (4.0f, 4.0f));
                g.setColour (mainColor);
                g.drawRect (buttonArea.reduced (3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5524")
        {
            auto mainColor = button.isEnabled() ? textButtonActiveBlue4FrameColor
                                                : textButtonActiveBlue4FrameColor.withAlpha (0.5f);
            if (isMouseOverButton)
            {
                g.setColour (mainColor.withAlpha (0.3f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour (mainColor.withAlpha (0.1f));
                g.fillRect (buttonArea.reduced (4.0f, 4.0f));
                g.setColour (mainColor);
                g.drawRect (buttonArea.reduced (3.0f, 3.0f), 2);
            }
        }
        //Close preset and close terminator buttons
        else if (button.getComponentID() == "5621" || button.getComponentID() == "5721")
        {
            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverBackgroundColor);
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedBackgroundColor);
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
        }
        else if (button.getButtonText() == "Eight Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::eightPatternIcon_svg,
                                                     BinaryData::eightPatternIcon_svgSize),
                buttonArea,
                0,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "HyperCardioid Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::hyperCardioidPatternIcon_svg,
                                                     BinaryData::hyperCardioidPatternIcon_svgSize),
                buttonArea,
                0,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "SuperCardioid Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::superCardioidPatternIcon_svg,
                                                     BinaryData::superCardioidPatternIcon_svgSize),
                buttonArea,
                0,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "Cardioid Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::cardioidPatternIcon_svg,
                                                     BinaryData::cardioidPatternIcon_svgSize),
                buttonArea,
                reduceYDirButtons,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "BCardioid Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::bCardioidPatternIcon_svg,
                                                     BinaryData::bCardioidPatternIcon_svgSize),
                buttonArea,
                reduceYDirButtons,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "Omni Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::omniPatternIcon_svg,
                                                     BinaryData::omniPatternIcon_svgSize),
                buttonArea,
                reduceYDirButtons,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "RevBCardioid Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::revBCardioidPatternIcon_svg,
                                                     BinaryData::revBCardioidPatternIcon_svgSize),
                buttonArea,
                reduceYDirButtons,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "RevCardioid Pattern")
        {
            drawPatternImage (
                g,
                juce::Drawable::createFromImageData (BinaryData::revCardioidPatternIcon_svg,
                                                     BinaryData::revCardioidPatternIcon_svgSize),
                buttonArea,
                reduceYDirButtons,
                reduceYDirButtons,
                cornerDirButtons,
                isMouseOverButton,
                isButtonDown);
        }
        else if (button.getButtonText() == "Trim Slider Pointer")
        {
            g.setColour (Colours::red);
            int pointerHeight = static_cast<int> (buttonArea.getHeight() * 0.05f);
            int pointerX = 3;
            int pointerY = static_cast<int> (buttonArea.getCentreY() - pointerHeight / 2);
            int pointerWidth = static_cast<int> (buttonArea.getWidth() - 1.f);

            Path path;
            Point<float> point1 (pointerX * 1.0f, pointerY * 1.0f);
            Point<float> point2 (pointerWidth - 4.f, pointerY * 1.0f);
            Point<float> point3 (pointerWidth * 1.0f, buttonArea.getCentreY());
            Point<float> point4 (pointerWidth - 4.f, (pointerY + pointerHeight) * 1.0f);
            Point<float> point5 (pointerX * 1.0f, (pointerY + pointerHeight) * 1.0f);

            path.startNewSubPath (point1);
            path.lineTo (point2);
            path.lineTo (point3);
            path.lineTo (point4);
            path.lineTo (point5);
            path.closeSubPath();

            g.setColour (textButtonFrameColor);
            g.fillPath (path);
        }
        else if (button.getButtonText() == "Preset undo")
        {
            auto imageArea = buttonArea.translated (0, 1);
            undoPresetIconImg->drawWithin (g, imageArea, juce::RectanglePlacement::centred, 1.f);
        }
        //Nr of bands buttons
        else
        {
            //            auto mainColor = button.isEnabled() ? textButtonActiveBlue4FrameColor : textButtonActiveBlue4FrameColor.withAlpha(0.5f);
            if (isMouseOverButton)
            {
                g.setColour (textButtonHoverRedBackgroundColor);
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour (textButtonPressedRedBackgroundColor);
                g.fillRect (buttonArea.reduced (3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour (textButtonPressedRedBackgroundColor);
                g.fillRect (buttonArea.reduced (4.0f, 4.0f));
                g.setColour (button.isEnabled() ? textButtonActiveRedFrameColor
                                                : textButtonActiveRedFrameColor.withAlpha (0.5f));
                g.drawRect (buttonArea.reduced (3.0f, 3.0f), 2);
            }
        }
    }

    void drawButtonText (Graphics& g,
                         TextButton& button,
                         bool /*isMouseOverButton*/,
                         bool /*isButtonDown*/) override
    {
        float scale = getScaleFactor (&button);
        Rectangle<int> buttonArea (0, 0, button.getWidth(), button.getHeight());

        g.setColour (button.isEnabled() ? mainTextColor : mainTextDisabledColor);

        Font font (normalFont);

        String text = button.getButtonText();

        int x = buttonArea.getX();
        int w = buttonArea.getWidth();
        int h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.023f / scale);
        int y = (buttonArea.getHeight() - h) / 2;

        auto justification = Justification::centred;

        if (button.getButtonText() == "Load")
        {
            x = buttonArea.proportionOfWidth (0.18f);
            w = buttonArea.proportionOfWidth (0.47f);
        }
        else if (button.getButtonText() == "Free Field")
        {
            y = buttonArea.proportionOfHeight (0.7f);
            h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.02f / scale);
        }
        else if (button.getButtonText() == "Diffuse Field")
        {
            y = buttonArea.proportionOfHeight (0.7f);
            h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.018f / scale);
        }
        else if (button.getButtonText() == "Terminate Spill"
                 || button.getButtonText() == "Maximize Target"
                 || button.getButtonText() == "Max Target-to-spill")
        {
            justification = Justification::centredLeft;
            x = buttonArea.proportionOfWidth (0.11f);
            w = buttonArea.proportionOfWidth (0.78f);
        }
        else if (button.getButtonText() == "Begin Terminate"
                 || button.getButtonText() == "Begin Maximize"
                 || button.getButtonText() == "Apply Max Target-to-Spill")
        {
            justification = Justification::centredLeft;
            x = buttonArea.proportionOfWidth (0.11f);
            w = buttonArea.proportionOfWidth (0.78f);
        }
        else if (button.getButtonText() == "Terminate spill"
                 || button.getButtonText() == "Maximize target"
                 || button.getButtonText() == "Max Target-to-Spill")
        {
            g.setColour (button.isEnabled() ? mainTextColor : mainTextInactiveColor);
            font = button.isEnabled() ? terminatorBoldFont : terminatorRegularFont;
            justification = Justification::centredLeft;
            h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.015f / scale);
            y = (buttonArea.getHeight() - h) / 2;
        }
        else if (button.getButtonText() == "01" || button.getButtonText() == "02"
                 || button.getButtonText() == "03")
        {
            if (button.getToggleState())
                return;

            g.setColour (button.isEnabled() ? mainTextColor : mainTextInactiveColor);
            String tmpText ("");
            tmpText += button.getButtonText().getCharPointer()[1];
            text = tmpText;
            font = button.isEnabled() ? terminatorBoldFont : terminatorRegularFont;
            justification = Justification::centred;
            h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.014f / scale);
            y = (buttonArea.getHeight() - h) / 2;
        }
        else if (button.getButtonText().contains ("TermLine"))
        {
            return;
        }
        else if (
            button.getButtonText()
                == "Click on the button below to apply polar\npatterns with minimum spill energy"
            || button.getButtonText()
                   == "Click on the button below to apply polar\npatterns with maximum signal energy"
            || button.getButtonText()
                   == "Find best compromise between reduction\nof spill and maximizing target signal")
        {
            g.setColour (button.isEnabled() ? mainTextColor : mainTextInactiveColor);
            font = terminatorRegularFont;
            justification = Justification::centredLeft;
            h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.012f / scale);
            w = buttonArea.proportionOfWidth (1.f);
        }
        else if (button.getButtonText() == "Eight Pattern"
                 || button.getButtonText() == "HyperCardioid Pattern"
                 || button.getButtonText() == "SuperCardioid Pattern"
                 || button.getButtonText() == "Cardioid Pattern"
                 || button.getButtonText() == "BCardioid Pattern"
                 || button.getButtonText() == "Omni Pattern"
                 || button.getButtonText() == "RevBCardioid Pattern"
                 || button.getButtonText() == "RevCardioid Pattern")
        {
            return;
        }
        else if (button.getButtonText() == "Trim Slider Pointer")
        {
            return;
        }
        else if (button.getButtonText() == "Austrian Audio Logo")
        {
            return;
        }
        else if (button.getButtonText() == "Preset undo")
        {
            return;
        }

        font.setHeight (h * 1.0f);
        g.setFont (font);
        g.drawFittedText (text, x, y, w, h, justification, 1);
    }

    void drawGroupComponentOutline (Graphics& g,
                                    int width,
                                    int height,
                                    const String& text,
                                    const Justification& position,
                                    GroupComponent& group) override
    {
        (void) width;
        (void) height;
        (void) position;
        float scale = getScaleFactor (&group);
        Rectangle<float> groupArea (0, 0, group.getWidth() * 1.0f, group.getHeight() * 1.0f);
        g.setColour (groupComponentBackgroundColor);

        juce::Path path;
        if (text == "Preset")
        {
            path.addRectangle (groupArea);
        }
        else
        {
            path.addRoundedRectangle (groupArea, 10.f);
        }

        path.closeSubPath();
        g.fillPath (path);

        g.setColour (group.isEnabled() ? mainTextColor : mainTextDisabledColor);

        int x = group.proportionOfWidth (0.06f);
        int y = static_cast<int> (10 / scale);
        int w = group.proportionOfWidth (0.87f);
        int h = static_cast<int> (group.getTopLevelComponent()->getHeight() * 0.025f / scale);

        if (text == "Preset")
        {
            y = group.proportionOfHeight (0.04f);
        }

        Font font (normalFont);
        font.setHeight (h * 1.0f);
        g.setFont (font);
        g.drawFittedText (text, x, y, w, h, Justification::left, 1);
    }

    Slider::SliderLayout getSliderLayout (Slider& slider) override
    {
        float scale = getScaleFactor (&slider);
        Rectangle<int> localBounds (0, 0, slider.getWidth(), slider.getHeight());
        Slider::SliderLayout layout;

        if (slider.getSliderStyle() == Slider::SliderStyle::LinearHorizontal)
        {
            layout.sliderBounds.setBounds (static_cast<int> (localBounds.getWidth() * 0.07f),
                                           localBounds.getY(),
                                           static_cast<int> (localBounds.getWidth() * 0.42f),
                                           localBounds.getHeight());
            layout.textBoxBounds.setBounds (
                static_cast<int> (localBounds.getWidth() * 0.71f),
                localBounds.getY(),
                static_cast<int> (localBounds.getWidth() * 0.29f / scale),
                static_cast<int> (localBounds.getHeight() / scale));
            layout.textBoxBounds.reduce (static_cast<int> (10.0f / scale),
                                         static_cast<int> (10.0f / scale));
        }
        else if (slider.getSliderStyle() == Slider::SliderStyle::LinearVertical)
        {
            auto layoutWidth = slider.getTopLevelComponent()->getWidth() * 0.027f / scale;

            layout.sliderBounds.setBounds (
                static_cast<int> (localBounds.getRight() - layoutWidth + 1),
                localBounds.getY(),
                static_cast<int> (layoutWidth),
                localBounds.getHeight());

            int textBoxWidth =
                static_cast<int> (slider.getTopLevelComponent()->getWidth() * 0.05f / scale);
            int textBoxHeight =
                static_cast<int> (slider.getTopLevelComponent()->getHeight() * 0.029f / scale);
            int textBoxX =
                static_cast<int> (layout.sliderBounds.getTopLeft().getX() - textBoxWidth + 3.f);
            layout.textBoxBounds.setBounds (textBoxX,
                                            layout.sliderBounds.getCentreY() - textBoxHeight / 2,
                                            textBoxWidth,
                                            textBoxHeight);
        }

        return layout;
    }

    void drawLinearSlider (Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const Slider::SliderStyle style,
                           Slider& slider) override
    {
        drawLinearSliderBackground (g,
                                    x,
                                    y,
                                    width,
                                    height,
                                    sliderPos,
                                    minSliderPos,
                                    maxSliderPos,
                                    style,
                                    slider);
        drawLinearSliderThumb (g,
                               x,
                               y,
                               width,
                               height,
                               sliderPos,
                               minSliderPos,
                               maxSliderPos,
                               style,
                               slider);
    }

    void drawLinearSliderBackground (Graphics& g,
                                     int x,
                                     int y,
                                     int width,
                                     int height,
                                     float sliderPos,
                                     float minSliderPos,
                                     float maxSliderPos,
                                     const Slider::SliderStyle style,
                                     Slider& slider) override
    {
        (void) minSliderPos;
        (void) maxSliderPos;
        float scale = getScaleFactor (&slider);
        const float h = slider.getTopLevelComponent()->getHeight() * 0.005f / scale;
        const float newDiameter = slider.getTopLevelComponent()->getHeight() * 0.024f / scale;

        Path pathBgr;
        Path pathFrg;
        auto pathBgrColor =
            slider.isEnabled() ? textButtonFrameColor : textButtonFrameColor.withAlpha (0.4f);
        auto pathFrgColor = slider.isEnabled() ? textButtonActiveRedFrameColor
                                               : textButtonActiveRedFrameColor.withAlpha (0.2f);

        if (style == Slider::SliderStyle::LinearHorizontal)
        {
            Rectangle<float> backgroundRect (x * 1.0f,
                                             0.5f * height - h / 2,
                                             width * 1.0f,
                                             h * 1.0f);
            pathBgr.addRoundedRectangle (backgroundRect, h);
            Rectangle<float> foregroundRect ((width / 2 + x) * 1.0f,
                                             0.5f * height - h / 2,
                                             (sliderPos - (width / 2 + x) * 1.0f),
                                             h * 1.0f);
            pathFrg.addRectangle (foregroundRect);
        }
        else if (style == Slider::SliderStyle::LinearVertical)
        {
            Rectangle<float> backgroundRect (x + width / 2 - h / 2,
                                             y + newDiameter / 2,
                                             h,
                                             height - newDiameter);
            pathBgr.addRoundedRectangle (backgroundRect, h);
            auto mappedSliderPos = jmap (sliderPos,
                                         0.f,
                                         static_cast<float> (height),
                                         y + newDiameter / 2,
                                         height - newDiameter / 2);
            Rectangle<float> foregroundRect (x + width / 2 - h / 2,
                                             y + backgroundRect.getHeight() * 0.43f,
                                             h,
                                             mappedSliderPos
                                                 - (y + backgroundRect.getHeight() * 0.43f));
            pathFrg.addRectangle (foregroundRect);
        }
        g.setColour (pathBgrColor);
        g.fillPath (pathBgr);
        g.setColour (pathFrgColor);
        g.fillPath (pathFrg);
    }

    void drawLinearSliderThumb (Graphics& g,
                                int x,
                                int y,
                                int width,
                                int height,
                                float sliderPos,
                                float minSliderPos,
                                float maxSliderPos,
                                const Slider::SliderStyle style,
                                Slider& slider) override
    {
        (void) minSliderPos;
        (void) maxSliderPos;
        float scale = getScaleFactor (&slider);
        const float newDiameter = slider.getTopLevelComponent()->getHeight() * 0.024f / scale;

        Path p;
        if (style == Slider::SliderStyle::LinearHorizontal)
        {
            p.addEllipse (sliderPos - newDiameter / 2,
                          height / 2.f - newDiameter / 2.f,
                          newDiameter,
                          newDiameter);
        }
        else if (style == Slider::SliderStyle::LinearVertical)
        {
            auto mappedSliderPos = jmap (sliderPos,
                                         0.f,
                                         static_cast<float> (height),
                                         y + newDiameter / 2,
                                         height - newDiameter / 2);
            p.addEllipse (x + width / 2 - newDiameter / 2,
                          mappedSliderPos - newDiameter / 2,
                          newDiameter,
                          newDiameter);
        }
        auto pathColor = slider.isEnabled() ? mainTextColor : mainTextColor.withAlpha (0.4f);
        g.setColour (pathColor);

        g.fillPath (p);

        if (slider.isMouseOver() && slider.isEnabled())
        {
            g.setColour (sliderHoverFrameColor);
            g.strokePath (p, PathStrokeType (1.0f / scale));
        }
    }

    void drawLabel (Graphics& g, Label& label) override
    {
        float scale = getScaleFactor (&label);
        Rectangle<float> labelArea (0.0f, 0.0f, label.getWidth() * 1.0f, label.getHeight() * 1.0f);

        g.setColour (labelBackgroundColor);
        auto labelInnerArea =
            labelArea.reduced (labelArea.getWidth() * 0.07f, labelArea.getHeight() * 0.08f);
        g.fillRect (labelInnerArea);

        auto textColor = label.isEnabled() ? mainTextColor : textButtonHoverBackgroundColor;
        g.setColour (textColor);

        int x = static_cast<int> (labelArea.getX());
        int w = static_cast<int> (labelArea.getWidth());
        int h = static_cast<int> (static_cast<float> (label.getTopLevelComponent()->getHeight())
                                  * 0.018f / scale);
        int y = static_cast<int> ((labelArea.getHeight() - h) / 2);

        Font font (normalFont);
        font.setHeight (h * 1.0f);
        g.setFont (font);

        if (! label.isBeingEdited())
        {
            g.drawFittedText (label.getText(), x, y, w, h, Justification::centred, 1);
        }
    }

    void fillTextEditorBackground (Graphics& g,
                                   int width,
                                   int height,
                                   TextEditor& textEditor) override
    {
        (void) width;
        (void) height;
        Rectangle<float> textEditorArea (0.0f,
                                         0.0f,
                                         textEditor.getWidth() * 1.0f,
                                         textEditor.getHeight() * 1.0f);
        g.setColour (textButtonHoverBackgroundColor);
        g.fillRect (textEditorArea);
    }

    void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor) override
    {
        juce::ignoreUnused (width, height);

        float scale = getScaleFactor (&textEditor);
        Rectangle<float> textEditorArea (0.0f,
                                         0.0f,
                                         textEditor.getWidth() * 1.0f,
                                         textEditor.getHeight() * 1.0f);
        if (textEditor.isEnabled())
        {
            g.setColour (mainTextColor);
            g.drawRect (textEditorArea, 1.f / scale);
        }
    }

    void drawToggleButton (Graphics& g,
                           ToggleButton& button,
                           bool isMouseOverButton,
                           bool isButtonDown) override
    {
        float scale = getScaleFactor (&button);
        Rectangle<float> toggleButtonBounds (0.0f,
                                             0.0f,
                                             button.getWidth() * 1.0f,
                                             button.getHeight() * 1.0f);

        Font font (normalFont);

        int x = static_cast<int> (toggleButtonBounds.getX());
        int w = static_cast<int> (toggleButtonBounds.getWidth());
        int h = static_cast<int> (button.getTopLevelComponent()->getHeight() * 0.023f / scale);
        int y = static_cast<int> ((button.getHeight() - h) / 2.0f);

        font.setHeight (h * 1.0f);
        g.setFont (font);

        if (button.getButtonText() == "S" || button.getButtonText() == "M")
        {
            toggleButtonBounds.reduce (4, 4);
            if (button.getToggleState())
            {
                Colour soloMainColour = button.isEnabled()
                                            ? polarVisualizerYellow
                                            : polarVisualizerYellow.withBrightness (0.5f);
                Colour muteMainColour = button.isEnabled()
                                            ? textButtonActiveRedFrameColor
                                            : textButtonActiveRedFrameColor.withBrightness (0.5f);

                Colour mainColour = button.getButtonText() == "S" ? soloMainColour : muteMainColour;
                Colour textColour =
                    button.isEnabled() ? mainTextColor : mainTextColor.withBrightness (0.5f);

                if (isMouseOverButton)
                {
                    g.setColour (mainColour.withAlpha (0.3f));
                    g.fillRect (toggleButtonBounds);
                }
                if (isButtonDown)
                {
                    g.setColour (mainColour.withAlpha (0.1f));
                    g.fillRect (toggleButtonBounds);
                }
                g.setColour (mainColour.withAlpha (0.2f));
                g.fillRect (toggleButtonBounds);

                g.setColour (mainColour.withAlpha (0.5f));
                g.drawRect (toggleButtonBounds, 1.f);
                g.setColour (textColour);
                g.drawFittedText (button.getButtonText(), x, y, w, h, Justification::centred, 1);
            }
            else
            {
                Colour mainColour =
                    button.isEnabled() ? mainTextColor.withBrightness (0.5f) : textButtonFrameColor;
                Colour textColour =
                    button.isEnabled() ? mainTextColor : mainTextColor.withBrightness (0.3f);

                if (isMouseOverButton)
                {
                    g.setColour (mainColour.withAlpha (0.3f));
                    g.fillRect (toggleButtonBounds);
                }
                if (isButtonDown)
                {
                    g.setColour (mainColour.withAlpha (0.1f));
                    g.fillRect (toggleButtonBounds);
                }
                g.setColour (mainColour.withAlpha (0.2f));
                g.fillRect (toggleButtonBounds);

                g.setColour (mainColour.withAlpha (0.5f));
                g.drawRect (toggleButtonBounds, 1.f / scale);
                g.setColour (textColour);
                g.drawFittedText (button.getButtonText(), x, y, w, h, Justification::centred, 1);
            }
        }
        else
        {
            g.setColour (button.isEnabled() ? textButtonActiveRedFrameColor
                                            : textButtonActiveRedFrameColor.withAlpha (0.5f));
            Path outline;
            outline.addRoundedRectangle (toggleButtonBounds.reduced ((button.getWidth() * 0.19f),
                                                                     (button.getHeight() * 0.28f)),
                                         button.getHeight() * 0.23f,
                                         button.getHeight() * 0.23f);

            g.strokePath (outline, PathStrokeType (2.0f));

            if (button.getToggleState() != true)
            {
                if (isMouseOverButton)
                {
                    g.setColour (textButtonHoverRedBackgroundColor);
                }
                else
                {
                    g.setColour (textButtonPressedRedBackgroundColor);
                }
            }
            else
            {
                if (isMouseOverButton)
                {
                    g.setColour (textButtonHoverRedBackgroundColor);
                }
                else
                {
                    g.setColour (textButtonPressedRedBackgroundColor);
                }
            }
            g.fillPath (outline);

            drawTickBox (g,
                         button,
                         toggleButtonBounds.getX(),
                         toggleButtonBounds.getY(),
                         toggleButtonBounds.getWidth(),
                         toggleButtonBounds.getHeight(),
                         button.getToggleState(),
                         false,
                         false,
                         false);
        }
    }

    void drawTickBox (Graphics& g,
                      Component& component,
                      float x,
                      float y,
                      float w,
                      float h,
                      bool ticked,
                      bool isEnabled,
                      bool isMouseOverButton,
                      bool isButtonDown) override
    {
        juce::ignoreUnused (x, y, isEnabled, isMouseOverButton, isButtonDown);

        const float newDiameter = h * 0.34f;

        Path p;

        if (ticked)
        {
            p.addEllipse (w * 0.77f - newDiameter,
                          h / 2 - newDiameter / 2,
                          newDiameter,
                          newDiameter);
        }
        else
        {
            p.addEllipse (w * 0.23f, h / 2 - newDiameter / 2, newDiameter, newDiameter);
        }

        g.setColour (component.isEnabled() ? mainTextColor : mainTextDisabledColor);
        g.fillPath (p);
    }

    void drawScrollbar (Graphics& g,
                        ScrollBar& scrollbar,
                        int x,
                        int y,
                        int width,
                        int height,
                        bool isScrollbarVertical,
                        int thumbStartPosition,
                        int thumbSize,
                        bool isMouseOver,
                        bool isMouseDown) override
    {
        juce::ignoreUnused (scrollbar, isScrollbarVertical, isMouseOver, isMouseDown);

        float scale = getScaleFactor (&scrollbar);

        Path pathBgr;
        pathBgr.addRoundedRectangle (x * 1.0f,
                                     y * 1.0f,
                                     width * 1.0f,
                                     height * 1.0f,
                                     (width / 2.0f) * 1.0f / scale);
        g.setColour (textButtonFrameColor);
        g.fillPath (pathBgr);
        Path pathFgr;
        pathFgr.addRoundedRectangle (0.0f,
                                     thumbStartPosition * 1.0f,
                                     width * 1.0f,
                                     thumbSize * 1.0f,
                                     (width / 2.0f) * 1.0f / scale);
        g.setColour (sliderHoverFrameColor);
        g.fillPath (pathFgr);
    }

private:
    void drawPatternImage (Graphics& g,
                           std::unique_ptr<Drawable> image,
                           Rectangle<float>& buttonArea,
                           int reduceX,
                           int reduceY,
                           int corner,
                           bool mouseOver,
                           bool mouseDown)
    {
        // Use global scale factor since no specific component is passed
        float scale = getScaleFactor (nullptr);
        g.setColour (labelBackgroundColor);

        int deltaX = 0;

        if (SystemStats::getOperatingSystemName() == "iOS")
            deltaX = static_cast<int> (buttonArea.proportionOfWidth (0.24f));

        int deltaY = static_cast<int> (1 / scale);
        g.fillRoundedRectangle (buttonArea.reduced (deltaX * 1.0f, deltaY * 1.0f),
                                corner * 1.0f / scale);
        auto imageRect = buttonArea.reduced (reduceX * 1.0f / scale, reduceY * 1.0f / scale);
        if (mouseOver)
        {
            bool resultMainImg = image->replaceColour (Colours::white, sliderHoverFrameColor);
            if (! resultMainImg)
                return;
        }
        if (mouseDown)
        {
            bool resultMainImg =
                image->replaceColour (sliderHoverFrameColor, Colours::white.withAlpha (0.7f));
            if (! resultMainImg)
                return;
        }
        image->drawWithin (g, imageRect, juce::RectanglePlacement::centred, 1.f);
    }
};
