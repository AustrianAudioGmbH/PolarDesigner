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

#pragma once

#include "../lookAndFeel/MainLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>

class EndlessSlider : public juce::Slider
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
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            currentMoved = static_cast<float> (e.getDistanceFromDragStartY());
            lastMoved = currentMoved + prevMoved;
            sliderValue = juce::jmap (lastMoved,
                                      static_cast<float> (proportionOfHeight (0.48f)),
                                      (-1) * static_cast<float> (proportionOfHeight (0.52f)),
                                      -0.5f,
                                      1.f);
            sliderValueSet();
            lastMovedPoportion = lastMoved / static_cast<float> (getHeight());
            repaint();
        }
    }

    void mouseWheelMove (const juce::MouseEvent& event,
                         const juce::MouseWheelDetails& wheel) override
    {
        (void) event;
        currentMoved = -10 * wheel.deltaY;
        lastMoved = currentMoved + prevMoved;
        sliderValue = juce::jmap (lastMoved,
                                  static_cast<float> (proportionOfHeight (0.48f)),
                                  (-1) * static_cast<float> (proportionOfHeight (0.52f)),
                                  -0.5f,
                                  1.f);
        sliderValueSet();
        lastMovedPoportion = lastMoved / static_cast<float> (getHeight());
        prevMoved = lastMoved;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        g.fillAll (Colours::black);

        Rectangle<float> bounds = getLocalBounds().toFloat();
        float height = bounds.getHeight();
        float numElem = 34;
        float spaceBetween = height / numElem;
        float y = lastMoved;
        float mappedY = 0;
        float elemWidth = 0;
        float counter = 0;
        auto r = std::sqrt (((height * height)) / 2); // circle radius

        ColourGradient cg = ColourGradient (mainLaF.trimSliderMainColor,
                                            bounds.getWidth() / 2,
                                            bounds.getHeight() / 2,
                                            Colours::black,
                                            bounds.getWidth() / 2,
                                            0,
                                            true);
        g.setGradientFill (cg);
        g.fillRect (bounds.reduced (5, 5));

        for (int i = 0; i < static_cast<int> (std::round (numElem)); i++)
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
                counter = std::round (std::abs (y / height));
                y -= height * counter;
            }
            else if (y < 0)
            {
                counter = std::round (std::abs (y / height) + 1);
                y += height * counter;
            }
            // calculate y when mousePos in component
            if (y < height / 2)
            {
                mappedY = -1 * (height / 2) + y;
            }
            else if (juce::approximatelyEqual (y, height / 2))
            {
                mappedY = 0;
            }
            else if (y > height / 2)
            {
                mappedY = y - height / 2;
            }
            // calculate width change with circle equation
            elemWidth = std::sqrt (r * r - (mappedY * mappedY));

            auto rect = Rectangle<float> (bounds.getWidth() * 0.22f,
                                          y - (elemWidth / (numElem * 2)) / 2.0f,
                                          bounds.getWidth() * 0.55f,
                                          elemWidth / (numElem * 2));

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

    void mouseExit (const juce::MouseEvent& e) override
    {
        (void) e;
        repaint();
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        (void) e;
        lastMoved = 0;
        prevMoved = 0;
        sliderReset();
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        (void) e;
        prevMoved = lastMoved;
    }

    void resized() override
    {
        if (! juce::approximatelyEqual (lastMoved, 0.0f))
        {
            lastMoved = static_cast<float> (proportionOfHeight (lastMovedPoportion));
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
    juce::Rectangle<float> filledRect;
    MainLookAndFeel mainLaF;
};
