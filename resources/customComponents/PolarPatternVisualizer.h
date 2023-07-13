/*
 This file is based on DirectivityVisualizer.h of the IEM plug-in suite.
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../resources/lookAndFeel/MainLookAndFeel.h"

//==============================================================================
/*
*/
using namespace dsp;
class PolarPatternVisualizer : public TextButton
{
    const float deg2rad = M_PI / 180.0f;
    const int degStep = 4;
    const int nLookUpSamples = 360;

public:
    PolarPatternVisualizer()
    {
        isActive = false;
        soloButton = nullptr;
        muteButton = nullptr;
        soloActive = false;
        mouseOver = false;

        colour = mainLaF.mainBackground;
        
        for (int phi = -180; phi <= 180; phi += degStep)
        {
            pointsOnCircle.add(Point<float>(cos(deg2rad * phi), sin(deg2rad * phi)));
        }

        Path circle;
        circle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);

        subGrid.clear();
        for (int i = 1; i < 5; i++)
            subGrid.addPath(circle, AffineTransform().scaled(i/4.0f));
    }

    ~PolarPatternVisualizer()
    {
    }

    void paint (Graphics& g) override
    {
        if (mouseOver && isEnabled())
        {
            Path innerCircle;
            innerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            innerCircle.applyTransform(transform);

            Path outerCircle;
            outerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            outerCircle.applyTransform(strokeTransform);
            outerCircle.setUsingNonZeroWinding(false);
            outerCircle.addPath(innerCircle);

            g.setColour(colour.withAlpha(0.1f));
            g.fillPath(outerCircle);
        }

        if (getToggleState())
        {
            Path innerCircle;
            innerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            innerCircle.applyTransform(transform);

            Path outerCircle;
            outerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            outerCircle.applyTransform(strokeTransform);

            g.setColour(colour.withAlpha(0.1f));
            g.fillPath(outerCircle);
        }

        Path gridPath;
        gridPath = subGrid;
        gridPath.applyTransform(transform);
        g.setColour (mainLaF.polarVisualizerGrid);
        g.strokePath(gridPath, PathStrokeType(1.f));

        // draw directivity
        Path dirPath;
        g.setColour(isEnabled() ? colour : colour.withBrightness(0.5f));
        
        int idx=0;
        for (int phi = -180; phi <= 180; phi += degStep)
        {
            float phiInRad = (float) phi * deg2rad;
            float gainLin = std::abs((1 - std::abs (dirWeight)) + dirWeight * std::cos(phiInRad));
            int dbMin = 25;
            float gainDb = 20 * std::log10 (std::max (gainLin, static_cast<float> (std::pow (10, -dbMin / 20.0f))));
            float effGain = std::max (std::abs ((gainDb + dbMin) / dbMin), 0.01f);
            Point<float> point = effGain * pointsOnCircle[idx];
            
            if (phi == -180)
                dirPath.startNewSubPath(point);
            else
                dirPath.lineTo(point);
            ++idx;
        }

        dirPath.closeSubPath();
        dirPath.applyTransform(transform);
        g.strokePath(dirPath, PathStrokeType(2.0f));
    }

    void resized() override
    {
        Rectangle<int> bounds = getLocalBounds();
        Rectangle<int> strokeBounds = getLocalBounds();
        Point<int> centre = bounds.getCentre();

        bounds.reduce(10,10);
        strokeBounds.reduce(6, 6);

        if (bounds.getWidth() > bounds.getHeight())
            bounds.setWidth(bounds.getHeight());
        else
            bounds.setHeight(bounds.getWidth());
        bounds.setCentre(centre);

        transform = AffineTransform::fromTargetPoints((float) centre.x, (float) centre.y, (float)  centre.x, bounds.getY(), bounds.getX(), centre.y);

        if (strokeBounds.getWidth() > strokeBounds.getHeight())
            strokeBounds.setWidth(strokeBounds.getHeight());
        else
            strokeBounds.setHeight(strokeBounds.getWidth());
        strokeBounds.setCentre(centre);

        strokeTransform = AffineTransform::fromTargetPoints((float)centre.x, (float)centre.y, (float)centre.x, strokeBounds.getY(), strokeBounds.getX(), centre.y);

        hitArea.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
        hitArea.applyTransform(strokeTransform);

        plotArea = bounds;
    }

    bool hitTest(int x, int y) override
    {
        return hitArea.contains(x, y);
    }

    void setDirWeight(float weight)
    {
        dirWeight = weight;
        repaint();
    }
    
    void setActive(bool active)
    {
        if (isActive != active)
        {
            isActive = active;
            repaint();
        }
    }

    bool isPvisActive()
    {
        return isActive;
    }

    void setMuteSoloButtons(ToggleButton* solo, ToggleButton* mute)
    {
        soloButton = solo;
        muteButton = mute;
    }
    
    float calcAlpha()
    {
        if ((soloButton == nullptr || !soloActive) && (muteButton == nullptr || !muteButton->getToggleState()))
        {
            return 1.0f;
        }
        else if ((soloActive && soloButton->getToggleState()) || (!soloActive && !muteButton->getToggleState()))
        {
            return 1.0f;
        }
        else
        {
            return 0.4f;
        }
    }
    
    void setSoloActive (bool set)
    {
        soloActive = set;
    }
    
    void setColour (Colour newColour)
    {
        colour = newColour;
        repaint();
    }

    void mouseEnter(const MouseEvent& e) override
    {
        mouseOver = true;
        repaint();
    }

    void mouseExit(const MouseEvent& e) override
    {
        mouseOver = false;
        repaint();
    }

private:
    Path grid;
    Path subGrid;
    AffineTransform transform, strokeTransform;
    Rectangle<int> plotArea;
    float dirWeight;
    bool isActive;
    ToggleButton* soloButton;
    ToggleButton* muteButton;
    bool soloActive;
    Colour colour;
    Colour hoverColour;
    bool mouseOver;
    Path hitArea;

    Array<Point<float>> pointsOnCircle;
    MainLookAndFeel mainLaF;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarPatternVisualizer)
};
