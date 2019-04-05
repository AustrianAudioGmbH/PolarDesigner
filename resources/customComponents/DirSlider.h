/*
 This file is based on ReverseSlider.h of the IEM plug-in suite.
 Modifications by Thomas Deppisch.
*/

/*
 ==============================================================================
 Author: Thomas Deppisch
 
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

/* Parts of this code originate from Yair Chuchem's AudioProcessorParameterSlider class:
 https://gist.github.com/yairchu */

#pragma once
#include "ImgPaths.h"

#define RS_FLT_EPSILON 1.19209290E-07F
class DirSlider : public Slider
{
public:
    DirSlider () :
        Slider(),
        lastDistanceFromDragStart(0),
        reversed(false),
        isDual(false),
        scrollWheelEnabled(true),
        tooltipActive(false),
        tooltipWidth(40),
        tooltipHeight(20),
        activePolarPatternPath(-1.0f),
        patternStripSize(12),
        dirStrip(this)
    {
        setTextBoxStyle (Slider::NoTextBox, false, 0, 0); // use tooltipValueBox instead
        setSliderStyle (Slider::LinearHorizontal);
        addAndMakeVisible(&dirStrip);
    }

    ~DirSlider () {}
    
    class DirPatternStrip : public Component
    {
    public:
        DirPatternStrip(DirSlider* newSlider) :
        Component(),
        dirImgSize(10),
        activePatternPath(-1.0f),
        slider(newSlider)
        {
            cardPath.loadPathFromData (cardData, sizeof (cardData));
            eightPath.loadPathFromData (eightData, sizeof (eightData));
            omniPath.loadPathFromData (omniData, sizeof (omniData));
            revCardPath.loadPathFromData (cardData, sizeof (cardData));
            revCardPath.applyTransform (AffineTransform::rotation(M_PI));
        }
        
        ~DirPatternStrip() {}
        
        void paint (Graphics& g) override
        {
            Rectangle<int> bounds = getLocalBounds();
            int lrMargin = 7;
            int topMargin = 1;
            int boundsX = bounds.getX() + lrMargin;
            int boundsY = bounds.getY() + topMargin;
            int width = bounds.getWidth() - 2*lrMargin;
            
            revCardPath.applyTransform (revCardPath.getTransformToScaleToFit(boundsX,
                                                                             boundsY, dirImgSize, dirImgSize,
                                                                             true, Justification::centred));
            (slider->isEnabled()) ? g.setColour (Colours::white) : g.setColour (Colours::white.withMultipliedAlpha(0.5f));
            g.strokePath (revCardPath, PathStrokeType (activePatternPath == revCardFact ? 2.0f : 1.0f));
            
            omniPath.applyTransform (omniPath.getTransformToScaleToFit(boundsX + 0.33 * width - dirImgSize/2.0f + 2.0f ,
                                                                       boundsY, dirImgSize, dirImgSize,
                                                                       true, Justification::centred));
            g.strokePath (omniPath, PathStrokeType (activePatternPath == omniFact ? 2.0f : 1.0f));

            cardPath.applyTransform (cardPath.getTransformToScaleToFit(boundsX + 0.66 * width - dirImgSize/2.0f - 1.0f,
                                                                       boundsY, dirImgSize, dirImgSize,
                                                                       true, Justification::centred));
            g.strokePath (cardPath, PathStrokeType (activePatternPath == cardFact ? 2.0f : 1.0f));

            eightPath.applyTransform (eightPath.getTransformToScaleToFit(boundsX + width - dirImgSize,
                                                                         boundsY, dirImgSize, dirImgSize,
                                                                         true, Justification::centred));
            g.strokePath (eightPath, PathStrokeType (activePatternPath == eightFact ? 2.0f : 1.0f));
        }
        
        void mouseMove(const MouseEvent &e) override
        {
            if (!slider->isEnabled())
                return;
            
            Point<float> posf = e.getPosition().toFloat();
            float oldActivePath = activePatternPath;
            activePatternPath = -1;
            
            // highlight active polar pattern path
            if (omniPath.getBounds().contains(posf)) activePatternPath = omniFact;
            else if (eightPath.getBounds().contains(posf)) activePatternPath = eightFact;
            else if (cardPath.getBounds().contains(posf)) activePatternPath = cardFact;
            else if (revCardPath.getBounds().contains(posf)) activePatternPath = revCardFact;
            
            if (oldActivePath != activePatternPath)
                repaint();
        }
        
        void mouseUp (const MouseEvent &e) override
        {
            if (!slider->isEnabled())
                return;
            
            if (activePatternPath != -1)
            {
                slider->setValue (activePatternPath, NotificationType::sendNotification);
            }
        }
        
        void mouseExit (const MouseEvent& e) override
        {
            activePatternPath = -1;
            repaint();
        }
        
    private:
        int dirImgSize;
        float activePatternPath;
        const float omniFact = 0.0f;
        const float eightFact = 1.0f;
        const float cardFact = 0.5f;
        const float revCardFact = -0.5f;
        
        Path cardPath;
        Path eightPath;
        Path omniPath;
        Path revCardPath;
        
        DirSlider* slider;
    };
    
    void paint (Graphics& g) override
    {
        auto& lf = getLookAndFeel();
        auto style = getSliderStyle();

        auto sliderPos = (float) valueToProportionOfLength (getValue());
        
        lf.drawLinearSlider (g,
                             sliderRect.getX(), sliderRect.getY(),
                             sliderRect.getWidth(), sliderRect.getHeight(),
                             getLinearSliderPos (sliderPos),
                             getLinearSliderPos (0.0f), getLinearSliderPos (0.0f),
                             style, *this);
        
        if (tooltipActive)
            tooltipValueBox->setVisible(true);

    }

    float getLinearSliderPos (double pos) const
    {
        return (float) (sliderRect.getX() + pos * sliderRect.getWidth());
    }
    
    void valueChanged() override
    {
        tooltipValueBox->setText(String(getValue(), 2), NotificationType::dontSendNotification);
    }

    void mouseDown (const MouseEvent& e) override
    {
        if (e.eventComponent != this)
            return; // mouseEvent started from tooltipValueBox
        
        lastDistanceFromDragStart = 0;
        Slider::mouseDown(e);
    }
    void mouseDrag (const MouseEvent& e) override
    {
        if (e.eventComponent != this)
            return;
        
        Slider::mouseDrag(e);
    }
    
    void mouseExit (const MouseEvent& e) override
    {
        tooltipActive = false;

        if (tooltipValueBox != nullptr && !tooltipValueBox->isMouseOver() && !tooltipValueBox->isBeingEdited())
            tooltipValueBox->setVisible(false);

    }
    
    void mouseEnter (const MouseEvent& e) override
    {
        if (e.eventComponent != this)
            return;
        
        if (tooltipValueBox != nullptr && isEnabled())
            tooltipActive = true;
    }

    void resized() override
    {
        Slider::resized();
        auto& lf = getLookAndFeel();
        auto layout = lf.getSliderLayout (*this);
        
        sliderRect = layout.sliderBounds;
        sliderRect.removeFromTop(patternStripSize);
        
        dirPatternBounds = getLocalBounds().removeFromTop(patternStripSize);
        dirStrip.setBounds(dirPatternBounds);
        
        initValueBox();
    }
    
    void initValueBox()
    {
        auto& lf = getLookAndFeel();
        
        tooltipValueBox.reset (lf.createSliderTextBox (*this));
        tooltipValueBox->addMouseListener(this, false);
        auto* parent = getParentComponent();
        if (parent != nullptr)
            parent->addChildComponent (tooltipValueBox.get());
        
        Rectangle<int> bounds = getBounds();
        
        tooltipValueBox->setBounds (bounds.getRight(), bounds.getY(), tooltipWidth, tooltipHeight);
        tooltipValueBox->setText (String (getValue(),2), NotificationType::dontSendNotification);
        tooltipValueBox->setAlwaysOnTop (true);
        tooltipValueBox->onTextChange = [this] { tooltipTextChanged(); };
        tooltipValueBox->onEditorHide = [this] { if (!tooltipActive)
                                                    tooltipValueBox->setVisible(false);
                                                };
    }
    
    void tooltipTextChanged()
    {
        if (getValueFromText (tooltipValueBox->getText()) == getValue())
            return;
        
        double newValue = snapValueToRange (getValueFromText (tooltipValueBox->getText()));
        
        if (newValue != static_cast<double> (getValue()))
        {
            setValue (newValue, NotificationType::sendNotification);
            tooltipValueBox->setText (getTextFromValue (newValue), NotificationType::dontSendNotification);
        }
        
        if (!tooltipActive)
            tooltipValueBox->setVisible(false);
    }
    
    double snapValueToRange(double attemptedValue)
    {
        return attemptedValue < getMinimum() ? getMinimum() : (attemptedValue > getMaximum() ? getMaximum() : attemptedValue);
    }
    
    void setTooltipEditable (bool shouldBeEditable)
    {
        if (tooltipValueBox != nullptr)
            tooltipValueBox->setEditable(shouldBeEditable);
    }

private:
    int lastDistanceFromDragStart;
    bool reversed;
    bool isDual;
    bool scrollWheelEnabled;
    bool tooltipActive;
    Rectangle<int> sliderRect;
    Rectangle<int> dirPatternBounds;
    
    int tooltipWidth;
    int tooltipHeight;
    float activePolarPatternPath;
    float patternStripSize;
    
    std::unique_ptr<Label> tooltipValueBox;
    
    DirPatternStrip dirStrip;

};
