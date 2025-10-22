/*
 This file is based on IEM_LaF.h of the IEM plug-in suite.
 Modifications by Thomas Deppisch.
*/

/*
 ============================================================================== 
 Copyright (c) 2019 - Austrian Audio GmbH
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

/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#pragma once

#include "BinaryFonts.h"

class LaF : public LookAndFeel_V4
{
public:
    //    const Colour AAGrey = Colour(180,180,185);
    const Colour AARed = Colour (155, 35, 35);
    const Colour ClBackground = Colour (0xFF2D2D2D);
    const Colour ClFace = Colour (0xFFD8D8D8);
    const Colour ClFaceShadow = Colour (0XFF272727);
    const Colour ClFaceShadowOutline = Colour (0xFF212121);
    //    const Colour ClFaceShadowOutlineActive = Colour(0xFF7C7C7C);
    const Colour ClRotSliderArrow = Colour (0xFF4A4A4A);
    const Colour ClRotSliderArrowShadow = Colour (0x445D5D5D);
    const Colour ClSliderFace = Colour (0xFF191919);
    const Colour ClText = Colour (0xFFFFFFFF);
    const Colour ClTextTextboxBackground = Colour (0xFF000000);
    const Colour ClSeparator = Colour (0xFF979797);
    //    const Colour ClWidgetColours[4] = {
    //        Colour(0xFF00CAFF), Colour(0xFF4FFF00), Colour(0xFFFF9F00), Colour(0xFFD0011B)
    //    };

    Typeface::Ptr aaLight, aaRegular, aaMedium, terminator;

    //float sliderThumbDiameter = 14.0f;
    //    float sliderBarSize = 8.0f;

    LaF()
    {
        aaLight = Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansLight_ttf,
                                                     BinaryFonts::NunitoSansLight_ttfSize);
        aaMedium = Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansRegular_ttf,
                                                      BinaryFonts::NunitoSansRegular_ttfSize);
        aaRegular = Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansSemiBold_ttf,
                                                       BinaryFonts::NunitoSansSemiBold_ttfSize);
        terminator = Typeface::createSystemTypefaceFor (BinaryFonts::terminator_ttf,
                                                        BinaryFonts::terminator_ttfSize);

        setColour (Slider::rotarySliderFillColourId, Colours::black);
        setColour (Slider::thumbColourId, Colour (0xCCFFFFFF));
        setColour (Slider::rotarySliderOutlineColourId,
                   Colours::transparentBlack); // colour of knob trace
        setColour (TextButton::buttonColourId, AARed);
        setColour (TextButton::buttonOnColourId, AARed);
        //        setColour (TextButton::textColourOnId, Colours::white);
        //        setColour (ResizableWindow::backgroundColourId, Colour(0xFF2D2D2D));
        //        setColour (ScrollBar::thumbColourId, Colours::steelblue);
        //        setColour (ScrollBar::thumbColourId, Colours::steelblue);
        setColour (PopupMenu::backgroundColourId, AARed);
        setColour (PopupMenu::highlightedBackgroundColourId, Colours::black);
        setColour (ToggleButton::tickColourId, AARed);
        //        setColour (ListBox::backgroundColourId, AARed);
        //        setColour (ListBox::outlineColourId, AARed);
        //
        //        setColour (TableHeaderComponent::backgroundColourId, Colours::lightgrey.withMultipliedAlpha(0.8f));
        //        setColour (TableHeaderComponent::highlightColourId, Colours::steelblue.withMultipliedAlpha(0.3f));
    }

    ~LaF() override = default;

    Typeface::Ptr getTypefaceForFont (const Font& f) override
    {
        switch (f.getStyleFlags())
        {
            case 2:
                return aaLight;
            case 1:
                return aaMedium;
            default:
                return aaRegular;
        }
    }
    Font getLabelFont (Label& label) override
    {
        (void) label;
        Font font (aaRegular);
        font.setHeight (18.0f);
        return font;
    }

    Font getPopupMenuFont() override
    {
        Font font (aaRegular);
        font.setHeight (12.0f);
        return font;
    }

    Font getTextButtonFont (TextButton& button, int height) override
    {
        (void) button;
        (void) height;
        Font font (aaRegular);
        font.setHeight (18.0f);
        return font;
    }

    Font getAlertWindowMessageFont() override
    {
        Font font (aaRegular);
        font.setHeight (16.0f);
        return font;
    }

    Font getAlertWindowTitleFont() override
    {
        Font font (terminator);
        font.setHeight (20.0f);
        return font;
    }

    Slider::SliderLayout getSliderLayout (Slider& slider) override
    {
        // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints

        int minXSpace = 0;
        int minYSpace = 0;

        Slider::TextEntryBoxPosition textBoxPos = slider.getTextBoxPosition();

        if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
            minXSpace = 30;
        else
            minYSpace = 15;

        if (slider.getSliderStyle() == Slider::IncDecButtons)
            minXSpace = 18;

        Rectangle<int> localBounds = slider.getLocalBounds();

        const int textBoxWidth =
            jmax (0, jmin (slider.getTextBoxWidth(), localBounds.getWidth() - minXSpace));
        const int textBoxHeight =
            jmax (0, jmin (slider.getTextBoxHeight(), localBounds.getHeight() - minYSpace));

        Slider::SliderLayout layout;

        // 2. set the textBox bounds

        if (textBoxPos != Slider::NoTextBox)
        {
            if (slider.isBar())
            {
                layout.textBoxBounds = localBounds;
            }
            else
            {
                layout.textBoxBounds.setWidth (textBoxWidth);
                layout.textBoxBounds.setHeight (textBoxHeight);

                if (textBoxPos == Slider::TextBoxLeft)
                    layout.textBoxBounds.setX (0);
                else if (textBoxPos == Slider::TextBoxRight)
                    layout.textBoxBounds.setX (localBounds.getWidth() - textBoxWidth);
                else /* above or below -> centre horizontally */
                    layout.textBoxBounds.setX ((localBounds.getWidth() - textBoxWidth) / 2);

                if (textBoxPos == Slider::TextBoxAbove)
                    layout.textBoxBounds.setY (0);
                else if (textBoxPos == Slider::TextBoxBelow)
                    layout.textBoxBounds.setY (localBounds.getHeight() - textBoxHeight);
                else /* left or right -> centre vertically */
                    layout.textBoxBounds.setY ((localBounds.getHeight() - textBoxHeight) / 2);
            }
        }

        // 3. set the slider bounds

        layout.sliderBounds = localBounds;

        if (slider.isBar())
        {
            layout.sliderBounds.reduce (1, 1); // bar border
        }
        else
        {
            int tbSpacing = 3;
            if (textBoxPos == Slider::TextBoxLeft)
                layout.sliderBounds.removeFromLeft (textBoxWidth + tbSpacing);
            else if (textBoxPos == Slider::TextBoxRight)
                layout.sliderBounds.removeFromRight (textBoxWidth + tbSpacing);
            else if (textBoxPos == Slider::TextBoxAbove)
                layout.sliderBounds.removeFromTop (textBoxHeight + tbSpacing);
            else if (textBoxPos == Slider::TextBoxBelow)
                layout.sliderBounds.removeFromBottom (textBoxHeight + tbSpacing);

            const int thumbIndent = getSliderThumbRadius (slider);

            if (slider.isHorizontal())
                layout.sliderBounds.reduce (thumbIndent, 0);
            else if (slider.isVertical())
                layout.sliderBounds.reduce (0, thumbIndent);
        }

        return layout;
    }

    void drawLabel (Graphics& g, Label& label) override
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.4f;
        g.fillAll (label.findColour (Label::backgroundColourId));
        Rectangle<int> bounds = label.getLocalBounds();
        auto x = (float) bounds.getX();
        auto y = (float) bounds.getY();
        auto w = (float) bounds.getWidth();
        auto h = (float) bounds.getHeight();
        Path p;
        p.addRoundedRectangle (x, y, w, h, h / 2.0f);
        g.setColour (ClTextTextboxBackground.withMultipliedAlpha (alpha));
        g.fillPath (p);

        if (! label.isBeingEdited())
        {
            const float editingAlpha = label.isEnabled() ? 1.0f : 0.5f;
            const Font font (aaRegular);

            //g.setColour (ClText.withMultipliedAlpha (alpha));
            g.setColour (ClText.withMultipliedAlpha (editingAlpha));
            g.setFont (getLabelFont (label));

            Rectangle<int> textArea (label.getBorderSize().subtractedFrom (label.getLocalBounds()));

            g.drawFittedText (label.getText(),
                              textArea,
                              label.getJustificationType(),
                              1,
                              label.getMinimumHorizontalScale());

            g.setColour (
                label.findColour (Label::outlineColourId).withMultipliedAlpha (editingAlpha));
        }
        else if (label.isEnabled())
        {
            g.setColour (label.findColour (Label::outlineColourId));
        }
        //g.drawRect (label.getLocalBounds());
        //g.drawRoundedRectangle (0,0,80,10,7.f,2);
    }

    void drawCornerResizer (Graphics& g,
                            int w,
                            int h,
                            bool /*isMouseOver*/,
                            bool /*isMouseDragging*/) override
    {
        g.setColour (Colours::white.withMultipliedAlpha (0.5f));

        Path triangle;
        triangle.startNewSubPath (static_cast<float> (w), static_cast<float> (h));
        triangle.lineTo (static_cast<float> (0.5 * w), static_cast<float> (h));
        triangle.lineTo (static_cast<float> (w), static_cast<float> (0.5 * h));
        triangle.closeSubPath();

        g.fillPath (triangle);
    }

    void fillTextEditorBackground (Graphics& g,
                                   int width,
                                   int height,
                                   TextEditor& textEditor) override
    {
        if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) != nullptr)
        {
            g.setColour (textEditor.findColour (TextEditor::backgroundColourId));
            g.fillRect (0, 0, width, height);

            g.setColour (textEditor.findColour (TextEditor::outlineColourId));
            g.drawHorizontalLine (height - 1, 0.0f, static_cast<float> (width));
        }
        else
        {
            Path p;
            p.addRoundedRectangle (static_cast<float> (0),
                                   static_cast<float> (0),
                                   static_cast<float> (width),
                                   static_cast<float> (height),
                                   12.0f);
            //g.setColour (ClTextTextboxBackground);
            g.setColour (textEditor.findColour (TextEditor::backgroundColourId));
            g.fillPath (p);
        }
    }

    void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor) override
    {
        if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) == nullptr)
        {
            if (textEditor.isEnabled())
            {
                if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
                {
                    g.setColour (Colours::white.withMultipliedAlpha (0.8f));
                    g.drawRoundedRectangle (0.5,
                                            0.5,
                                            static_cast<float> (width - 1),
                                            static_cast<float> (height - 1),
                                            static_cast<float> (height - 1) / 2.0f,
                                            0.8f);
                }
                else
                {
                    g.setColour (Colours::white.withMultipliedAlpha (0.8f));
                    g.drawRoundedRectangle (0,
                                            0,
                                            static_cast<float> (width),
                                            static_cast<float> (height),
                                            static_cast<float> (height) / 2.0f,
                                            0);
                }
            }
        }
    }

    void drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header) override
    {
        Rectangle<int> r (header.getLocalBounds());
        auto outlineColour = header.findColour (TableHeaderComponent::outlineColourId);

        g.setColour (outlineColour);
        g.fillRect (r.removeFromBottom (1));

        g.setColour (header.findColour (TableHeaderComponent::backgroundColourId));
        g.fillRect (r);

        g.setColour (outlineColour);

        for (int i = header.getNumColumns (true); --i >= 0;)
            g.fillRect (header.getColumnPosition (i).removeFromRight (1));
    }

    void drawTableHeaderColumn (Graphics& g,
                                TableHeaderComponent& header,
                                const String& columnName,
                                int /*columnId*/,
                                int width,
                                int height,
                                bool isMouseOver,
                                bool isMouseDown,
                                int columnFlags) override
    {
        auto highlightColour = header.findColour (TableHeaderComponent::highlightColourId);

        if (isMouseDown)
            g.fillAll (highlightColour);
        else if (isMouseOver)
            g.fillAll (highlightColour.withMultipliedAlpha (0.625f));

        Rectangle<int> area (width, height);
        area.reduce (4, 0);

        if ((columnFlags
             & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards))
            != 0)
        {
            Path sortArrow;
            sortArrow.addTriangle (0.0f,
                                   0.0f,
                                   0.5f,
                                   (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f
                                                                                             : 0.8f,
                                   1.0f,
                                   0.0f);

            g.setColour (Colour (0x99000000));
            g.fillPath (sortArrow,
                        sortArrow.getTransformToScaleToFit (
                            area.removeFromRight (height / 2).reduced (2).toFloat(),
                            true));
        }

        g.setColour (header.findColour (TableHeaderComponent::textColourId));
        g.setFont (aaRegular);
        g.setFont (static_cast<float> (height) * 0.6f);
        g.drawFittedText (columnName, area, Justification::centred, 1);
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
        //g.fillAll (slider.findColour (Slider::backgroundColourId));

        //Label* l = createSliderTextBox(slider);
        //l->showEditor();

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            const auto fx = (float) x, fy = (float) y, fw = (float) width, fh = (float) height;

            Path p;

            if (style == Slider::LinearBarVertical)
                p.addRectangle (fx, sliderPos, fw, 1.0f + fh - sliderPos);
            else
                p.addRectangle (fx, fy, sliderPos - fx, fh);

            Colour baseColour (slider.findColour (Slider::rotarySliderFillColourId)
                                   .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                   .withMultipliedAlpha (1.0f));

            g.setColour (baseColour);
            g.fillPath (p);

            const auto lineThickness =
                jmin (15.0f, static_cast<float> (jmin (width, height)) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
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
        const float sliderRadius = 8.f; //getSliderThumbRadius (slider) - 5.0f;
        Path sliderBackgroundPath;
        Path controlBarPath;

        Colour statusColour =
            slider.findColour (Slider::rotarySliderOutlineColourId).withMultipliedAlpha (0.8f);

        const double min = slider.getMinimum();
        const double max = slider.getMaximum();
        const auto zeroPos = -min / (max - min);
        bool isTwoValue = (style == Slider::SliderStyle::TwoValueVertical
                           || style == Slider::SliderStyle::TwoValueHorizontal);

        if (slider.isHorizontal())
        {
            const auto iy = static_cast<float> (
                static_cast<float> (y) + static_cast<float> (height) * 0.5f - sliderRadius * 0.5f);
            Rectangle<float> r (static_cast<float> (static_cast<float> (x) - sliderRadius * 0.5f),
                                iy,
                                static_cast<float> (static_cast<float> (width) + sliderRadius),
                                sliderRadius);
            sliderBackgroundPath.addRoundedRectangle (r,
                                                      static_cast<float> (sliderRadius / 2.0),
                                                      static_cast<float> (sliderRadius / 2.0));

            if (isTwoValue)
            {
                controlBarPath.addRoundedRectangle (
                    Rectangle<float> (Point<float> (minSliderPos, iy),
                                      Point<float> (maxSliderPos, iy + sliderRadius)),
                    static_cast<float> (sliderRadius / 2.0),
                    static_cast<float> (sliderRadius / 2.0));
            }
            else
            {
                controlBarPath.addRoundedRectangle (
                    Rectangle<float> (Point<float> (static_cast<float> (x + width * (zeroPos)), iy),
                                      Point<float> (sliderPos, iy + sliderRadius)),
                    static_cast<float> (sliderRadius / 2.0),
                    static_cast<float> (sliderRadius / 2.0));
            }
        }
        else
        {
            const auto ix = static_cast<float> (
                static_cast<float> (x) + static_cast<float> (width) * 0.5f - sliderRadius * 0.5f);
            Rectangle<float> r (ix,
                                static_cast<float> (y) - sliderRadius * 0.5f,
                                sliderRadius,
                                static_cast<float> (height) + sliderRadius);
            sliderBackgroundPath.addRoundedRectangle (r,
                                                      static_cast<float> (sliderRadius / 2.0),
                                                      static_cast<float> (sliderRadius / 2.0));
            controlBarPath.addRoundedRectangle (
                Rectangle<float> (
                    Point<float> (ix + 1.0f, static_cast<float> (y + height * (1.0f - zeroPos))),
                    Point<float> (ix - 1.0f + sliderRadius, sliderPos)),
                static_cast<float> (sliderRadius / 2.0),
                static_cast<float> (sliderRadius / 2.0));
        }

        g.setColour (ClSliderFace);
        g.fillPath (sliderBackgroundPath);
        g.setColour (statusColour);
        g.fillPath (controlBarPath);
        g.setColour (ClFaceShadowOutline);

        g.strokePath (sliderBackgroundPath, PathStrokeType (1.0f));
    }

    void drawRotarySlider (Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           Slider& slider) override
    {
        drawRotarySliderDual (g,
                              x,
                              y,
                              width,
                              height,
                              sliderPos,
                              rotaryStartAngle,
                              rotaryEndAngle,
                              slider,
                              false);
    }

    void drawRotarySliderDual (Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPos,
                               float rotaryStartAngle,
                               float rotaryEndAngle,
                               Slider& slider,
                               bool isDual)
    {
        bool isEnabled = slider.isEnabled();
        const float alpha = isEnabled ? 1.0f : 0.4f;
        const auto radius = static_cast<const float> (jmin (width / 2, height / 2));
        const float centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
        const float centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;

        const auto min = static_cast<const float> (slider.getMinimum());
        const auto max = static_cast<const float> (slider.getMaximum());
        const float zeroPos = -min / (max - min);
        const float zeroAngle = rotaryStartAngle + zeroPos * (rotaryEndAngle - rotaryStartAngle);
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const float negAngle =
            rotaryStartAngle + (2 * zeroPos - sliderPos) * (rotaryEndAngle - rotaryStartAngle);

        const float bedThickness = 2.0f;
        const float bedOutline = 1.4f;
        const float statusOutline = 1.6f;
        const float extraMargin = 1.0f;

        const float pointerThickness = 1.2f;
        const float pointerLength =
            (radius - extraMargin - statusOutline - bedOutline - bedThickness - 1.0f) * 0.8f;

        Path p, q, a;
        Rectangle<float> r = Rectangle<float> (rx, ry, rw, rw);

        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        const Colour statusColour = slider.findColour (Slider::rotarySliderOutlineColourId);
        //status ring
        g.setColour (statusColour.withMultipliedAlpha (alpha));

        a.addCentredArc (centreX,
                         centreY,
                         radius - extraMargin,
                         radius - extraMargin,
                         0.0f,
                         rotaryStartAngle,
                         angle,
                         true);
        if (isDual)
            a.addCentredArc (centreX,
                             centreY,
                             radius - extraMargin,
                             radius - extraMargin,
                             0.0f,
                             negAngle,
                             zeroAngle,
                             true);
        g.strokePath (a, PathStrokeType (statusOutline));

        //bed ellipse
        g.setColour (ClFaceShadow);
        g.fillEllipse (r.reduced (extraMargin + statusOutline));

        //(isMouseOver)?g.setColour(ClFaceShadowOutlineActive) : g.setColour (ClFaceShadowOutline);
        (isMouseOver) ? g.setColour (statusColour.withMultipliedAlpha (0.4f))
                      : g.setColour (ClFaceShadowOutline);
        g.drawEllipse (r.reduced (extraMargin + statusOutline), bedOutline);

        //knob
        g.setColour (ClFace.withMultipliedAlpha (alpha));
        g.fillEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness));
        g.setColour (statusColour.withMultipliedAlpha (alpha));
        g.drawEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness),
                       statusOutline);

        g.setColour (ClRotSliderArrowShadow.withMultipliedAlpha (alpha));
        g.drawEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness + 1.0f),
                       1.0f);

        q.addRectangle (pointerThickness * 0.3f, -radius + 6.0f, pointerThickness, pointerLength);
        q.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (ClRotSliderArrowShadow.withMultipliedAlpha (alpha));
        g.fillPath (q);

        p.addRectangle (-pointerThickness * 0.5f, -radius + 6.0f, pointerThickness, pointerLength);
        p.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (ClRotSliderArrow.withMultipliedAlpha (alpha));
        g.fillPath (p);
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
        const float sliderRadius = 7.0f;

        //bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
        //Colour knobColour (slider.findColour (Slider::thumbColourId).withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
        // .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f));
        Colour knobColour = slider.findColour (Slider::thumbColourId)
                                .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);
        const float outlineThickness = slider.isEnabled() ? 1.9f : 0.3f;

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            float kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = static_cast<float> (x + width) * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = static_cast<float> (y + height) * 0.5f;
            }

            drawRoundThumb (g, kx, ky, sliderRadius * 2.0f, knobColour, outlineThickness);
        }
        else if (style == Slider::TwoValueVertical)
        {
            drawRoundThumb (g,
                            jmax (sliderRadius, static_cast<float> (x + width) * 0.5f),
                            minSliderPos,
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);

            drawRoundThumb (g,
                            jmax (sliderRadius, static_cast<float> (x + width) * 0.5f),
                            maxSliderPos,
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);
        }
        else if (style == Slider::TwoValueHorizontal)
        {
            drawRoundThumb (g,
                            minSliderPos,
                            jmax (sliderRadius, static_cast<float> (y + height) * 0.5f),
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);

            drawRoundThumb (g,
                            maxSliderPos,
                            jmax (sliderRadius, static_cast<float> (y + height) * 0.5f),
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g,
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
    }
    void drawRoundThumb (Graphics& g,
                         const float centreX,
                         const float centreY,
                         const float diameter,
                         const Colour& colour,
                         float outlineThickness)
    {
        //        const Rectangle<float> a (x, y, diameter, diameter);

        const float newDiameter = (diameter - outlineThickness);
        const float halfThickness = newDiameter * 0.5f;

        Path p;
        p.addEllipse (centreX - halfThickness, centreY - halfThickness, newDiameter, newDiameter);

        g.setColour (ClFace);
        g.fillPath (p);

        g.setColour (colour);
        g.strokePath (p, PathStrokeType (outlineThickness));

        g.setColour (ClRotSliderArrowShadow);
        g.drawEllipse (centreX + 1.0f - halfThickness,
                       centreY + 1.0f - halfThickness,
                       diameter - outlineThickness - 1.0f,
                       diameter - outlineThickness - 1.0f,
                       1.4f);
    }

    Button* createSliderButton (Slider&, const bool isIncrement) override
    {
        return new TextButton (isIncrement ? "+" : "-", String());
        //return new ArrowButton (String(),isIncrement ? 0.75 : 0.25f,Colours::white);
    }

    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        Rectangle<float> buttonArea (0.0f,
                                     0.0f,
                                     static_cast<float> (button.getWidth()),
                                     static_cast<float> (button.getHeight()));
        buttonArea.reduce (1.0f, 1.0f);

        g.setColour (backgroundColour);
        if (isButtonDown)
            buttonArea.reduce (0.8f, 0.8f);
        else if (isMouseOverButton)
            buttonArea.reduce (0.4f, 0.4f);

        g.drawRoundedRectangle (buttonArea, 2.0f, 1.0f);

        buttonArea.reduce (1.5f, 1.5f);

        if (button.getButtonText() == "zero latency")
        {
            g.setColour (
                backgroundColour.withMultipliedAlpha (button.getToggleState() ? 1.0f : 0.4f));

            g.fillRoundedRectangle (buttonArea, 2.0f);
        }
        else
        {
            g.setColour (backgroundColour.withMultipliedAlpha (isButtonDown        ? 1.0f
                                                               : isMouseOverButton ? 0.6f
                                                                                   : 0.9f));

            g.fillRoundedRectangle (buttonArea, 2.0f);
        }
    }

    void drawButtonText (Graphics& g,
                         TextButton& button,
                         bool /*isMouseOverButton*/,
                         bool /*isButtonDown*/) override
    {
        Font font (getTextButtonFont (button, button.getHeight()));
        g.setFont (font);
        g.setColour (button
                         .findColour (button.getToggleState() ? TextButton::textColourOnId
                                                              : TextButton::textColourOffId)
                         .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

        const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
        const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

        const int fontHeight = roundToInt (font.getHeight() * 0.6f);
        const int leftIndent =
            jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
        const int rightIndent =
            jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
        const int textWidth = button.getWidth() - leftIndent - rightIndent;

        if (textWidth > 0)
            g.drawFittedText (button.getButtonText(),
                              leftIndent,
                              yIndent,
                              textWidth,
                              button.getHeight() - yIndent * 2,
                              Justification::centred,
                              1);
    }

    void drawToggleButton (Graphics& g,
                           ToggleButton& button,
                           bool isMouseOverButton,
                           bool isButtonDown) override
    {
        if (button.getButtonText() == "ON/OFF")
        {
            Colour baseColour (
                Colours::black
                    .withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                    .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.8f));

            const auto width = static_cast<const float> (button.getWidth());
            const auto height = static_cast<const float> (button.getHeight());
            bool isOn = button.getToggleState();
            const float cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);

            Path outline;
            outline.addRoundedRectangle (0.5f, 0.5f, width - 1, height - 1, cornerSize, cornerSize);

            g.setColour (baseColour);
            g.fillPath (outline);
            if (isMouseOverButton)
            {
                g.setColour (button.findColour (ToggleButton::tickColourId)
                                 .withMultipliedAlpha (isButtonDown ? 0.8f : 0.6f));
                g.strokePath (outline, PathStrokeType (isButtonDown ? 1.0f : 0.8f));
            }
            g.setFont (aaRegular);
            g.setFont (height - 1);
            g.setColour (isOn ? button.findColour (ToggleButton::tickColourId) : Colours::white);
            g.drawText (isOn ? "ON" : "OFF",
                        0,
                        0,
                        (int) width,
                        (int) height,
                        Justification::centred);
        }

        else
        {
            const auto fontSize = jmin (15.0f, static_cast<float> (button.getHeight()) * 0.75f);
            const auto tickWidth = fontSize * 1.1f;

            drawTickBox (g,
                         button,
                         4.0f,
                         (static_cast<float> (button.getHeight()) - tickWidth) * 0.5f,
                         tickWidth,
                         tickWidth,
                         button.getToggleState(),
                         button.isEnabled(),
                         isMouseOverButton,
                         isButtonDown);

            g.setColour (button.findColour (ToggleButton::textColourId));
            g.setFont (fontSize);

            if (! button.isEnabled())
                g.setOpacity (0.5f);

            g.setFont (aaRegular);
            g.setFont (18.0f);
            g.drawFittedText (button.getButtonText(),
                              button.getLocalBounds()
                                  .withTrimmedLeft (roundToInt (tickWidth) + 10)
                                  .withTrimmedRight (2),
                              Justification::centredLeft,
                              1);
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
        Rectangle<float> buttonArea (x, y, w, h);

        g.setColour (component.findColour (ToggleButton::tickColourId)
                         .withMultipliedAlpha (! isEnabled         ? 0.1f
                                               : ticked            ? 1.0f
                                               : isMouseOverButton ? 0.7f
                                                                   : 0.5f));

        if (isButtonDown)
            buttonArea.reduce (0.8f, 0.8f);
        else if (isMouseOverButton)
            buttonArea.reduce (0.4f, 0.4f);

        g.drawRoundedRectangle (buttonArea, 2.0f, 1.0f);

        buttonArea.reduce (1.5f, 1.5f);
        g.setColour (component.findColour (ToggleButton::tickColourId)
                         .withMultipliedAlpha (! isEnabled         ? 0.1f
                                               : ticked            ? 1.0f
                                               : isMouseOverButton ? 0.5f
                                                                   : 0.2f));

        g.fillRoundedRectangle (buttonArea, 2.0f);
    }

    Path getTickShape (const float height) override
    {
        Path p;
        Path stroke;
        stroke.addRoundedRectangle (Rectangle<float> (-0.5f, -5.0f, 1.0f, 10.0f), 0.1f, 0.1f);
        p.addPath (stroke, AffineTransform().rotation (static_cast<float> (0.25f * M_PI)));
        p.addPath (stroke, AffineTransform().rotation (static_cast<float> (-0.25f * M_PI)));
        p.scaleToFit (0, 0, height * 2.0f, height, true);
        return p;
    }

    void drawGroupComponentOutline (Graphics& g,
                                    int width,
                                    int height,
                                    const String& text,
                                    const Justification& position,
                                    GroupComponent& group) override
    {
        (void) height;
        (void) group;
        Rectangle<int> r (6, 0, width - 6, 15);
        g.setColour (ClText);
        g.setFont (aaRegular);
        g.setFont (20.0f);
        g.drawFittedText (text, r, position, 1, 0.f);

        g.setColour (ClSeparator);
        g.drawLine (0, 18, static_cast<float> (width), 18, .8f);
    }
    void positionComboBoxText (ComboBox& box, Label& label) override
    {
        label.setBounds (0, 0, box.getWidth() - box.getHeight(), box.getHeight());

        Font font (aaRegular);
        font.setHeight (18.0f);
        label.setFont (font);
    }

    void drawComboBox (Graphics& g,
                       int width,
                       int height,
                       bool isButtonDown,
                       int buttonX,
                       int buttonY,
                       int buttonW,
                       int buttonH,
                       ComboBox& box) override
    {
        (void) width;
        (void) height;
        (void) isButtonDown;
        //const auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        //        const Rectangle<int> boxBounds (0, 0, width, height);
        //
        //        g.setColour (box.findColour (ComboBox::backgroundColourId));
        //        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);
        //
        //        g.setColour (box.findColour (ComboBox::outlineColourId));
        //        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        Rectangle<int> buttonArea (buttonX, buttonY, buttonW, buttonH);
        Path path;
        path.startNewSubPath (static_cast<float> (buttonX) + 3.0f,
                              static_cast<float> (buttonArea.getCentreY()) - 2.0f);
        path.lineTo (static_cast<float> (buttonArea.getCentreX()),
                     static_cast<float> (buttonArea.getCentreY()) + 3.0f);
        path.lineTo (static_cast<float> (buttonArea.getRight()) - 3.0f,
                     static_cast<float> (buttonArea.getCentreY()) - 2.0f);

        g.setColour (Colours::white.withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
        g.strokePath (path, PathStrokeType (2.0f));
    }

    void drawPopupMenuSectionHeader (Graphics& g,
                                     const Rectangle<int>& area,
                                     const String& sectionName) override
    {
        g.setFont (aaRegular);
        g.setFont (18.0f);
        g.setColour (findColour (PopupMenu::headerTextColourId));

        g.drawFittedText (sectionName,
                          area.getX() + 12,
                          area.getY(),
                          area.getWidth() - 16,
                          (int) (static_cast<float> (area.getHeight()) * 0.8f),
                          Justification::bottomLeft,
                          1);
    }

    void drawPopupMenuItem (Graphics& g,
                            const Rectangle<int>& area,
                            const bool isSeparator,
                            const bool isActive,
                            const bool isHighlighted,
                            const bool isTicked,
                            const bool hasSubMenu,
                            const String& text,
                            const String& shortcutKeyText,
                            const Drawable* icon,
                            const Colour* const textColourToUse) override
    {
        if (isSeparator)
        {
            Rectangle<int> r (area.reduced (5, 0));
            r.removeFromTop (r.getHeight() / 2 - 1);

            g.setColour (Colour (0x33000000));
            g.fillRect (r.removeFromTop (1));

            g.setColour (Colour (0x66ffffff));
            g.fillRect (r.removeFromTop (1));
        }
        else
        {
            Colour textColour (findColour (PopupMenu::textColourId));

            if (textColourToUse != nullptr)
                textColour = *textColourToUse;

            Rectangle<int> r (area.reduced (1));

            if (isHighlighted)
            {
                g.setColour (findColour (PopupMenu::highlightedBackgroundColourId));
                g.fillRect (r);

                g.setColour (findColour (PopupMenu::highlightedTextColourId));
            }
            else
            {
                g.setColour (textColour);
            }

            if (! isActive)
                g.setOpacity (0.3f);

            Font font (getPopupMenuFont());
            font.setHeight (18.0f);

            const float maxFontHeight = static_cast<float> (area.getHeight()) / 1.3f;

            if (font.getHeight() > maxFontHeight)
                font.setHeight (maxFontHeight);

            g.setFont (font);

            Rectangle<float> iconArea (
                r.removeFromLeft ((r.getHeight() * 5) / 4).reduced (3).toFloat());

            if (icon != nullptr)
            {
                icon->drawWithin (g,
                                  iconArea,
                                  RectanglePlacement::centred
                                      | RectanglePlacement::onlyReduceInSize,
                                  1.0f);
            }
            else if (isTicked)
            {
                const Path tick (getTickShape (1.0f));
                g.fillPath (tick, tick.getTransformToScaleToFit (iconArea.reduced (4), true));
            }

            if (hasSubMenu)
            {
                const float arrowH = 0.6f * getPopupMenuFont().getAscent();

                const float x = (float) r.removeFromRight ((int) arrowH).getX();
                const auto halfH = static_cast<float> (r.getCentreY());

                Path p;
                p.addTriangle (x,
                               halfH - arrowH * 0.5f,
                               x,
                               halfH + arrowH * 0.5f,
                               x + arrowH * 0.6f,
                               halfH);

                g.fillPath (p);
            }

            r.removeFromRight (3);
            g.drawFittedText (text, r, Justification::centredLeft, 1);

            if (shortcutKeyText.isNotEmpty())
            {
                Font f2 (font);
                f2.setHeight (f2.getHeight() * 0.75f);
                f2.setHorizontalScale (0.95f);
                g.setFont (f2);

                g.drawText (shortcutKeyText, r, Justification::centredRight, true);
            }
        }
    }

    void drawCallOutBoxBackground (CallOutBox& box,
                                   Graphics& g,
                                   const Path& path,
                                   Image& cachedImage) override
    {
        if (cachedImage.isNull())
        {
            cachedImage = { Image::ARGB, box.getWidth(), box.getHeight(), true };
            Graphics g2 (cachedImage);

            DropShadow (Colours::black.withAlpha (0.7f), 8, { 0, 2 }).drawForPath (g2, path);
        }

        g.setColour (Colours::black);
        g.drawImageAt (cachedImage, 0, 0);

        g.setColour (ClBackground.withAlpha (0.8f));
        g.fillPath (path);

        g.setColour (Colours::white.withAlpha (0.8f));
        g.strokePath (path, PathStrokeType (1.0f));
    }
};
