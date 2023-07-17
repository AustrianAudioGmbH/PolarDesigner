/*
 This file is based on FilterVisualizer.h of the IEM plug-in suite.
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

#pragma once
#include "../resources/lookAndFeel/MainLookAndFeel.h"

class  DirectivityEQ : public Component, private Slider::Listener, private Label::Listener, private Button::Listener
{
    struct Settings {
        float fMin = 20.0f;    // minimum displayed frequency
        float fMax = 20000.0f; // maximum displayed frequency
        float yMin = -0.5f;
        float yMax = 1.0f;
        float gridDiv = 0.5;
    };

    struct BandElements {
        Slider* dirSlider = nullptr;
        Slider* lowerFrequencySlider = nullptr;
        Slider* upperFrequencySlider = nullptr;
        ToggleButton* soloButton = nullptr;
        ToggleButton* muteButton = nullptr;
        Colour colour;
        Slider* gainSlider = nullptr;
        PolarPatternVisualizer* polarPatternVisualizer = nullptr;

        Point<int> handlePos;
    };

    // margins
    float mL = 43.0f;
    const float mR = 15.0f;
    const float mT = 30.0f;
    float mB = 20.0f;
    const float OH = 3.0f;
    const float mLabel = 5.0f;

    class PathComponent : public Component
    {
    public:
        PathComponent() : Component() {
            setAlwaysOnTop(true);
            setName("PathComponent");
            path.preallocateSpace(1000);    // !J! Arbitrary Magic Number
        };
        ~PathComponent() {};
        void setBounds() { Component::setBounds(path.getBounds().toNearestInt()); }
        Path& getPath() { return path; }
    private:
        Path path;
    };

    class BandLimitDividerHolder : public Component
    {
    public:
        BandLimitDividerHolder() : Component() {
            setAlwaysOnTop(true);
            setName("PathComponent");
            path.preallocateSpace(1000);
        };
        ~BandLimitDividerHolder() {};
        void setBounds() { Component::setBounds(path.getBounds().toNearestInt()); }
        Path& getPath() { return path; }
    private:
        Path path;
    };

    class RectangleComponent : public Component
    {
    public:
        RectangleComponent() : Component() {
            setAlwaysOnTop(true);
            setName("RectangleComponent");
        };
        ~RectangleComponent() {};

        void setBounds(float x, float y, float width, float height)
        {
            rectangle.setBounds (x, y, width, height);
            Component::setBounds (rectangle.toNearestInt());
        }
    private:
        Rectangle<float> rectangle;
    };

    class BandKnobComponent : public Component
    {
    public:
        BandKnobComponent() : Component() {
            setAlwaysOnTop(true);
            setName("BandKnobComponent");
        };
        ~BandKnobComponent() {};
        void paint(Graphics& g) override
        {
            int circX = getLocalBounds().getCentreX();
            int circY = getLocalBounds().getCentreY();
            int circleSize = getLocalBounds().getWidth();

            auto bandHandleKnobImg = juce::Drawable::createFromImageData(BinaryData::bandHandleKnob_svg, BinaryData::bandHandleKnob_svgSize);
            auto bandHandleKnobImageArea = Rectangle<float>(circX - (circleSize / 2), circY - (circleSize / 2), circleSize, circleSize);

            if (!isEnabled())
            {
                bool resultMainImg = bandHandleKnobImg->replaceColour(Colours::white, Colours::black);
            }
            else
            {
                bool resultMainImg = bandHandleKnobImg->replaceColour(Colours::black, Colours::white);
            }
            bandHandleKnobImg->drawWithin(g, bandHandleKnobImageArea, juce::RectanglePlacement::centred, 1.f);
        }

        void setBounds(float x, float y, float width, float height)
        {
            rectangle.setBounds(x, y, width, height);
            Component::setBounds(rectangle.toNearestInt());
        }

    private:
        Rectangle<float> rectangle;
    };

public:
    DirectivityEQ(PolarDesignerAudioProcessor& p) :
                 Component(), processor(p), nrActiveBands(0), soloActive(false),
                 dirFactArray{omniFact, cardFact, rCardFact, eightFact,
                             bCardFact, rbCardFact, sCardFact, hCardFact}
    {
        setLookAndFeel(&mainLaF);

        addAndMakeVisible(&tbPrimDirButtons[0]);
        tbPrimDirButtons[0].setButtonText("Eight Pattern");
        tbPrimDirButtons[0].addListener(this);
        addAndMakeVisible(&tbPrimDirButtons[1]);
        tbPrimDirButtons[1].setButtonText("Cardioid Pattern");
        tbPrimDirButtons[1].addListener(this);
        addAndMakeVisible(&tbPrimDirButtons[2]);
        tbPrimDirButtons[2].setButtonText("Omni Pattern");
        tbPrimDirButtons[2].addListener(this);
        addAndMakeVisible(&tbPrimDirButtons[3]);
        tbPrimDirButtons[3].setButtonText("RevCardioid Pattern");
        tbPrimDirButtons[3].addListener(this);

        addAndMakeVisible(&tbSecDirButtons[0]);
        tbSecDirButtons[0].setButtonText("HyperCardioid Pattern");
        tbSecDirButtons[0].addListener(this);
        addAndMakeVisible(&tbSecDirButtons[1]);
        tbSecDirButtons[1].setButtonText("SuperCardioid Pattern");
        tbSecDirButtons[1].addListener(this);
        addAndMakeVisible(&tbSecDirButtons[2]);
        tbSecDirButtons[2].setButtonText("BCardioid Pattern");
        tbSecDirButtons[2].addListener(this);
        addAndMakeVisible(&tbSecDirButtons[3]);
        tbSecDirButtons[3].setButtonText("RevBCardioid Pattern");
        tbSecDirButtons[3].addListener(this);

        init();

        for (int i = 0; i < 4; ++i)
        {
            addAndMakeVisible (&bandLimitPaths[i]);
            bandLimitPaths[i].addMouseListener(this, true);
            addAndMakeVisible(&bandLimitDividerHolders[i]);
            bandLimitDividerHolders[i].addMouseListener(this, true);
        }

        for (int i = 0; i < 5; ++i)
        {
            addAndMakeVisible (&dirPathRects[i]);
            dirPathRects[i].addMouseListener(this, true);
            addAndMakeVisible(&bandKnobs[i]);
            bandKnobs[i].addMouseListener(this, true);
        }
    };

    void init ()
    {
        dyn = s.yMax - s.yMin;
        zero = s.yMax / dyn;
    }
    
    ~DirectivityEQ() {
        setLookAndFeel(nullptr);
    };

    void paint (Graphics& g) override
    {
        nrActiveBands = processor.getNBands();

        if (processor.zeroDelayModeActive())
            nrActiveBands = 1;

        // make sure visibility of paths for grabbing is on for active bands and if no mouse dragging occurs
        if (nrActiveBands != oldNrActiveBands && activeElem == -1)
        {
            for (int i = 0; i < 5; ++i)
            {
                if (i < nrActiveBands)
                {
                    dirPathRects[i].setVisible(true);
                    bandKnobs[i].setVisible(true);
                }
                else
                {
                    dirPathRects[i].setVisible(false);
                    bandKnobs[i].setVisible(false);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                if (i < nrActiveBands - 1)
                {
                    bandLimitPaths[i].setVisible(true);
                    bandLimitDividerHolders[i].setVisible(true);
                } 
                else
                {
                    bandLimitPaths[i].setVisible(false);
                    bandLimitDividerHolders[i].setVisible(false);
                }
            }
        }

        // frequency labels
        Font axisLabelFont = getLookAndFeel().getTypefaceForFont(Font(12.0f, 1));
        g.setFont (axisLabelFont);
        g.setFont (getTopLevelComponent()->getHeight()*0.017f);
        g.setColour(mainLaF.mainTextColor);
        for (float f=s.fMin; f <= s.fMax; f += powf(10, floorf(log10(f))))
        {
            int xpos = hzToX(f);

            String axislabel;
            bool drawText = false;

            if ((f == 20) || (f == 50) || (f == 100) || (f == 200) || (f == 500))
            {
                axislabel = String((int)f);
                drawText = true;
            }
            else if ((f == 1000) || (f == 2000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                axislabel = String((int)f/1000);
                axislabel << "k";
                drawText = true;
            }

            if (drawText)
            {
                g.drawText (axislabel, xpos - axisLabelFont.getStringWidth(axislabel)/2 - mLabel, dirToY(s.yMin) + OH + 0.0f, axisLabelFont.getStringWidth(axislabel) + 2 * mLabel, axisLabelFont.getHeight(), Justification::centred, true);
            }
        }

        g.setColour (Colours::white.withAlpha(0.05f));
        g.fillRect (static_cast<float>(hzToX(s.fMin)), dirToY(0),
                    static_cast<float>(hzToX(s.fMax) - hzToX(s.fMin)),
                    dirToY(-0.5) - dirToY(0));

        // set path colours and stroke
        g.setColour (mainLaF.mainTextInactiveColor);
        g.strokePath (dirGridPath, PathStrokeType (0.25f));
        
        g.setColour(mainLaF.mainTextInactiveColor);
        g.strokePath (smallDirGridPath, PathStrokeType (0.25f));

        g.setColour(mainLaF.mainTextInactiveColor);
        g.strokePath (hzGridPathBold, PathStrokeType (0.25f));

        g.setColour(mainLaF.mainTextInactiveColor);
        g.strokePath (hzGridPath, PathStrokeType (0.25f));
        
        for (PathComponent& p : bandLimitPaths)
        {
            p.getPath().clear();
        }
        for (BandLimitDividerHolder& p : bandLimitDividerHolders)
        {
            p.getPath().clear();
        }        
        for (Path& p : dirPaths)
        {
            p.clear();
        }

        float lastRightBound;
        float lastCircY;
        int bandMargin = 20;
        int interpPointMargin = 15;
        int patternRectHeight = 14;
        int bandLimitDividerWidth = getTopLevelComponent()->getWidth() * 0.0035f;
        int bandLimitDividerHolderWidth = getTopLevelComponent()->getWidth() * 0.012f;
        int bandLimitDividerHolderHeight = dirToY(s.yMax) - 4.f;

        // paint dirPaths and bandLimitPaths
        for (int i = 0; i < nrActiveBands; ++i)
        {
            BandElements& handle(elements.getReference(i));

            float rightBound = (handle.upperFrequencySlider == nullptr || nrActiveBands == i + 1) ?
                                hzToX(s.fMax) : hzToX (processor.hzFromZeroToOne(i, handle.upperFrequencySlider->getValue()));

            float circY = handle.dirSlider == nullptr ? dirToY(0.0f) : dirToY(handle.dirSlider->getValue());

            // paint band limits
            if (i != nrActiveBands - 1)
            {
                Path& blPath = bandLimitPaths[i].getPath();
                bandsWidth[i] = bandLimitPaths[i].getX() - mL + bandLimitDividerWidth/2;
                blPath.addRectangle(rightBound - bandLimitDividerWidth/2, dirToY(s.yMax), bandLimitDividerWidth, dirToY(s.yMin) - mT);

                g.setColour (mainLaF.textButtonFrameColor.withMultipliedAlpha(activeBandLimitPath == i ? 1.0f : 0.8f));

                g.fillPath(blPath);

                bandLimitPaths[i].setBounds();

                Path& blDhPath = bandLimitDividerHolders[i].getPath();

                Point<float> point1(rightBound - bandLimitDividerHolderWidth / 2, 8.f);
                Point<float> point2(rightBound + bandLimitDividerHolderWidth / 2, 8.f);
                Point<float> point3(rightBound + bandLimitDividerHolderWidth / 2, bandLimitDividerHolderHeight * 0.75f);
                Point<float> point4(rightBound, bandLimitDividerHolderHeight);
                Point<float> point5(rightBound - bandLimitDividerHolderWidth / 2, bandLimitDividerHolderHeight * 0.75f);

                blDhPath.startNewSubPath(point1);
                blDhPath.lineTo(point2);
                blDhPath.lineTo(point3);
                blDhPath.lineTo(point4);
                blDhPath.lineTo(point5);
                blDhPath.closeSubPath();

                g.setColour(mainLaF.textButtonFrameColor.withMultipliedAlpha(activeBandLimitPath == i ? 1.0f : 0.8f));
                g.fillPath(blDhPath);
                bandLimitDividerHolders[i].setBounds();
            }

            // dirPath
            if (nrActiveBands == 1)
            {
                // set surrounding rectangle to trigger mouse drag and for aax automation shortcut
                dirPathRects[i].setBounds(hzToX(s.fMin), circY - patternRectHeight/2, hzToX(s.fMax) - hzToX(s.fMin), patternRectHeight);
                
                dirPaths[i].startNewSubPath (hzToX(s.fMin), circY);
                dirPaths[i].lineTo (hzToX(s.fMax), circY);
                g.setColour(dirPathRects[i].isEnabled() ? handle.colour : handle.colour.withBrightness(0.3f));
                g.strokePath (dirPaths[i], PathStrokeType (2.0f));
                break;
            }

            if (i == 0)
            {
                // set surrounding rectangle to trigger mouse drag and for aax automation shortcut
                dirPathRects[i].setBounds(hzToX(s.fMin), circY - patternRectHeight/2, rightBound - bandMargin - hzToX(s.fMin), patternRectHeight);
                
                dirPaths[i].startNewSubPath (hzToX(s.fMin), circY);
                dirPaths[i].lineTo (rightBound - bandMargin, circY);
            }
            else if (i == nrActiveBands - 1)
            {
                // set surrounding rectangle to trigger mouse drag and for aax automation shortcut
                dirPathRects[i].setBounds(lastRightBound + bandMargin, circY - patternRectHeight/2, hzToX(s.fMax) - lastRightBound - bandMargin, patternRectHeight);

                dirPaths[i-1].startNewSubPath (lastRightBound - bandMargin, lastCircY);
                dirPaths[i-1].quadraticTo (lastRightBound - bandMargin + interpPointMargin, lastCircY, lastRightBound, (lastCircY + circY) / 2);
                g.setColour(dirPathRects[i-1].isEnabled() ? elements.getReference(i-1).colour : elements.getReference(i - 1).colour.withBrightness(0.3f));
                g.strokePath (dirPaths[i-1], PathStrokeType (2.0f));

                dirPaths[i].startNewSubPath (lastRightBound, (lastCircY + circY) / 2);
                dirPaths[i].quadraticTo (lastRightBound + bandMargin - interpPointMargin, circY, lastRightBound + bandMargin, circY);
                dirPaths[i].lineTo (hzToX(s.fMax), circY);
                g.setColour(dirPathRects[i].isEnabled() ? handle.colour : handle.colour.withBrightness(0.3f));
                g.strokePath (dirPaths[i], PathStrokeType (2.0f));
            }
            else
            {
                // set surrounding rectangle to trigger mouse drag and for aax automation shortcut
                dirPathRects[i].setBounds(lastRightBound + bandMargin, circY - patternRectHeight/2, rightBound - bandMargin - lastRightBound - bandMargin, patternRectHeight);

                dirPaths[i-1].startNewSubPath (lastRightBound - bandMargin, lastCircY);
                dirPaths[i-1].quadraticTo (lastRightBound - bandMargin + interpPointMargin, lastCircY, lastRightBound, (lastCircY + circY) / 2);
                g.setColour(dirPathRects[i-1].isEnabled() ? elements.getReference(i - 1).colour : elements.getReference(i - 1).colour.withBrightness(0.3f));
                g.strokePath (dirPaths[i-1], PathStrokeType (2.0f));

                dirPaths[i].startNewSubPath (lastRightBound, (lastCircY + circY) / 2);
                dirPaths[i].quadraticTo (lastRightBound + bandMargin - interpPointMargin, circY, lastRightBound + bandMargin, circY);
                dirPaths[i].lineTo (rightBound - bandMargin, circY);
            }
            lastRightBound = rightBound;
            lastCircY = circY;
        }

        // band handle knobs
        auto knobSize = getTopLevelComponent()->getHeight() * 0.03f;
        for (int i = 0; i < nrActiveBands; ++i)
        {
            BandElements& handle (elements.getReference(i));
            float leftBound = handle.lowerFrequencySlider == nullptr ? hzToX (s.fMin) : hzToX (processor.hzFromZeroToOne(i-1, handle.lowerFrequencySlider->getValue()));
            float rightBound = (handle.upperFrequencySlider == nullptr || nrActiveBands == i + 1) ? hzToX (s.fMax) : hzToX (processor.hzFromZeroToOne(i, handle.upperFrequencySlider->getValue()));
            float circX = (rightBound + leftBound) / 2;
            float circY = handle.dirSlider == nullptr ? dirToY (0.0f) : dirToY (handle.dirSlider->getValue());
            handle.handlePos.setXY(circX,circY);

            bandKnobs[i].setBounds(circX - knobSize / 2, circY - knobSize / 2, knobSize, knobSize);

            if (!handle.dirSlider->isEnabled())
            {
                bandKnobs[i].setEnabled(false);
            }
            else
            {
                bandKnobs[i].setEnabled(true);
            }

            if (bandKnobs[i].isEnabled() && (bandKnobs[i].isMouseOver() || tooltipValueKnobBox[i]->isMouseOver()))
            {
                int y = bandKnobs[i].getY() > dirToY(s.yMin - s.yMax) / 2 ? bandKnobs[i].getY() - bandKnobs[i].getHeight() - 5 : bandKnobs[i].getBottom() + 5;
                drawTooltip(i, bandKnobs[i].getX() - 5, y, true);
            }
            else
                tooltipValueKnobBox[i]->setVisible(false);

            // align elements
            handle.polarPatternVisualizer->getParentComponent()->resized();

            if (i < nrActiveBands - 1)
            {
                // draw tooltip showing frequency
                if (activeBandLimitPath == i || tooltipValueBox[i]->isMouseOver() || tooltipValueBox[i]->isBeingEdited())
                    drawTooltip(i, rightBound-70, dirToY(s.yMax)-OH, false);
                else
                    tooltipValueBox[i]->setVisible(false);
            }
        }
        oldNrActiveBands = nrActiveBands;
    }

    float dirToY(const float dir)
    {
        float height = (float) getHeight() - mB - mT;
        if (height <= 0.0f) return 0.0f;
        float temp = zero - dir / dyn;

        return mT + height * temp;
    }

    float yToDir (const float y)
    {
        float height = (float) getHeight() - mB - mT;

        float temp = -1.0f * ((y - mT) / height - zero) * dyn;
        return temp;
    }

    int hzToX(float hz)
    {
        float width = (float) getWidth() - mL - mR;
        int xpos = mL + width * (log(hz/s.fMin) / log(s.fMax/s.fMin));
        return xpos;
    }

    float xToHz(int x)
    {
        float width = (float) getWidth() - mL - mR;
        return s.fMin * powf ((s.fMax / s.fMin), ((x - mL) / width));
    }

    void mouseDrag(const MouseEvent &event) override
    {
        if (!active || !event.eventComponent->isEnabled())
            return;

        if (event.eventComponent->getName() == "RectangleComponent" || event.eventComponent->getName() == "BandKnobComponent" || event.eventComponent == this)
        {
            if (activeElem != -1)
            {
                // this improves rendering performance!
                for (auto& rec : dirPathRects)
                    rec.setVisible(false);

                MouseEvent ev = event.getEventRelativeTo(this);
                Point<int> pos = ev.getPosition();
                float dirFactor = yToDir(pos.y);

                BandElements& handle(elements.getReference(activeElem));
                if (handle.dirSlider != nullptr)
                    setValueAndSnapToGrid(handle, dirFactor);

                // change all bands by holding alt key
                if (event.mods.isAltDown())
                {
                    for (BandElements& elem : elements)
                    {
                        if (elem.dirSlider != nullptr) setValueAndSnapToGrid(elem, dirFactor);
                    }
                }
                isDraggingDirPath = true;
            }
        }
        else if (event.eventComponent->getName() == "PathComponent" || event.eventComponent->getName() == "BandLimitDividerHolder")
        {
            if (activeBandLimitPath != -1)
            {
                MouseEvent ev = event.getEventRelativeTo(this);
                Point<int> pos = ev.getPosition();
                float frequency = xToHz(pos.x);

                BandElements& handle(elements.getReference(activeBandLimitPath));
                Slider* slider = handle.upperFrequencySlider;
                if (slider != nullptr)
                {
                    slider->setValue(processor.hzToZeroToOne(activeBandLimitPath,
                                                             getXoverValueInRange(activeBandLimitPath, frequency)));
                }
            }
        }
    }

    void mouseMove(const MouseEvent &event) override
    {
        if (!active)
            return;

        int oldActiveElem = activeElem;
        int oldActiveBandLimitPath = activeBandLimitPath;
        activeElem = -1;
        activeBandLimitPath = -1;
        Point<int> pos = event.getPosition();

        if (event.eventComponent->getName() == "RectangleComponent" || event.eventComponent->getName() == "BandKnobComponent")
        {
            // highlight active pattern path if mouse is on rectangle
            for (int i = 0; i < nrActiveBands; i++)
            {
                if (event.eventComponent == &dirPathRects[i] || event.eventComponent == &bandKnobs[i])
                {
                    activeElem = i;
                    break;
                }
            }
        }
        else if (event.eventComponent->getName() == "PathComponent" || event.eventComponent->getName() == "BandLimitDividerHolder")
        {
            // highlight active band limit path
            for (int i = 0; i < nrActiveBands - 1; i++)
            {
                if (event.eventComponent == &bandLimitPaths[i] || event.eventComponent == &bandLimitDividerHolders[i])
                {
                    activeBandLimitPath = i;
                    break;
                }
            }
        }

        if (oldActiveElem != activeElem
            || oldActiveBandLimitPath != activeBandLimitPath)
            repaint();
    }

    void mouseUp (const MouseEvent &event) override
    {
        // this improves rendering performance!
        if (isDraggingDirPath)
        {
            for (int i = 0; i < nrActiveBands; ++i)
            {
                dirPathRects[i].setVisible(true);
                bandKnobs[i].setVisible(true);
            }
            isDraggingDirPath = false;
        }

        if (!active || event.eventComponent != this)
            return;
    }

    void mouseDoubleClick (const MouseEvent &event) override
    {
        if (!active || (event.eventComponent->getName() != "RectangleComponent" && event.eventComponent != this))
            return;

        // set one band to omni
        for (int i = 0; i < nrActiveBands; ++i)
        {
            if (activeElem == i)
            {
                BandElements& elem = elements.getReference(i);
                if (elem.dirSlider != nullptr)
                    elem.dirSlider->setValue(0);
            }
        }
    }

    void mouseExit (const MouseEvent &event) override
    {
        if (event.eventComponent == this)
        {
            activeElem = -1;
            repaint();
        }
        else if (event.eventComponent->getName() == "RectangleComponent" || event.eventComponent->getName() == "BandKnobComponent")
        {
            activeElem = -1;
            repaint();
        }
        else if (event.eventComponent->getName() == "PathComponent" || event.eventComponent->getName() == "BandLimitDividerHolder")
        {
            activeBandLimitPath = -1;
            repaint();
        }
        else
        {
            for (auto& tooltip : tooltipValueBox)
            {
                if (!tooltip->isBeingEdited())
                    tooltip->setVisible(false);
            }
            for (auto& tooltip : tooltipValueKnobBox)
            {
                if (!tooltip->isBeingEdited())
                    tooltip->setVisible(false);
            }
            repaint();
        }
    }

    void resized() override
    {
        Rectangle<int> area(getLocalBounds());

        int xMin = hzToX(s.fMin);
        int xMax = hzToX(s.fMax);
        numPixels = xMax - xMin + 1;

        mB = area.proportionOfHeight(0.06f);
        mL = area.proportionOfHeight(0.13f);
        dirPatternButtonWidth = mL * 0.6f;
        dirPatternButtonHeight = mL * 0.5f;

        frequencies.resize(numPixels);
        for (int i = 0; i < numPixels; ++i)
            frequencies.set(i, xToHz(xMin + i));

        // directivity grid lines
        const float width = getWidth() - mL - mR;
        dirGridPath.clear();
        dyn = s.yMax - s.yMin;
        int numgridlines = dyn/s.gridDiv+1;

        for (int i=0; i < numgridlines; i++)
        {
            float db_val = s.yMax - i * s.gridDiv;
            int ypos = dirToY(db_val);
            dirGridPath.startNewSubPath(mL, ypos);
            dirGridPath.lineTo(mL + width, ypos);
            //Directivity primary buttons
            tbPrimDirButtons[i].setBounds(5.f, ypos, dirPatternButtonWidth, dirPatternButtonHeight);
            tbPrimDirButtons[i].setCentrePosition(mL/2, ypos);
        }

        // add grid for super card, hyper card and broad card
        // and directivity secondary buttons
        smallDirGridPath.clear();
        int ypos = dirToY(hCardFact);
        smallDirGridPath.startNewSubPath(mL, ypos);
        smallDirGridPath.lineTo(mL + width, ypos);
        tbSecDirButtons[0].setBounds(5.f, ypos, dirPatternButtonWidth, dirPatternButtonHeight);
        tbSecDirButtons[0].setCentrePosition(mL / 2, ypos);

        ypos = dirToY(sCardFact);
        smallDirGridPath.startNewSubPath(mL, ypos);
        smallDirGridPath.lineTo(mL + width, ypos);
        tbSecDirButtons[1].setBounds(5.f, ypos, dirPatternButtonWidth, dirPatternButtonHeight);
        tbSecDirButtons[1].setCentrePosition(mL / 2, ypos);

        ypos = dirToY(bCardFact);
        smallDirGridPath.startNewSubPath(mL, ypos);
        smallDirGridPath.lineTo(mL + width, ypos);
        tbSecDirButtons[2].setBounds(5.f, ypos, dirPatternButtonWidth, dirPatternButtonHeight);
        tbSecDirButtons[2].setCentrePosition(mL / 2, ypos);

        ypos = dirToY(rbCardFact);
        smallDirGridPath.startNewSubPath(mL, ypos);
        smallDirGridPath.lineTo(mL + width, ypos);
        tbSecDirButtons[3].setBounds(5.f, ypos, dirPatternButtonWidth, dirPatternButtonHeight);
        tbSecDirButtons[3].setCentrePosition(mL / 2, ypos);

        // frequency grid
        hzGridPath.clear();
        hzGridPathBold.clear();
        for (float f=s.fMin; f <= s.fMax; f += powf(10, floorf(log10(f)))) {
            int xpos = hzToX(f);

            if ((f == 20) || (f == 50) || (f == 100) || (f == 200) || (f == 500) || (f == 1000) || (f == 2000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                hzGridPathBold.startNewSubPath(xpos, dirToY(s.yMax));
                hzGridPathBold.lineTo(xpos, dirToY(s.yMin));
            }
            else
            {
                hzGridPath.startNewSubPath(xpos, dirToY(s.yMax));
                hzGridPath.lineTo(xpos, dirToY(s.yMin));
            }
        }
        initValueBox();
    }

    void addSliders(Colour newColour, Slider* dirSlider = nullptr, Slider* lowerFrequencySlider = nullptr, Slider* upperFrequencySlider = nullptr, ToggleButton* soloButton = nullptr, ToggleButton* muteButton = nullptr, Slider* gainSlider = nullptr, PolarPatternVisualizer* directivityVis = nullptr)
    {
        elements.add({dirSlider, lowerFrequencySlider, upperFrequencySlider, soloButton, muteButton, newColour, gainSlider, directivityVis});
    }

    void setValueAndSnapToGrid(BandElements& elem, float dirFact)
    {
        float snapMargin = 0.035f;
        for (float gridFact : dirFactArray)
        {
            if (std::abs(dirFact - gridFact) < snapMargin)
            {
                elem.dirSlider->setValue(gridFact);
                return;
            }
        }
        elem.dirSlider->setValue(dirFact);
    }

    void setSoloActive(bool set)
    {
        soloActive = set;
    }

    float calcAlphaOfDirPath(BandElements& elem)
    {
        float maxGain = std::max(elem.gainSlider->getMaximum(), std::abs(elem.gainSlider->getMinimum()));
        float absRange = elem.gainSlider->getMaximum() + std::abs(elem.gainSlider->getMinimum());
        float gain = ((float) elem.gainSlider->getValue() + maxGain) / (7.0f/5.0f * absRange) + 2.0f/7.0f;
        
        if (!active)
        {
            return 2.0f/7.0f;
        }
        else if ((elem.soloButton == nullptr || !soloActive) && (elem.muteButton == nullptr || !elem.muteButton->getToggleState()))
        {
            return gain;
        }
        else if ((soloActive && elem.soloButton->getToggleState()) || (!soloActive && !elem.muteButton->getToggleState()))
        {
            return gain;
        }
        else
        {
            return 2.0f/7.0f;
        }
    }

    void setActive(bool set)
    {
        active = set;
        repaint();
    }

    void initValueBox()
    {
        auto& lf = getLookAndFeel();

        for (int i = 0; i < 4; ++i)
        {
            Slider* slider = elements[i].upperFrequencySlider;
            if (slider == nullptr)
                continue;

            slider->addListener(this);

            tooltipValueBox[i].reset (lf.createSliderTextBox (*slider));
            tooltipValueBox[i]->addMouseListener(this, false);
            tooltipValueBox[i]->addListener(this);
            addChildComponent (tooltipValueBox[i].get());
            tooltipValueBox[i]->setText (slider->getTextFromValue(slider->getValue()), NotificationType::dontSendNotification);
            tooltipValueBox[i]->setAlwaysOnTop (true);
            tooltipValueBox[i]->setEditable(true);
        }

        for (int i = 0; i < 5; ++i)
        {
            Slider* slider = elements[i].dirSlider;
            if (slider == nullptr)
                continue;

            slider->addListener(this);

            tooltipValueKnobBox[i].reset(lf.createSliderTextBox(*slider));
            tooltipValueKnobBox[i]->addMouseListener(this, false);
            tooltipValueKnobBox[i]->addListener(this);
            addChildComponent(tooltipValueKnobBox[i].get());
            tooltipValueKnobBox[i]->setText(slider->getTextFromValue(slider->getValue()), NotificationType::dontSendNotification);
            tooltipValueKnobBox[i]->setAlwaysOnTop(true);
            tooltipValueKnobBox[i]->setEditable(false);
        }
    }

    void drawTooltip(int tooltipIndex, int xCoord, int yCoord, bool isKnobTooltip)
    {
         if (tooltipValueBox[tooltipIndex] == nullptr || tooltipValueKnobBox[tooltipIndex] == nullptr)
             return;

        if (!isKnobTooltip)
        {
            tooltipValueBox[tooltipIndex]->setBounds(xCoord, yCoord, getTopLevelComponent()->getWidth() * 0.06f, getTopLevelComponent()->getHeight() * 0.03f);
            tooltipValueBox[tooltipIndex]->setVisible(true);
        }
        else
        {
            tooltipValueKnobBox[tooltipIndex]->setBounds(xCoord, yCoord, getTopLevelComponent()->getWidth() * 0.04f, getTopLevelComponent()->getHeight() * 0.03f);
            tooltipValueKnobBox[tooltipIndex]->setVisible(true);
        }
    }

    void sliderValueChanged(Slider* slider) override
    {
        for (int i = 0; i < 4; ++i)
        {
            Slider* freqSlider = elements[i].upperFrequencySlider;
            if (slider == freqSlider && tooltipValueBox[i]->isVisible())
                tooltipValueBox[i]->setText (slider->getTextFromValue(slider->getValue()), NotificationType::dontSendNotification);
        }
        for (int i = 0; i < 5; ++i)
        {
            Slider* dSlider = elements[i].dirSlider;
            if (slider == dSlider && tooltipValueKnobBox[i]->isVisible())
                tooltipValueKnobBox[i]->setText(slider->getTextFromValue(slider->getValue()), NotificationType::dontSendNotification);
        }
    }

    void labelTextChanged (Label* label) override
    {
        for (int i = 0; i < 4; ++i)
        {
            if (label == tooltipValueBox[i].get())
            {
                Slider* slider = elements[i].upperFrequencySlider;
                if (slider == nullptr)
                    return;

                float attemptedVal = tooltipValueBox[i]->getText().getFloatValue();
                if (attemptedVal == 0)
                    attemptedVal = processor.hzFromZeroToOne(i, slider->getValue());

                float newValue = getXoverValueInRange (i, attemptedVal);

                slider->setValue (processor.hzToZeroToOne(i, newValue), NotificationType::sendNotification);
                tooltipValueBox[i]->setText (slider->getTextFromValue(slider->getValue()) , NotificationType::dontSendNotification);
            }

        }
    }

    float getXoverValueInRange(int sliderNum, float attemptedValue)
    {
        float min = processor.getXoverSliderRangeStart(sliderNum);
        float max = processor.getXoverSliderRangeEnd(sliderNum);
        return attemptedValue < min ? min : (attemptedValue > max ? max : attemptedValue);
    }

    void resetTooltipTexts()
    {
        for (int i = 0; i < 4; ++i)
        {
            Slider* slider = elements[i].upperFrequencySlider;
            if (slider != nullptr)
                tooltipValueBox[i]->setText (slider->getTextFromValue (slider->getValue()), NotificationType::dontSendNotification);
        }
        for (int i = 0; i < 5; ++i)
        {
            Slider* slider = elements[i].dirSlider;
            if (slider != nullptr)
                tooltipValueKnobBox[i]->setText(slider->getTextFromValue(slider->getValue()), NotificationType::dontSendNotification);
        }
    }

    PathComponent& getBandlimitPathComponent (int idx)
    {
        return bandLimitPaths[idx];
    }

    RectangleComponent& getDirPathComponent (int idx)
    {
        return dirPathRects[idx];
    }

    void buttonClicked(Button* button) override
    {
        if (button == &tbPrimDirButtons[0])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(eightFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbPrimDirButtons[1])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(cardFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbPrimDirButtons[2])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(omniFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbPrimDirButtons[3])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(rCardFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbSecDirButtons[0])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(hCardFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbSecDirButtons[1])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(sCardFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbSecDirButtons[2])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(bCardFact);
            }
            resetTooltipTexts();
        }
        else if (button == &tbSecDirButtons[3])
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr && elem.dirSlider->isEnabled())
                    elem.dirSlider->setValue(rbCardFact);
            }
            resetTooltipTexts();
        }
    }

    int getEqWidth()
    {
        return (hzToX(20000) - hzToX(20));
    }

    int getBandWidth(int band)
    {
        return bandsWidth[band];
    }

private:
    PolarDesignerAudioProcessor& processor;

    bool active = true;
    int activeElem = -1;
    int activeBandLimitPath = -1;
    int nrActiveBands;
    int oldNrActiveBands;
    float dyn, zero, scale;
    bool soloActive;
    bool isDraggingDirPath = false;

    Settings s;
    Path dirGridPath;
    Path hzGridPath;
    Path hzGridPathBold;
    Path dirPaths[5];
    Path smallDirGridPath;

    int dirPatternButtonWidth;
    int dirPatternButtonHeight;

    std::unique_ptr<Label> tooltipValueBox[4];
    std::unique_ptr<Label> tooltipValueKnobBox[5];

    Array<double> frequencies;
    int numPixels;
    Array<BandElements> elements;

    static constexpr float omniFact = 0.0f;
    static constexpr float cardFact = 0.5f;
    static constexpr float rCardFact = -0.5f;
    static constexpr float eightFact = 1.0f;
    static constexpr float bCardFact = 0.37f;
    static constexpr float rbCardFact = -0.37f;
    static constexpr float sCardFact = 0.634f;
    static constexpr float hCardFact = 0.75f;
    const float dirFactArray[8];

    // components for aax getControlParameterIndex()
    PathComponent bandLimitPaths[4];
    RectangleComponent dirPathRects[5];
    BandLimitDividerHolder bandLimitDividerHolders[4];
    BandKnobComponent bandKnobs[5];
    int bandsWidth[4];

    // ImageButtons
    TextButton tbPrimDirButtons[4];
    TextButton tbSecDirButtons[4];

    MainLookAndFeel mainLaF;
};
