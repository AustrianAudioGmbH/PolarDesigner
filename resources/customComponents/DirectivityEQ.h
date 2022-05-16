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
#include "ImgPaths.h"

// !J! On iOS we make the knobs fatter for touchscreen ease-of-use
#ifdef JUCE_IOS
static const float POLAR_DESIGNER_KNOBS_SIZE              = 40.0f;
static const float POLAR_DESIGNER_BANDLIMIT_DIVIDER_SIZE  =  8.0f;
#else
static const float POLAR_DESIGNER_KNOBS_SIZE              = 20.0f;
static const float POLAR_DESIGNER_BANDLIMIT_DIVIDER_SIZE  =  4.0f;
#endif

class  DirectivityEQ : public Component, private Slider::Listener, private Label::Listener
{

//#define AA_DO_DEBUG_PATH
#ifdef AA_DO_DEBUG_PATH
#warning "AUSTRIANAUDIO: DEBUG PATHS ARE TURNED ON!"
    Path debugPath; // !J! used for the purpose of debugging UI elements only
#endif
    
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
        MuteSoloButton* soloButton = nullptr;
        MuteSoloButton* muteButton = nullptr;
        Colour colour;
        Slider* gainSlider = nullptr;
        PolarPatternVisualizer* polarPatternVisualizer = nullptr;

        Point<int> handlePos;
    };

    // margins
    const float mL = 33.0f;
    const float mR = 10.0f;
    const float mT = 15.0f;
    const float mB = 15.0f;
    const float OH = 3.0f;
    
    class PathComponent : public Component
    {
    public:
        PathComponent() : Component() {
            setAlwaysOnTop(true);
            setName("PathComponent");
            path.preallocateSpace(1000);    // !J! Arbitrary Magic Number
        };
        ~PathComponent() {};
        
        void setBounds() { Component::setBounds ( path.getBounds().withWidth(10.0f).translated(-3.0f,0.0f).toNearestInt()); }
        
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

public:
    DirectivityEQ(PolarDesignerAudioProcessor& p) :
                 Component(), processor(p), nrActiveBands(0), soloActive(false),
                 dirFactArray{omniFact, cardFact, rCardFact, eightFact,
                             bCardFact, rbCardFact, sCardFact, hCardFact}
    {
        cardPath.loadPathFromData (cardData, sizeof (cardData));
        eightPath.loadPathFromData (eightData, sizeof (eightData));
        omniPath.loadPathFromData (omniData, sizeof (omniData));
        rCardPath.loadPathFromData (cardData, sizeof (cardData));
        rCardPath.applyTransform (AffineTransform::rotation(M_PI));
        sCardPath.loadPathFromData (sCardData, sizeof (sCardData));
        hCardPath.loadPathFromData (hCardData, sizeof (hCardData));
        bCardPath.loadPathFromData (bCardData, sizeof (bCardData));
        rbCardPath.loadPathFromData (bCardData, sizeof (bCardData));
        rbCardPath.applyTransform (AffineTransform::rotation(M_PI));
        init();
        
        for (int i = 0; i < 4; ++i)
        {
            addAndMakeVisible (&bandLimitPaths[i]);
            bandLimitPaths[i].addMouseListener(this, true);
        }
        
        for (int i = 0; i < 5; ++i)
        {
            addAndMakeVisible (&dirPathRects[i]);
            dirPathRects[i].addMouseListener(this, true);
        }
        
    };

    void init ()
    {
        dyn = s.yMax - s.yMin;
        zero = s.yMax / dyn;
    }
    
    ~DirectivityEQ() {
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
                    dirPathRects[i].setVisible(true);
                else
                    dirPathRects[i].setVisible(false);
            }
            for (int i = 0; i < 4; ++i)
            {
                if (i < nrActiveBands - 1)
                    bandLimitPaths[i].setVisible(true);
                else
                    bandLimitPaths[i].setVisible(false);
            }
        }
        
        // directivity labels
        int height = getHeight();
        int dirImgSize = 20;
        int smallImgSize = 15;
        float strokeSizeThin = 0.5f;
        float strokeSizeThick = 2.0f;
        eightPath.applyTransform (eightPath.getTransformToScaleToFit (5.0f, mT - dirImgSize / 2, dirImgSize, dirImgSize, true, Justification::left));
        g.setColour (Colours::white);
        g.strokePath (eightPath, PathStrokeType (activePatternPath == eightFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (eightPath);
        
        hCardPath.applyTransform (hCardPath.getTransformToScaleToFit (mL / 2 - 15.0f, static_cast<float>(height) / 6 + 3.0f, smallImgSize, smallImgSize, true, Justification::right));
        g.strokePath (hCardPath, PathStrokeType (activePatternPath == hCardFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (hCardPath);
        
        sCardPath.applyTransform (sCardPath.getTransformToScaleToFit (mL / 2 - 14.0f, static_cast<float>(height) / 4, smallImgSize, smallImgSize, true, Justification::right));
        g.strokePath (sCardPath, PathStrokeType (activePatternPath == sCardFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (sCardPath);
        
        cardPath.applyTransform (cardPath.getTransformToScaleToFit (1.0f, static_cast<float>(height) / 3 - dirImgSize / 2 + 4.0f, dirImgSize, dirImgSize, true, Justification::right));
        g.strokePath (cardPath, PathStrokeType (activePatternPath == cardFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (cardPath);
        
        bCardPath.applyTransform (bCardPath.getTransformToScaleToFit (mL / 2 - 13.0f, static_cast<float>(height) / 2 - 27.0f, smallImgSize, smallImgSize, true, Justification::right));
        g.strokePath (bCardPath, PathStrokeType (activePatternPath == bCardFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (bCardPath);
        
        omniPath.applyTransform (omniPath.getTransformToScaleToFit (1.0f, static_cast<float>(height) * 2 / 3 - dirImgSize / 2 - 6.0f, dirImgSize, dirImgSize, true, Justification::right));
        g.strokePath (omniPath, PathStrokeType (activePatternPath == omniFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (omniPath);
        
        rbCardPath.applyTransform (rbCardPath.getTransformToScaleToFit (mL / 2 - 13.0f, static_cast<float>(height) - mB - smallImgSize / 2 - 22.0f, smallImgSize, smallImgSize, true, Justification::right));
        g.strokePath (rbCardPath, PathStrokeType (activePatternPath == rbCardFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (rbCardPath);
        
        rCardPath.applyTransform (rCardPath.getTransformToScaleToFit (1.0f, static_cast<float>(height) - mB - dirImgSize / 2, dirImgSize, dirImgSize, true, Justification::right));
        g.strokePath (rCardPath, PathStrokeType (activePatternPath == rCardFact ? strokeSizeThick : strokeSizeThin));
        g.fillPath (rCardPath);
        
        // frequency labels
        g.setFont (getLookAndFeel().getTypefaceForFont (Font(12.0f, 2)));
        g.setFont (16.0f);
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
                g.drawText (axislabel, xpos - 10, dirToY(s.yMin) + OH + 0.0f, 30, 12, Justification::centred, true);
            }
        }

        g.setColour (Colours::whitesmoke.withMultipliedAlpha(0.1f));
        g.fillRect (static_cast<float>(hzToX(s.fMin)), dirToY(0),
                    static_cast<float>(hzToX(s.fMax) - hzToX(s.fMin)),
                    dirToY(-0.5) - dirToY(0));
        
        // set path colours and stroke
        g.setColour (Colours::white);
        g.strokePath (dirGridPath, PathStrokeType (0.5f));
        
        g.setColour (Colours::white.withMultipliedAlpha(0.5f));
        g.strokePath (smallDirGridPath, PathStrokeType (0.5f));

        g.setColour (Colours::white);
        g.strokePath (hzGridPathBold, PathStrokeType (0.5f));

        g.setColour (Colours::white.withMultipliedAlpha(0.5f));
        g.strokePath (hzGridPath, PathStrokeType (0.5f));
        
        for (PathComponent& p : bandLimitPaths)
        {
            p.getPath().clear();
        }
        for (Path& p : dirPaths)
        {
            p.clear();
        }
       
#ifdef AA_DO_DEBUG_PATH
        debugPath.clear();      // !J! Used only for debugging UI elements
#endif

        
        float lastRightBound;
        float lastCircY;
        int bandMargin = 20;
        int interpPointMargin = 15;
        int patternRectHeight = 14;
        
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
                
                blPath.startNewSubPath (rightBound, dirToY(s.yMax)-OH);
                
                blPath.lineTo (rightBound, dirToY(s.yMin)+OH);
                
//blPath.addRectangle(rightBound - 20.0, dirToY(s.yMin)+OH - 20, 40, 40);
                
                g.setColour (Colours::steelblue.withMultipliedAlpha(activeBandLimitPath == i ? 1.0f : 0.8f));

                g.strokePath (blPath, PathStrokeType (POLAR_DESIGNER_BANDLIMIT_DIVIDER_SIZE));


#ifdef AA_DO_DEBUG_PATH
#if 0
                { // !J! for debug purposes only
                    
                    debugPath.startNewSubPath(bandLimitPaths[i].getBounds().getX(),
                                              bandLimitPaths[i].getBounds().getY());

                    debugPath.lineTo(bandLimitPaths[i].getBounds().getRight(),
                                     bandLimitPaths[i].getBounds().getBottom());

                    debugPath.addRectangle(bandLimitPaths[i].getBounds().getX(), bandLimitPaths[i].getBounds().getY(),
                                           bandLimitPaths[i].getBounds().getWidth(), bandLimitPaths[i].getBounds().getHeight());

//                    debugPath.addStar(bandLimitPaths[i].getScreenPosition().toFloat(), 8, 10, 20);
                    
                }
#endif
#endif

                bandLimitPaths[i].setBounds();

            }
            
            // dirPath
            if (nrActiveBands == 1)
            {
                // set surrounding rectangle to trigger mouse drag and for aax automation shortcut
                dirPathRects[i].setBounds(hzToX(s.fMin), circY - patternRectHeight/2, hzToX(s.fMax) - hzToX(s.fMin), patternRectHeight);
                
                dirPaths[i].startNewSubPath (hzToX(s.fMin), circY);
                dirPaths[i].lineTo (hzToX(s.fMax), circY);
                g.setColour (handle.colour.withSaturation(activeElem == i ? 0.6 : 0.5f).withMultipliedAlpha(calcAlphaOfDirPath(handle)));
                g.strokePath (dirPaths[i], PathStrokeType (3.0f));
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
                g.setColour (elements.getReference(i-1).colour.withSaturation(activeElem == i-1 ? 0.6 : 0.5f).
                             withMultipliedAlpha(calcAlphaOfDirPath(elements.getReference(i-1))));
                g.strokePath (dirPaths[i-1], PathStrokeType (3.0f));
                
                dirPaths[i].startNewSubPath (lastRightBound, (lastCircY + circY) / 2);
                dirPaths[i].quadraticTo (lastRightBound + bandMargin - interpPointMargin, circY, lastRightBound + bandMargin, circY);
                dirPaths[i].lineTo (hzToX(s.fMax), circY);
                g.setColour (handle.colour.withSaturation(activeElem == i ? 0.6 : 0.5f).withMultipliedAlpha(calcAlphaOfDirPath(handle)));
                g.strokePath (dirPaths[i], PathStrokeType (3.0f));
            }
            else
            {
                // set surrounding rectangle to trigger mouse drag and for aax automation shortcut
                dirPathRects[i].setBounds(lastRightBound + bandMargin, circY - patternRectHeight/2, rightBound - bandMargin - lastRightBound - bandMargin, patternRectHeight);
                
                dirPaths[i-1].startNewSubPath (lastRightBound - bandMargin, lastCircY);
                dirPaths[i-1].quadraticTo (lastRightBound - bandMargin + interpPointMargin, lastCircY, lastRightBound, (lastCircY + circY) / 2);
                g.setColour (elements.getReference(i-1).colour.withSaturation(activeElem == i-1 ? 0.6 : 0.5f).
                             withMultipliedAlpha(calcAlphaOfDirPath(elements.getReference(i-1))));
                g.strokePath (dirPaths[i-1], PathStrokeType (3.0f));
                
                dirPaths[i].startNewSubPath (lastRightBound, (lastCircY + circY) / 2);
                dirPaths[i].quadraticTo (lastRightBound + bandMargin - interpPointMargin, circY, lastRightBound + bandMargin, circY);
                dirPaths[i].lineTo (rightBound - bandMargin, circY);
            }
            lastRightBound = rightBound;
            lastCircY = circY;
        }


        // band handle knobs
        for (int i = 0; i < nrActiveBands; ++i)
        {
            BandElements& handle (elements.getReference(i));
            float leftBound = handle.lowerFrequencySlider == nullptr ? hzToX (s.fMin) : hzToX (processor.hzFromZeroToOne(i-1, handle.lowerFrequencySlider->getValue()));
            float rightBound = (handle.upperFrequencySlider == nullptr || nrActiveBands == i + 1) ? hzToX (s.fMax) : hzToX (processor.hzFromZeroToOne(i, handle.upperFrequencySlider->getValue()));
            float circX = (rightBound + leftBound) / 2;
            float circY = handle.dirSlider == nullptr ? dirToY (0.0f) : dirToY (handle.dirSlider->getValue());
            handle.handlePos.setXY(circX,circY);
                        
            // paint band handles
            g.setColour (Colour (0xFF191919));
            
            g.drawEllipse (circX - (POLAR_DESIGNER_KNOBS_SIZE / 2),
                           circY - (POLAR_DESIGNER_KNOBS_SIZE / 2) ,
                           POLAR_DESIGNER_KNOBS_SIZE,
                           POLAR_DESIGNER_KNOBS_SIZE, 3.0f);
            
            g.setColour (handle.colour);
            
            g.drawEllipse (circX - (POLAR_DESIGNER_KNOBS_SIZE / 2),
                           circY - (POLAR_DESIGNER_KNOBS_SIZE / 2),
                           POLAR_DESIGNER_KNOBS_SIZE,
                           POLAR_DESIGNER_KNOBS_SIZE, 1.0f);
            
            g.setColour (activeElem == i ? handle.colour.withSaturation(0.8) : handle.colour.withSaturation (0.2).withMultipliedAlpha (calcAlphaOfDirPath(handle)));
            g.fillEllipse (circX - (POLAR_DESIGNER_KNOBS_SIZE / 2),
                           circY - (POLAR_DESIGNER_KNOBS_SIZE / 2),
                           POLAR_DESIGNER_KNOBS_SIZE,
                           POLAR_DESIGNER_KNOBS_SIZE);


            // align elements
            handle.dirSlider->setBounds(circX + (handle.dirSlider->getWidth() + POLAR_DESIGNER_KNOBS_SIZE),
                                        handle.dirSlider->getY(),
                                        handle.dirSlider->getWidth(), handle.dirSlider->getHeight());
            handle.gainSlider->setBounds(circX + (handle.gainSlider->getWidth() + POLAR_DESIGNER_KNOBS_SIZE),
                                         handle.gainSlider->getY(),
                                         handle.gainSlider->getWidth(), handle.gainSlider->getHeight());
            
            handle.muteButton->setTopLeftPosition(handle.gainSlider->getX(), handle.muteButton->getY());
            handle.soloButton->setTopLeftPosition(handle.gainSlider->getX() +
                                                  handle.gainSlider->getWidth() -
                                                  handle.soloButton->getWidth(),
                                                  handle.soloButton->getY());

            handle.polarPatternVisualizer->setBounds(handle.gainSlider->getX(), handle.polarPatternVisualizer->getY(),
                                                     handle.polarPatternVisualizer->getWidth(),
                                                     handle.polarPatternVisualizer->getHeight());

            
#if 1
        std::cout << "ppVis bounds: " << handle.polarPatternVisualizer->getBounds().getX() << ", " << handle.polarPatternVisualizer->getBounds().getY() << "\n";
#endif

//#ifdef AA_DO_DEBUG_PATH
//            { // !J! for debug purposes only
//
//                debugPath.addStar(handle.polarPatternVisualizer->getScreenPosition().toFloat(), 8, 10, 20);
//                debugPath.startNewSubPath(handle.polarPatternVisualizer->getBounds().getX(),
//                                      handle.polarPatternVisualizer->getBounds().getY());
//
//                debugPath.lineTo(handle.polarPatternVisualizer->getBounds().getRight(),
//                             handle.polarPatternVisualizer->getBounds().getBottom());
//
//                debugPath.addRectangle(handle.polarPatternVisualizer->getBounds().getX(), handle.polarPatternVisualizer->getBounds().getY(),
//                                       handle.polarPatternVisualizer->getBounds().getWidth(), handle.polarPatternVisualizer->getBounds().getHeight());
//            }
//
//            { // !J! for debug purposes only
//
//                debugPath.startNewSubPath(handle.handlePos.getX(),
//                                          handle.handlePos.getY());
//                debugPath.addStar(handle.handlePos.toFloat(), 6, 5, 10);
//            }
//#endif

            if (i < nrActiveBands - 1)
            {
                // draw tooltip showing frequency
                if (activeBandLimitPath == i || tooltipValueBox[i]->isMouseOver() || tooltipValueBox[i]->isBeingEdited())
                    drawTooltip(i, rightBound-60, dirToY(s.yMax)-OH);
                else
                    tooltipValueBox[i]->setVisible(false);
            }
        }
        
        oldNrActiveBands = nrActiveBands;

#ifdef AA_DO_DEBUG_PATH
        g.strokePath (debugPath, PathStrokeType (5.0f)); // !J!
#endif

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
        if (!active)
            return;
        
        if (event.eventComponent->getName() == "RectangleComponent" || event.eventComponent == this)
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
        else if (event.eventComponent->getName() == "PathComponent")
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
        float oldActivePath = activePatternPath;
        int oldActiveBandLimitPath = activeBandLimitPath;
        activeElem = -1;
        activePatternPath = -1;
        activeBandLimitPath = -1;
        Point<int> pos = event.getPosition();

        if (event.eventComponent == this)
        {
            Point<float> posf = Point<float>(pos.getX(),pos.getY());
            
            // highlight active polar pattern path
            if (omniPath.getBounds().contains(posf))
                activePatternPath = omniFact;
            else if (eightPath.getBounds().contains(posf))
                activePatternPath = eightFact;
            else if (cardPath.getBounds().contains(posf))
                activePatternPath = cardFact;
            else if (rCardPath.getBounds().contains(posf))
                activePatternPath = rCardFact;
            else if (sCardPath.getBounds().contains(posf))
                activePatternPath = sCardFact;
            else if (hCardPath.getBounds().contains(posf))
                activePatternPath = hCardFact;
            else if (bCardPath.getBounds().contains(posf))
                activePatternPath = bCardFact;
            else if (rbCardPath.getBounds().contains(posf))
                activePatternPath = rbCardFact;
            
            // highlight active pattern path if mouse is close to handle circle
            for (int i = 0; i < nrActiveBands; i++)
            {
                BandElements& handle(elements.getReference(i));
                
                if (pos.getDistanceSquaredFrom(handle.handlePos) < 140)
                {
                    activeElem = i;
                    break;
                }
            }
        }
        else if (event.eventComponent->getName() == "RectangleComponent")
        {
            // highlight active pattern path if mouse is on rectangle
            for (int i = 0; i < nrActiveBands; i++)
            {
                if (event.eventComponent == &dirPathRects[i])
                {
                    activeElem = i;
                    break;
                }
            }
        }
        else if (event.eventComponent->getName() == "PathComponent")
        {
            // highlight active band limit path
            for (int i = 0; i < nrActiveBands - 1; i++)
            {
                if (event.eventComponent == &bandLimitPaths[i])
                {
                    activeBandLimitPath = i;
                    break;
                }
            }
        }

        if (oldActiveElem != activeElem
            || oldActivePath != activePatternPath
            || oldActiveBandLimitPath != activeBandLimitPath)
            repaint();
    }
    
    void mouseUp (const MouseEvent &event) override
    {
        // this improves rendering performance!
        if (isDraggingDirPath)
        {
            for (int i = 0; i < nrActiveBands; ++i)
                dirPathRects[i].setVisible(true);
            
            isDraggingDirPath = false;
        }
        
        if (!active || event.eventComponent != this)
            return;
        
        // set all patterns according to click on pattern symbol
        if (activePatternPath != -1)
        {
            for (BandElements& elem : elements)
            {
                if (elem.dirSlider != nullptr)
                    elem.dirSlider->setValue(activePatternPath);
            }
        }
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
            activePatternPath = -1;
            activeElem = -1;
            repaint();
        }
        else if (event.eventComponent->getName() == "RectangleComponent")
        {
            activeElem = -1;
            repaint();
        }
        else if (event.eventComponent->getName() == "PathComponent")
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
        }
    }

    void resized() override
    {
        int xMin = hzToX(s.fMin);
        int xMax = hzToX(s.fMax);
        numPixels = xMax - xMin + 1;

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
            dirGridPath.startNewSubPath(mL-OH, ypos);
            dirGridPath.lineTo(mL + width+OH, ypos);
        }
        
        // add grid for super card, hyper card and broad card
        smallDirGridPath.clear();
        int ypos = dirToY(hCardFact);
        smallDirGridPath.startNewSubPath(mL-OH, ypos);
        smallDirGridPath.lineTo(mL + width+OH, ypos);
        
        ypos = dirToY(sCardFact);
        smallDirGridPath.startNewSubPath(mL-OH, ypos);
        smallDirGridPath.lineTo(mL + width+OH, ypos);
        
        ypos = dirToY(bCardFact);
        smallDirGridPath.startNewSubPath(mL-OH, ypos);
        smallDirGridPath.lineTo(mL + width+OH, ypos);
        
        ypos = dirToY(rbCardFact);
        smallDirGridPath.startNewSubPath(mL-OH, ypos);
        smallDirGridPath.lineTo(mL + width+OH, ypos);

        // frequency grid
        hzGridPath.clear();
        hzGridPathBold.clear();
        for (float f=s.fMin; f <= s.fMax; f += powf(10, floorf(log10(f)))) {
            int xpos = hzToX(f);

            if ((f == 20) || (f == 50) || (f == 100) || (f == 200) || (f == 500) || (f == 1000) || (f == 2000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                hzGridPathBold.startNewSubPath(xpos, dirToY(s.yMax)-OH);
                hzGridPathBold.lineTo(xpos, dirToY(s.yMin)+OH);
            }
            else
            {
                hzGridPath.startNewSubPath(xpos, dirToY(s.yMax)-OH);
                hzGridPath.lineTo(xpos, dirToY(s.yMin)+OH);
            }
        }
        
        initValueBox();
    }

    void addSliders(Colour newColour, Slider* dirSlider = nullptr, Slider* lowerFrequencySlider = nullptr, Slider* upperFrequencySlider = nullptr, MuteSoloButton* soloButton = nullptr, MuteSoloButton* muteButton = nullptr, Slider* gainSlider = nullptr, PolarPatternVisualizer* directivityVis = nullptr
                    )
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
    }
    
    void drawTooltip(int tooltipIndex, int xCoord, int yCoord)
    {
         if (tooltipValueBox[tooltipIndex] == nullptr)
             return;
        
        int tooltipWidth = 60;
        int tooltipHeight = 20;
        tooltipValueBox[tooltipIndex]->setBounds (xCoord, yCoord, tooltipWidth, tooltipHeight);
        tooltipValueBox[tooltipIndex]->setVisible (true);
    }
    
    void sliderValueChanged(Slider* slider) override
    {
        for (int i = 0; i < 4; ++i)
        {
            Slider* freqSlider = elements[i].upperFrequencySlider;
            if (slider == freqSlider && tooltipValueBox[i]->isVisible())
                tooltipValueBox[i]->setText (slider->getTextFromValue(slider->getValue()), NotificationType::dontSendNotification);
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
    }
    
    PathComponent& getBandlimitPathComponent (int idx)
    {
        return bandLimitPaths[idx];
    }
                                                 
    RectangleComponent& getDirPathComponent (int idx)
    {
        return dirPathRects[idx];
    }

private:
    PolarDesignerAudioProcessor& processor;
    
    bool active = true;
    int activeElem = -1;
    int activeBandLimitPath = -1;
    float activePatternPath = -1;
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
    
    std::unique_ptr<Label> tooltipValueBox[4];

    Array<double> frequencies;
    int numPixels;
    Array<BandElements> elements;
    
    Path cardPath;
    Path eightPath;
    Path omniPath;
    Path rCardPath;
    Path hCardPath;
    Path bCardPath;
    Path rbCardPath;
    Path sCardPath;
    
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
};
