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

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include "BinaryData.h"

#include "../lookAndFeel/MainLookAndFeel.h"

#ifdef AA_INCLUDE_MELATONIN
#include "melatonin_inspector/melatonin/helpers/timing.h"
#endif


//==============================================================================
/*
*/
using namespace dsp;
class PolarPatternVisualizer : public TextButton
{
    const float deg2rad = static_cast<float>(M_PI / 180.0f);
    const int degStep = 4;
//    const int nLookUpSamples = 360;

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
            subGrid.addPath(circle, AffineTransform().scaled(i / 4.0f));

        hitArea.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
    }

    ~PolarPatternVisualizer() override
    {
        DBG("PolarPatternVisualizer destructor called");

        isActive = false;

        soloButton = nullptr;
        muteButton = nullptr;
    }

    void paint(Graphics& g) override
    {
#ifdef AA_INCLUDE_MELATONIN
        melatonin::ComponentTimer timer { this };
#endif

        innerCircle.clear();
        outerCircle.clear();

        if (mouseOver && isEnabled())
        {
            innerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            innerCircle.applyTransform(transform);

            outerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            outerCircle.applyTransform(strokeTransform);
            outerCircle.setUsingNonZeroWinding(false);
            outerCircle.addPath(innerCircle);

            g.setColour(colour.withAlpha(0.1f));
            g.fillPath(outerCircle);
        }

        if (getToggleState())
        {
            innerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            innerCircle.applyTransform(transform);

            outerCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
            outerCircle.applyTransform(strokeTransform);

            g.setColour(colour.withAlpha(0.1f));
            g.fillPath(outerCircle);
        }

        Path gridPath;
        gridPath = subGrid;
        gridPath.applyTransform(transform);
        g.setColour(mainLaF.polarVisualizerGrid);
        g.strokePath(gridPath, PathStrokeType(1.f));

        // draw directivity
        g.setColour(isEnabled() ? colour : colour.withBrightness(0.5f));

        Path hitCircle;
        hitCircle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);

        if (SystemStats::getOperatingSystemName() == "iOS")
        {
            hitCircle.applyTransform(strokeTransform);
            hitArea.addRectangle(hitCircle.getBounds().getX(), hitCircle.getBounds().getY(), hitCircle.getBounds().getWidth(), hitCircle.getBounds().getHeight());
        }
        else
        {
            hitCircle.applyTransform(transform);
            hitArea = hitCircle;
        }
        g.strokePath(dirPath, PathStrokeType(2.0f));
    }

    void resized() override
    {
        Rectangle<int> bounds = getLocalBounds();
        Rectangle<int> strokeBounds = getLocalBounds();
        Point<int> centre = bounds.getCentre();

        bounds.reduce(5,5);

        if (bounds.getWidth() > bounds.getHeight())
            bounds.setWidth(bounds.getHeight());
        else
            bounds.setHeight(bounds.getWidth());
        bounds.setCentre(centre);

        transform = AffineTransform::fromTargetPoints(centre.x * 1.0f, centre.y * 1.0f, centre.x * 1.0f, bounds.getY() * 1.0f, bounds.getX() * 1.0f, centre.y * 1.0f).translated(((getWidth() - getHeight()) / 2.0f), 0.0f);

        if (strokeBounds.getWidth() > strokeBounds.getHeight())
            strokeBounds.setWidth(strokeBounds.getHeight());
        else
            strokeBounds.setHeight(strokeBounds.getWidth());
        strokeBounds.setCentre(centre);

        strokeTransform = AffineTransform::fromTargetPoints(centre.x * 1.0f, centre.y * 1.0f, centre.x * 1.0f, strokeBounds.getY() * 1.0f, strokeBounds.getX() * 1.0f, centre.y * 1.0f).translated(((getWidth() - getHeight()) / 2) * 1.0f, 0.0f);

        plotArea = bounds;
        recalculateDirectivity();
    }

    void recalculateDirectivity()
    {
        dirPath.clear();
        int idx = 0;
        for (int phi = -180; phi <= 180; phi += degStep)
        {
            float phiInRad = (float)phi * deg2rad;
            float gainLin = std::abs((1 - std::abs(dirWeight)) + dirWeight * std::cos(phiInRad));
            int dbMin = 25;
            float gainDb = 20 * std::log10(std::max(gainLin, static_cast<float> (std::pow(10, -dbMin / 20.0f))));
            float effGain = std::max(std::abs((gainDb + dbMin) / dbMin), 0.01f);
            Point<float> point = effGain * pointsOnCircle[idx];

            if (phi == -180)
                dirPath.startNewSubPath(point);
            else
                dirPath.lineTo(point);
            ++idx;
        }

        dirPath.closeSubPath();
        dirPath.applyTransform(transform);
    }

    bool hitTest(int x, int y) override
    {
        return hitArea.contains(x * 1.0f, y * 1.0f);
    }

    void setDirWeight(float weight)
    {
        dirWeight = weight;
        recalculateDirectivity();
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

    void setSoloActive(bool set)
    {
        soloActive = set;
    }

    void setColour(Colour newColour)
    {
        colour = newColour;
        repaint();
    }

    void mouseEnter(const MouseEvent& e) override
    {
        (void)e;
        mouseOver = true;
        repaint();
    }

    void mouseExit(const MouseEvent& e) override
    {
        (void)e;
        mouseOver = false;
        repaint();
    }

private:
    Path grid;
    Path subGrid;
    Path innerCircle;
    Path outerCircle;
    Path dirPath;
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolarPatternVisualizer)
};
