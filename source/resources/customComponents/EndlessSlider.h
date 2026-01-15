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

#include "../../Constants.hpp"
#include "../lookAndFeel/MainLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class EndlessSlider : public juce::Slider
{
public:
    EndlessSlider() : Slider()
    {
        lastMoved = 0;
        prevMoved = 0;
        lastFilledElem = 17;
    }

    ~EndlessSlider() override {}

    // set these callbacks where you use this class in order to get inc/dec messages

    // calculate whether to callback to an increment or decrement, and update UI
    void mouseDrag (const juce::MouseEvent& e) override
    {
        const auto currentMoved = static_cast<float> (e.getDistanceFromDragStartY());
        lastMoved = currentMoved + prevMoved;

        const auto height = static_cast<float> (getHeight());
        lastMovedPoportion = lastMoved / height;
        repaint();

        constexpr auto sensitivity = 1.5f;
        const auto sliderValue = -currentMoved / height * sensitivity;

        for (size_t i = 0; i < elements.size(); ++i)
            if (elements[i] != nullptr)
                elements[i]->setValueNotifyingHost (
                    elements[i]->convertTo0to1 (startPositions[i] + sliderValue));
    }

    void mouseWheelMove (const juce::MouseEvent& event,
                         const juce::MouseWheelDetails& wheel) override
    {
        using namespace juce;
        ignoreUnused (event);

        const auto currentMoved = -10 * wheel.deltaY;
        lastMoved = currentMoved + prevMoved;
        const auto height = static_cast<float> (getHeight());
        lastMovedPoportion = lastMoved / height;
        prevMoved = lastMoved;
        repaint();

        for (size_t i = 0; i < elements.size(); ++i)
            if (elements[i] != nullptr)
            {
                auto& element = elements[i];
                const auto oldValue = element->convertFrom0to1 (element->getValue());
                const auto newValue = oldValue - currentMoved / height;
                element->setValueNotifyingHost (element->convertTo0to1 (newValue));
            }
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
        repaint();

        for (auto& el : elements)
            el->setValueNotifyingHost (el->convertTo0to1 (0.0f));
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        (void) e;
        prevMoved = lastMoved;

        for (size_t i = 0; i < elements.size(); ++i)
            if (elements[i] != nullptr)
                startPositions[i] = elements[i]->convertFrom0to1 (elements[i]->getValue());
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        using namespace juce;

        ignoreUnused (e);

        for (size_t i = 0; i < elements.size(); ++i)
            if (elements[i] != nullptr)
                startPositions[i] = elements[i]->convertFrom0to1 (elements[i]->getValue());
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

    void setElement (size_t index, juce::RangedAudioParameter* elementToSet)
    {
        elements[index] = elementToSet;
    }

private:
    std::array<juce::RangedAudioParameter*, MAX_NUM_EQS> elements;
    std::array<float, MAX_NUM_EQS> startPositions;

    float lastMoved;
    int lastFilledElem;
    float prevMoved;

    float lastMovedPoportion = 0;

    juce::Rectangle<float> filledRect;
    MainLookAndFeel mainLaF;
};
