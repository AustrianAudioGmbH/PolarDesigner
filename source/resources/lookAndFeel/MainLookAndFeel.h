/*
  ==============================================================================

    MainLookAndFeel.h
    Created: 30 May 2023 4:54:48pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/
#pragma once

#include "BinaryData.h"
#include "BinaryFonts.h"

#include <juce_gui_basics/juce_gui_basics.h>

class MainLookAndFeel : public juce::LookAndFeel_V4
{
    using Colour = juce::Colour;
    using Typeface = juce::Typeface;
    using Font = juce::Font;
    using Component = juce::Component;
    using Graphics = juce::Graphics;
    using Button = juce::Button;
    using TextButton = juce::TextButton;
    using ListBox = juce::ListBox;
    using Desktop = juce::Desktop;
    using String = juce::String;
    using Justification = juce::Justification;
    using Slider = juce::Slider;
    using Label = juce::Label;
    using ToggleButton = juce::ToggleButton;
    using GroupComponent = juce::GroupComponent;
    using TextEditor = juce::TextEditor;
    using ScrollBar = juce::ScrollBar;
    using Rectangle = juce::Rectangle<float>;
    using Drawable = juce::Drawable;
    using Line = juce::Line<float>;
    using Path = juce::Path;
    using Point = juce::Point<float>;

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
        Rectangle buttonArea (0.0f,
                              0.0f,
                              static_cast<float> (button.getWidth()),
                              static_cast<float> (button.getHeight()));

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

        int cornerDirButtons = static_cast<int> (
            static_cast<float> (button.getTopLevelComponent()->proportionOfHeight (0.004f))
            / scale);
        int reduceYDirButtons = static_cast<int> (
            static_cast<float> (button.getTopLevelComponent()->proportionOfHeight (0.005f))
            / scale);

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

            auto arrowArea =
                buttonArea
                    .reduced (static_cast<float> (button.proportionOfWidth (0.45f)),
                              static_cast<float> (button.proportionOfHeight (0.33f)))
                    .translated (static_cast<float> (button.proportionOfWidth (0.36f)), 0.0f);

            if (! button.isEnabled())
            {
                bool result =
                    loadArrowImg->replaceColour (juce::Colours::white, mainTextDisabledColor);
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
                    freeFieldImg->replaceColour (juce::Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
                bool resultSignOutImg =
                    eqFieldCheckSign->replaceColour (juce::Colours::white, mainTextDisabledColor);
                if (! resultSignOutImg)
                    return;
                bool resultSignInImg =
                    eqFieldCheckSign->replaceColour (textButtonActiveRedFrameColor,
                                                     textButtonHoverRedBackgroundColor);
                if (! resultSignInImg)
                    return;
            }

            auto freeFieldImageArea = buttonArea;
            freeFieldImageArea.removeFromTop (
                static_cast<float> (button.proportionOfHeight (0.11f)));
            freeFieldImageArea.removeFromBottom (
                static_cast<float> (button.proportionOfHeight (0.33f)));
            freeFieldImg->drawWithin (g,
                                      freeFieldImageArea,
                                      juce::RectanglePlacement::centred,
                                      1.f);

            if (button.getToggleState() == true)
            {
                auto eqFieldCheckSignArea =
                    juce::Rectangle<float> (static_cast<float> (button.getWidth()) * 0.77f,
                                            static_cast<float> (button.getHeight()) * 0.05f,
                                            static_cast<float> (button.getWidth()) * 0.18f,
                                            static_cast<float> (button.getWidth()) * 0.18f);
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
                    diffuseFieldImg->replaceColour (juce::Colours::white, mainTextDisabledColor);
                if (! resultMainImg)
                    return;
                bool resultSignOutImg =
                    eqFieldCheckSign->replaceColour (juce::Colours::white, mainTextDisabledColor);
                if (! resultSignOutImg)
                    return;
                bool resultSignInImg =
                    eqFieldCheckSign->replaceColour (textButtonActiveRedFrameColor,
                                                     textButtonHoverRedBackgroundColor);
                if (! resultSignInImg)
                    return;
            }

            auto diffuseFieldImageArea = buttonArea;
            diffuseFieldImageArea.removeFromTop (
                static_cast<float> (button.proportionOfHeight (0.11f)));
            diffuseFieldImageArea.removeFromBottom (
                static_cast<float> (button.proportionOfHeight (0.33f)));
            diffuseFieldImg->drawWithin (g,
                                         diffuseFieldImageArea,
                                         juce::RectanglePlacement::centred,
                                         1.f);

            if (button.getToggleState() == true)
            {
                auto eqFieldCheckSignArea =
                    juce::Rectangle<float> (static_cast<float> (button.getWidth()) * 0.77f,
                                            static_cast<float> (button.getHeight()) * 0.05f,
                                            static_cast<float> (button.getWidth()) * 0.18f,
                                            static_cast<float> (button.getWidth()) * 0.18f);
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

            Rectangle iconArea =
                buttonArea
                    .reduced (static_cast<float> (button.proportionOfWidth (0.45f)),
                              static_cast<float> (button.proportionOfHeight (0.33f)))
                    .translated (static_cast<float> (button.proportionOfWidth (0.38f)), 0.0f);

            if (! button.isEnabled())
            {
                bool resultMainImg = terminateSpillIconImg->replaceColour (juce::Colours::white,
                                                                           mainTextDisabledColor);
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

            auto iconArea =
                buttonArea
                    .reduced (static_cast<float> (button.proportionOfWidth (0.45f)),
                              static_cast<float> (button.proportionOfHeight (0.33f)))
                    .translated (static_cast<float> (button.proportionOfWidth (0.38f)), 0.0f);

            if (! button.isEnabled())
            {
                bool resultMainImg = maximizeTargetIconImg->replaceColour (juce::Colours::white,
                                                                           mainTextDisabledColor);
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

            auto iconArea =
                buttonArea
                    .reduced (static_cast<float> (button.proportionOfWidth (0.45f)),
                              static_cast<float> (button.proportionOfHeight (0.33f)))
                    .translated (static_cast<float> (button.proportionOfWidth (0.38f)), 0.0f);

            if (! button.isEnabled())
            {
                bool resultMainImg = maxTargetToSpillIconImg->replaceColour (juce::Colours::white,
                                                                             mainTextDisabledColor);
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
            Line line (buttonArea.getCentreX(), 0, buttonArea.getCentreX(), buttonArea.getHeight());
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
                    juce::jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setHeight (
                    juce::jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setCentre (buttonArea.getCentre());
                path.addEllipse (ellipseArea);
                g.setColour (toggleButtonActiveRedBackgroundColor);
                g.fillPath (path);
                g.setColour (textButtonActiveRedFrameColor);
                g.strokePath (path, juce::PathStrokeType (ellipseArea.proportionOfWidth (0.06f)));
            }
            else
            {
                Path path;
                auto reducedButtonArea = buttonArea.reduced (1);
                auto ellipseArea = reducedButtonArea;
                ellipseArea.setWidth (
                    juce::jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setHeight (
                    juce::jmin (reducedButtonArea.getWidth(), reducedButtonArea.getHeight()));
                ellipseArea.setCentre (buttonArea.getCentre());
                path.addEllipse (ellipseArea);

                auto color = textButtonPressedBackgroundColor;
                if (button.getToggleState())
                {
                    //Draw check sign when terminator stage completed
                    Line line1 (
                        Point (ellipseArea.getX() + ellipseArea.proportionOfWidth (0.24f),
                               ellipseArea.getY() + ellipseArea.proportionOfHeight (0.47f)),
                        Point (ellipseArea.getX() + ellipseArea.proportionOfWidth (0.41f),
                               ellipseArea.getY() + ellipseArea.proportionOfHeight (0.67f)));

                    Line line2 (
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
                    g.strokePath (path,
                                  juce::PathStrokeType (ellipseArea.proportionOfWidth (0.06f)));

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
            g.setColour (juce::Colours::red);
            auto pointerHeight = buttonArea.getHeight() * 0.05f;
            auto pointerX = 3.0f;
            auto pointerY = buttonArea.getCentreY() - pointerHeight / 2.0f;
            auto pointerWidth = buttonArea.getWidth() - 1.0f;

            Path path;
            Point point1 (pointerX, pointerY);
            Point point2 (pointerWidth - 4.0f, pointerY);
            Point point3 (pointerWidth, buttonArea.getCentreY());
            Point point4 (pointerWidth - 4.0f, pointerY + pointerHeight);
            Point point5 (pointerX, pointerY + pointerHeight);

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
        juce::Rectangle<int> buttonArea (0, 0, button.getWidth(), button.getHeight());

        g.setColour (button.isEnabled() ? mainTextColor : mainTextDisabledColor);

        Font font (normalFont);

        String text = button.getButtonText();

        auto x = buttonArea.getX();
        auto w = buttonArea.getWidth();
        auto h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                   * 0.023f / scale);
        auto y = (buttonArea.getHeight() - h) / 2;

        auto justification = Justification::centred;

        if (button.getButtonText() == "Load")
        {
            x = buttonArea.proportionOfWidth (0.18f);
            w = buttonArea.proportionOfWidth (0.47f);
        }
        else if (button.getButtonText() == "Free Field")
        {
            y = buttonArea.proportionOfHeight (0.7f);
            h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                  * 0.02f / scale);
        }
        else if (button.getButtonText() == "Diffuse Field")
        {
            y = buttonArea.proportionOfHeight (0.7f);
            h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                  * 0.018f / scale);
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
            h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                  * 0.015f / scale);
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
            h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                  * 0.014f / scale);
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
            h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                  * 0.012f / scale);
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

        font.setHeight (static_cast<float> (h));
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
        Rectangle groupArea (0,
                             0,
                             static_cast<float> (group.getWidth()),
                             static_cast<float> (group.getHeight()));
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
        int h = static_cast<int> (static_cast<float> (group.getTopLevelComponent()->getHeight())
                                  * 0.025f / scale);

        if (text == "Preset")
        {
            y = group.proportionOfHeight (0.04f);
        }

        Font font (normalFont);
        font.setHeight (static_cast<float> (h));
        g.setFont (font);
        g.drawFittedText (text, x, y, w, h, Justification::left, 1);
    }

    Slider::SliderLayout getSliderLayout (Slider& slider) override
    {
        float scale = getScaleFactor (&slider);
        juce::Rectangle<int> localBounds (0, 0, slider.getWidth(), slider.getHeight());
        Slider::SliderLayout layout;

        if (slider.getSliderStyle() == Slider::SliderStyle::LinearHorizontal)
        {
            layout.sliderBounds.setBounds (
                static_cast<int> (static_cast<float> (localBounds.getWidth()) * 0.07f),
                localBounds.getY(),
                static_cast<int> (static_cast<float> (localBounds.getWidth()) * 0.42f),
                localBounds.getHeight());
            layout.textBoxBounds.setBounds (
                static_cast<int> (static_cast<float> (localBounds.getWidth()) * 0.71f),
                localBounds.getY(),
                static_cast<int> (static_cast<float> (localBounds.getWidth()) * 0.29f / scale),
                static_cast<int> (static_cast<float> (localBounds.getHeight()) / scale));
            layout.textBoxBounds.reduce (static_cast<int> (10.0f / scale),
                                         static_cast<int> (10.0f / scale));
        }
        else if (slider.getSliderStyle() == Slider::SliderStyle::LinearVertical)
        {
            auto layoutWidth =
                static_cast<float> (slider.getTopLevelComponent()->getWidth()) * 0.027f / scale;

            layout.sliderBounds.setBounds (
                static_cast<int> (static_cast<float> (localBounds.getRight()) - layoutWidth + 1),
                localBounds.getY(),
                static_cast<int> (layoutWidth),
                localBounds.getHeight());

            int textBoxWidth = static_cast<int> (
                static_cast<float> (slider.getTopLevelComponent()->getWidth()) * 0.05f / scale);
            int textBoxHeight = static_cast<int> (
                static_cast<float> (slider.getTopLevelComponent()->getHeight()) * 0.029f / scale);
            int textBoxX = layout.sliderBounds.getTopLeft().getX() - textBoxWidth + 3;
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
                                     [[maybe_unused]] float minSliderPos,
                                     [[maybe_unused]] float maxSliderPos,
                                     const Slider::SliderStyle style,
                                     Slider& slider) override
    {
        float scale = getScaleFactor (&slider);
        const auto h =
            static_cast<float> (slider.getTopLevelComponent()->getHeight()) * 0.005f / scale;
        const auto newDiameter =
            static_cast<float> (slider.getTopLevelComponent()->getHeight()) * 0.024f / scale;
        const auto fx = static_cast<float> (x);
        const auto fy = static_cast<float> (y);
        const auto fWidth = static_cast<float> (width);
        const auto fHeight = static_cast<float> (height);

        Path pathBgr;
        Path pathFrg;
        auto pathBgrColor =
            slider.isEnabled() ? textButtonFrameColor : textButtonFrameColor.withAlpha (0.4f);
        auto pathFrgColor = slider.isEnabled() ? textButtonActiveRedFrameColor
                                               : textButtonActiveRedFrameColor.withAlpha (0.2f);

        if (style == Slider::SliderStyle::LinearHorizontal)
        {
            Rectangle backgroundRect (fx, 0.5f * fHeight - h / 2, fWidth, h);
            pathBgr.addRoundedRectangle (backgroundRect, h);
            Rectangle foregroundRect (fWidth / 2.0f + fx,
                                      0.5f * fHeight - h / 2,
                                      sliderPos - (fWidth / 2.0f + fx),
                                      h);
            pathFrg.addRectangle (foregroundRect);
        }
        else if (style == Slider::SliderStyle::LinearVertical)
        {
            Rectangle backgroundRect (fx + fWidth / 2.0f - h / 2,
                                      fy + newDiameter / 2,
                                      h,
                                      fHeight - newDiameter);
            pathBgr.addRoundedRectangle (backgroundRect, h);
            auto mappedSliderPos = juce::jmap (sliderPos,
                                               0.f,
                                               fHeight,
                                               fy + newDiameter / 2,
                                               fHeight - newDiameter / 2);
            Rectangle foregroundRect (fx + fWidth / 2.0f - h / 2,
                                      fy + backgroundRect.getHeight() * 0.43f,
                                      h,
                                      mappedSliderPos - (fy + backgroundRect.getHeight() * 0.43f));
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
                                [[maybe_unused]] float minSliderPos,
                                [[maybe_unused]] float maxSliderPos,
                                const Slider::SliderStyle style,
                                Slider& slider) override
    {
        float scale = getScaleFactor (&slider);
        const float fx = static_cast<float> (x);
        const float fy = static_cast<float> (y);
        const float fWidth = static_cast<float> (width);
        const float fHeight = static_cast<float> (height);
        const float newDiameter =
            static_cast<float> (slider.getTopLevelComponent()->getHeight()) * 0.024f / scale;

        Path p;
        if (style == Slider::SliderStyle::LinearHorizontal)
        {
            p.addEllipse (sliderPos - newDiameter / 2,
                          fHeight / 2.f - newDiameter / 2.f,
                          newDiameter,
                          newDiameter);
        }
        else if (style == Slider::SliderStyle::LinearVertical)
        {
            auto mappedSliderPos = juce::jmap (sliderPos,
                                               0.f,
                                               static_cast<float> (height),
                                               fy + newDiameter / 2,
                                               fHeight - newDiameter / 2);
            p.addEllipse (fx + fWidth / 2.0f - newDiameter / 2,
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
            g.strokePath (p, juce::PathStrokeType (1.0f / scale));
        }
    }

    void drawLabel (Graphics& g, Label& label) override
    {
        float scale = getScaleFactor (&label);
        Rectangle labelArea (0.0f,
                             0.0f,
                             static_cast<float> (label.getWidth()),
                             static_cast<float> (label.getHeight()));

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
        int y = (static_cast<int> (labelArea.getHeight()) - h) / 2;

        Font font (normalFont);
        font.setHeight (static_cast<float> (h));
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
        Rectangle textEditorArea (0.0f,
                                  0.0f,
                                  static_cast<float> (textEditor.getWidth()),
                                  static_cast<float> (textEditor.getHeight()));
        g.setColour (textButtonHoverBackgroundColor);
        g.fillRect (textEditorArea);
    }

    void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor) override
    {
        juce::ignoreUnused (width, height);

        float scale = getScaleFactor (&textEditor);
        Rectangle textEditorArea (0.0f,
                                  0.0f,
                                  static_cast<float> (textEditor.getWidth()),
                                  static_cast<float> (textEditor.getHeight()));
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
        Rectangle toggleButtonBounds (0.0f,
                                      0.0f,
                                      static_cast<float> (button.getWidth()),
                                      static_cast<float> (button.getHeight()));

        Font font (normalFont);

        int x = static_cast<int> (toggleButtonBounds.getX());
        int w = static_cast<int> (toggleButtonBounds.getWidth());
        int h = static_cast<int> (static_cast<float> (button.getTopLevelComponent()->getHeight())
                                  * 0.023f / scale);
        int y = (button.getHeight() - h) / 2;

        font.setHeight (static_cast<float> (h));
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
            outline.addRoundedRectangle (
                toggleButtonBounds.reduced ((static_cast<float> (button.getWidth()) * 0.19f),
                                            (static_cast<float> (button.getHeight()) * 0.28f)),
                static_cast<float> (button.getHeight()) * 0.23f,
                static_cast<float> (button.getHeight()) * 0.23f);

            g.strokePath (outline, juce::PathStrokeType (2.0f));

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

        const float fx = static_cast<float> (x);
        const float fy = static_cast<float> (y);
        const float fWidth = static_cast<float> (width);
        const float fHeight = static_cast<float> (height);
        const float fThumbStart = static_cast<float> (thumbStartPosition);
        const float fThumbSize = static_cast<float> (thumbSize);

        Path pathBgr;
        pathBgr.addRoundedRectangle (fx, fy, fWidth, fHeight, (fWidth / 2.0f) / scale);
        g.setColour (textButtonFrameColor);
        g.fillPath (pathBgr);
        Path pathFgr;
        pathFgr.addRoundedRectangle (0.0f,
                                     fThumbStart,
                                     fWidth * 1.0f,
                                     fThumbSize,
                                     (fWidth / 2.0f) * 1.0f / scale);
        g.setColour (sliderHoverFrameColor);
        g.fillPath (pathFgr);
    }

private:
    void drawPatternImage (Graphics& g,
                           std::unique_ptr<Drawable> image,
                           Rectangle buttonArea,
                           int reduceX,
                           int reduceY,
                           int corner,
                           bool mouseOver,
                           bool mouseDown)
    {
        // Use global scale factor since no specific component is passed
        float scale = getScaleFactor (nullptr);
        g.setColour (labelBackgroundColor);

        float deltaX = 0;

        if (juce::SystemStats::getOperatingSystemName() == "iOS")
            deltaX = buttonArea.proportionOfWidth (0.24f);

        float deltaY = 1 / scale;
        g.fillRoundedRectangle (buttonArea.reduced (deltaX, deltaY),
                                static_cast<float> (corner) / scale);
        auto imageRect = buttonArea.reduced (static_cast<float> (reduceX) / scale,
                                             static_cast<float> (reduceY) / scale);
        if (mouseOver)
        {
            bool resultMainImg = image->replaceColour (juce::Colours::white, sliderHoverFrameColor);
            if (! resultMainImg)
                return;
        }
        if (mouseDown)
        {
            bool resultMainImg =
                image->replaceColour (sliderHoverFrameColor, juce::Colours::white.withAlpha (0.7f));
            if (! resultMainImg)
                return;
        }
        image->drawWithin (g, imageRect, juce::RectanglePlacement::centred, 1.f);
    }
};
