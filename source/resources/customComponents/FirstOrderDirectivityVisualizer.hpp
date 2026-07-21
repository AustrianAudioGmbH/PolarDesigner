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

//==============================================================================
/*
*/
using namespace dsp;
class FirstOrderDirectivityVisualizer : public Component
{
    const float deg2rad = std::numbers::pi / 180.0f;
    const int degStep = 4;
    const int nLookUpSamples = 360;

public:
    FirstOrderDirectivityVisualizer()
    {
        isActive = true;
        soloButton = nullptr;
        muteButton = nullptr;
        soloActive = false;

        colour = Colour (0xFFD0011B);

        for (int phi = -180; phi <= 180; phi += degStep)
        {
            pointsOnCircle.add (Point<float> (cos (deg2rad * phi), sin (deg2rad * phi)));
        }

        Path circle;
        circle.addEllipse (-1.0f, -1.0f, 2.0f, 2.0f);
        Path line;
        line.startNewSubPath (0.0f, -1.0f);
        line.lineTo (0.0f, 1.0f);

        grid.clear();
        grid.addPath (circle);

        subGrid.clear();
        for (int i = 1; i < 5; i++)
            subGrid.addPath (circle, AffineTransform().scaled (i / 4.0f));

        subGrid.addPath (line);
        subGrid.addPath (line, AffineTransform().rotation (0.25f * std::numbers::pi));
        subGrid.addPath (line, AffineTransform().rotation (0.5f * std::numbers::pi));
        subGrid.addPath (line, AffineTransform().rotation (0.75f * std::numbers::pi));
    }

    ~FirstOrderDirectivityVisualizer() {}

    void paint (Graphics& g) override
    {
        Path path;
        path = grid;
        path.applyTransform (transform);
        g.setColour (Colours::skyblue.withMultipliedAlpha (0.1f));
        g.fillPath (path);
        g.setColour (Colours::white.withMultipliedAlpha (! isActive ? 0.5f : calcAlpha()));
        g.strokePath (path, PathStrokeType (1.0f));

        path = subGrid;
        path.applyTransform (transform);
        g.setColour (Colours::skyblue.withMultipliedAlpha (0.3f));
        g.strokePath (path, PathStrokeType (0.5f));

        // draw directivity
        g.setColour (colour.withMultipliedAlpha (! isActive ? 0.0f : calcAlpha()));
        path.clear();

        int idx = 0;
        for (int phi = -180; phi <= 180; phi += degStep)
        {
            float phiInRad = (float) phi * deg2rad;
            float gainLin = std::abs ((1 - std::abs (dirWeight)) + dirWeight * std::cos (phiInRad));
            int dbMin = 25;
            float gainDb =
                20
                * std::log10 (
                    std::max (gainLin, static_cast<float> (std::pow (10, -dbMin / 20.0f))));
            float effGain = std::max (std::abs ((gainDb + dbMin) / dbMin), 0.01f);
            Point<float> point = effGain * pointsOnCircle[idx];

            if (phi == -180)
                path.startNewSubPath (point);
            else
                path.lineTo (point);
            ++idx;
        }

        path.closeSubPath();
        path.applyTransform (transform);
        g.strokePath (path, PathStrokeType (2.0f));
    }

    void resized() override
    {
        Rectangle<int> bounds = getLocalBounds();
        Point<int> centre = bounds.getCentre();

        bounds.reduce (10, 10);

        if (bounds.getWidth() > bounds.getHeight())
            bounds.setWidth (bounds.getHeight());
        else
            bounds.setHeight (bounds.getWidth());
        bounds.setCentre (centre);

        transform = AffineTransform::fromTargetPoints ((float) centre.x,
                                                       (float) centre.y,
                                                       (float) centre.x,
                                                       bounds.getY(),
                                                       bounds.getX(),
                                                       centre.y);

        plotArea = bounds;
    }

    void setDirWeight (float weight)
    {
        dirWeight = weight;
        repaint();
    }

    void setActive (bool active)
    {
        if (isActive != active)
        {
            isActive = active;
            repaint();
        }
    }

    void setMuteSoloButtons (MuteSoloButton* solo, MuteSoloButton* mute)
    {
        soloButton = solo;
        muteButton = mute;
    }

    float calcAlpha()
    {
        if ((soloButton == nullptr || ! soloActive)
            && (muteButton == nullptr || ! muteButton->getToggleState()))
        {
            return 1.0f;
        }
        else if ((soloActive && soloButton->getToggleState())
                 || (! soloActive && ! muteButton->getToggleState()))
        {
            return 1.0f;
        }
        else
        {
            return 0.4f;
        }
    }

    void setSoloActive (bool set) { soloActive = set; }

    void setColour (Colour newColour) { colour = newColour; }

private:
    Path grid;
    Path subGrid;
    AffineTransform transform;
    Rectangle<int> plotArea;
    float dirWeight;
    bool isActive;
    MuteSoloButton* soloButton;
    MuteSoloButton* muteButton;
    bool soloActive;
    Colour colour;

    Array<Point<float>> pointsOnCircle;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderDirectivityVisualizer)
};
