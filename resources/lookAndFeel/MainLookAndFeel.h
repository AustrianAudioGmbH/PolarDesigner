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
    const Colour textButtonHoverBackgroundColor = textButtonActiveFrameColor.withAlpha(0.3f);
    const Colour textButtonHoverRedBackgroundColor = textButtonActiveRedFrameColor.withAlpha(0.3f);
    const Colour textButtonPressedBackgroundColor = textButtonActiveFrameColor.withAlpha(0.1f);
    const Colour textButtonPressedRedBackgroundColor = textButtonActiveRedFrameColor.withAlpha(0.1f);
    const Colour toggleButtonActiveRedBackgroundColor = Colour(182, 22, 22).withAlpha(0.7f);

    Typeface::Ptr normalFont;

    MainLookAndFeel()
    {
        normalFont = Typeface::createSystemTypefaceFor(BinaryFonts::NunitoSansSemiBold_ttf, BinaryFonts::NunitoSansSemiBold_ttfSize);

        setColour(ListBox::backgroundColourId, groupComponentBackgroundColor);
    }

    ~MainLookAndFeel() {}

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
        //Sync channel buttons
        else if (button.getComponentID() == "5521")
        {
            if (isMouseOverButton)
            {
                g.setColour(textButtonActiveBlue1FrameColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonActiveBlue1FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(textButtonActiveBlue1FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(textButtonActiveBlue1FrameColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        else if(button.getComponentID() == "5522")
        {
            if (isMouseOverButton)
            {
                g.setColour(textButtonActiveBlue2FrameColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonActiveBlue2FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(textButtonActiveBlue2FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(textButtonActiveBlue2FrameColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5523")
        {
            if (isMouseOverButton)
            {
                g.setColour(textButtonActiveBlue3FrameColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonActiveBlue3FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(textButtonActiveBlue3FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(textButtonActiveBlue3FrameColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
        else if (button.getComponentID() == "5524")
        {
            if (isMouseOverButton)
            {
                g.setColour(textButtonActiveBlue4FrameColor.withAlpha(0.3f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (isButtonDown)
            {
                g.setColour(textButtonActiveBlue4FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(3.0f, 3.0f));
            }
            if (button.getToggleState() == true)
            {
                g.setColour(textButtonActiveBlue4FrameColor.withAlpha(0.1f));
                g.fillRect(buttonArea.reduced(4.0f, 4.0f));
                g.setColour(textButtonActiveBlue4FrameColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
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
        else
        {
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
                g.setColour(textButtonActiveRedFrameColor);
                g.drawRect(buttonArea.reduced(3.0f, 3.0f), 2);
            }
        }
    }

    void drawButtonText(Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        Rectangle<int> buttonArea(0, 0, button.getWidth(), button.getHeight());

        g.setColour(mainTextColor);

        int x = buttonArea.getX();
        int w = buttonArea.getWidth();
        int h = button.getTopLevelComponent()->getHeight() * 0.023f;
        int y = (buttonArea.getHeight() - h)/2;

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
            || button.getButtonText() == "Max Target-to-spill")
        {
            justification = Justification::centredLeft;
            x = buttonArea.proportionOfWidth(0.11f);
            w = buttonArea.proportionOfWidth(0.78f);
        }

        Font font(normalFont);
        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(button.getButtonText(), x, y, w, h, justification, 1);
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

        g.setColour(mainTextColor);

        int x = group.proportionOfWidth(0.06f);
        int y = 10.f;
        int w = group.proportionOfWidth(0.87f);
        int h = group.getTopLevelComponent()->getHeight()*0.025f;

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

        layout.sliderBounds.setBounds(localBounds.getWidth()*0.07f, localBounds.getY(), localBounds.getWidth()*0.42f, localBounds.getHeight());
        layout.textBoxBounds.setBounds(localBounds.getWidth() * 0.71f, localBounds.getY(), localBounds.getWidth()*0.29f, localBounds.getHeight());
        layout.textBoxBounds.reduce(10, 10);

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
        Rectangle<float> backgroundRect(x, 0.5f * height - h / 2, width, h);
        pathBgr.addRoundedRectangle(backgroundRect, h);
        auto pathBgrColor = slider.isEnabled() ? textButtonFrameColor : textButtonFrameColor.withAlpha(0.4f);
        g.setColour(pathBgrColor);
        g.fillPath(pathBgr);

        Path pathFrg;
        Rectangle<float> foregroundRect(width/2 + x, 0.5f * height - h / 2, sliderPos - (width / 2 + x), h);
        pathFrg.addRectangle(foregroundRect);
        auto pathFrgColor = slider.isEnabled() ? textButtonActiveRedFrameColor : textButtonFrameColor.withAlpha(0.4f);
        g.setColour(pathFrgColor);
        g.fillPath(pathFrg);
    }

    void drawLinearSliderThumb(Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider) override
    {
        const float newDiameter = slider.getTopLevelComponent()->getHeight() * 0.024f;

        Path p;
        p.addEllipse(sliderPos - newDiameter/2, height/2.f - newDiameter/2.f, newDiameter, newDiameter);

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

        auto textColor = label.isEnabled() ? mainTextColor : mainTextColor.withAlpha(0.4f);
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

        g.setColour(textButtonActiveRedFrameColor);
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

        g.setColour(mainTextColor);
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
        pathFgr.addRoundedRectangle(0, thumbStartPosition, width, thumbSize, width/2);
        g.setColour(sliderHoverFrameColor);
        g.fillPath(pathFgr);
    }
};