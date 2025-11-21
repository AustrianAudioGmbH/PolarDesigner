/*
  ==============================================================================

    TextMultiButton.h
    Created: 30 May 2023 7:42:57pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/

#pragma once

#include "../lookAndFeel/MainLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
class TextMultiButton : public juce::Component
{
public:
    TextMultiButton() { setLookAndFeel (&mainLaF); }

    ~TextMultiButton() override
    {
        DBG ("TextMultiButton destructor called");
        textButtonArray.clear();
        setLookAndFeel (nullptr);
    }

    void AddTextButton (juce::TextButton* tb)
    {
        textButtonArray.add (tb);
        addAndMakeVisible (tb);
    }

    void setButtonsNumber (int btnNrs)
    {
        for (int i = 0; i < btnNrs; i++)
        {
            AddTextButton (new juce::TextButton);
        }
        resized();
    }

    int getSelectedButton()
    {
        for (int i = 0; i < textButtonArray.size(); i++)
        {
            if (textButtonArray.getUnchecked (i)->getToggleState())
                return (i);
        }

        return -1;
    }

    void disableAllButtons()
    {
        for (int i = 0; i < textButtonArray.size(); i++)
        {
            textButtonArray.getUnchecked (i)->setToggleState (
                false,
                juce::NotificationType::dontSendNotification);
        }
    }

    void disableAllButtonsExcept (int btnNr)
    {
        for (int i = 0; i < textButtonArray.size(); i++)
        {
            if (i != btnNr)
                textButtonArray.getUnchecked (i)->setToggleState (
                    false,
                    juce::NotificationType::dontSendNotification);
        }
    }

    juce::TextButton& operator[] (int i) { return *textButtonArray.getUnchecked (i); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (mainLaF.multiTextButtonBackgroundColor);
        g.setColour (mainLaF.textButtonFrameColor);
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        using namespace juce;

        Rectangle<int> area (getLocalBounds());

        juce::FlexBox fb;
        fb.flexDirection = FlexBox::Direction::row;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.alignContent = juce::FlexBox::AlignContent::center;

        float buttonFlex = 1.f / static_cast<float> (textButtonArray.size());

        for (TextButton* tb : textButtonArray)
        {
            fb.items.add (juce::FlexItem (*tb).withFlex (buttonFlex));
        }
        fb.performLayout (area);
    }

private:
    MainLookAndFeel mainLaF;
    juce::OwnedArray<juce::TextButton> textButtonArray;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextMultiButton)
};
