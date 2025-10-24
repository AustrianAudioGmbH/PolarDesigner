//
//  EndlessSlider.h
//  PolarDesigner - All
//
//  Created by Jay Vaughan on 23.02.22.
//  Copyright © 2022 Austrian Audio. All rights reserved.
//
// This Component implements an 'endless slider', which can be useful for implementing
// 'trim' controls, i.e. for applying trim to a set of EQ's, volume sliders, etc.
//
// To adjust the rate of trim, use the EndlessSlider.step value in the slider inc/dec
// callbacks.

#ifndef EndlessSlider_h
#define EndlessSlider_h

#include "../lookAndFeel/MainLookAndFeel.h"

class EndlessSlider : public Slider
{
public:
    EndlessSlider() : Slider()
    {
        currentMoved = 0;
        lastMoved = 0;
        prevMoved = 0;
        lastFilledElem = 17;
    }

    ~EndlessSlider() override {}

    // set these callbacks where you use this class in order to get inc/dec messages
    std::function<void()> sliderValueSet;
    std::function<void()> sliderReset;

    // calculate whether to callback to an increment or decrement, and update UI
    void mouseDrag (const MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            currentMoved = e.getDistanceFromDragStartY() * 1.0f;
            lastMoved = currentMoved + prevMoved;
            sliderValue = jmap (lastMoved,
                                static_cast<float> (proportionOfHeight (0.48f)),
                                (-1) * static_cast<float> (proportionOfHeight (0.52f)),
                                -0.5f,
                                1.f);
            sliderValueSet();
            lastMovedPoportion = (lastMoved / (getHeight() * 1.0f));
            repaint();
        }
    }

    void mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel) override
    {
        (void) event;
        currentMoved = -10 * wheel.deltaY;
        lastMoved = currentMoved + prevMoved;
        sliderValue = jmap (lastMoved,
                            static_cast<float> (proportionOfHeight (0.48f)),
                            (-1) * static_cast<float> (proportionOfHeight (0.52f)),
                            -0.5f,
                            1.f);
        sliderValueSet();
        lastMovedPoportion = static_cast<float> (lastMoved / getHeight());
        prevMoved = lastMoved;
        repaint();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        Rectangle<float> bounds = getLocalBounds().toFloat();
        float height = bounds.getHeight();
        int numElem = 34;
        float spaceBetween = height / static_cast<float> (numElem);
        float y = lastMoved;
        int mappedY = 0;
        int elemWidth = 0;
        int counter = 0;
        int r = static_cast<int> (sqrt (((height * height)) / 2)); // circle radius

        ColourGradient cg = ColourGradient (mainLaF.trimSliderMainColor,
                                            bounds.getWidth() / 2,
                                            bounds.getHeight() / 2,
                                            Colours::black,
                                            bounds.getWidth() / 2,
                                            0,
                                            true);
        g.setGradientFill (cg);
        g.fillRect (bounds.reduced (5, 5));

        for (int i = 0; i < numElem; i++)
        {
            if (i == 0)
            {
                y += spaceBetween / 2.f; // place first element
            }
            else
            {
                y += spaceBetween;
            }
            // calculate y when mouse out of component
            if (y > height)
            {
                counter = static_cast<int> (std::abs (y / height));
                y -= height * counter;
            }
            else if (y < 0)
            {
                counter = static_cast<int> (std::abs (y / height) + 1);
                y += height * counter;
            }
            // calculate y when mousePos in component
            if (y < height / 2)
            {
                mappedY = static_cast<int> ((-1) * (height / 2) + y);
            }
            else if (juce::approximatelyEqual (y, height / 2))
            {
                mappedY = 0;
            }
            else if (y > height / 2)
            {
                mappedY = static_cast<int> (y - height / 2);
            }
            // calculate width change with circle equation
            elemWidth = static_cast<int> (sqrt (r * r - (mappedY * mappedY)));

            auto rect = Rectangle<float> (bounds.getWidth() * 0.22f,
                                          y - (elemWidth / (numElem * 2)) / 2.0f,
                                          bounds.getWidth() * 0.55f,
                                          elemWidth / (numElem * 2.0f));

            if (i == lastFilledElem)
                filledRect = rect;
            else
            {
                g.setColour (Colours::black);
                g.fillRoundedRectangle (rect, 2.f);
            }
        }
        g.setColour (Colours::grey);
        g.fillRoundedRectangle (filledRect, 2.f);
    }

    void mouseExit (const MouseEvent& e) override
    {
        (void) e;
        repaint();
    }

    void mouseDoubleClick (const MouseEvent& e) override
    {
        (void) e;
        lastMoved = 0;
        prevMoved = 0;
        sliderReset();
        repaint();
    }

    void mouseUp (const MouseEvent& e) override
    {
        (void) e;
        prevMoved = lastMoved;
    }

    void resized() override
    {
        if (! juce::approximatelyEqual (lastMoved, 0.0f))
        {
            lastMoved = proportionOfHeight (lastMovedPoportion) * 1.0f;
            prevMoved = lastMoved;
        }
        repaint();
    }

    float getCurrentSliderValue()
    {
        // Set precision 0.00 for sliderValue
        sliderValue = std::round (sliderValue * 100.f) / 100.f;
        return sliderValue;
    }

private:
    float lastMoved;
    float currentMoved;
    int lastFilledElem;
    float prevMoved;

    //    bool dragStarted;
    //    bool isMouseUp;
    float lastMovedPoportion = 0;

    float sliderValue;
    Rectangle<float> filledRect;
    MainLookAndFeel mainLaF;
};

#endif /* EndlessSlider_h */
