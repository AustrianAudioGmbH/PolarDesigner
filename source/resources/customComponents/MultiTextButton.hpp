/*
  ==============================================================================

    TextMultiButton.hpp
    Created: 30 May 2023 7:42:57pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/

#pragma once

#include "Colours.hpp"
#include "TextButton.hpp"
#include "juce_core/juce_core.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
namespace AAGuiComponents
{

template <ButtonColor color>
class MultiTextButton : public juce::Component
{
public:
    MultiTextButton (int btnsNrs)
    {
        for (int i = 0; i < btnsNrs; i++)
        {
            AddTextButton (new MultiTextButtonComponent<color>());
            addAndMakeVisible (textButtonArray.getLast());
        }
    }

    ~MultiTextButton() override
    {
        DBG ("TextMultiButton destructor called");
        textButtonArray.clear();
    }

    void AddTextButton (MultiTextButtonComponent<color>* tb) { textButtonArray.add (tb); }

    void setButtonsNumber (int btnNrs)
    {
        for (int i = 0; i < btnNrs; i++)
        {
            AddTextButton (new MultiTextButtonComponent<color>);
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

    juce::TextButton& operator[] (int i) { return getButton (i); }

    juce::TextButton& getButton (int i) { return *textButtonArray.getUnchecked (i); }

    void setButtonText (int buttonNumber, juce::String text)
    {
        textButtonArray.getUnchecked (buttonNumber)->setButtonText (text);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Colours::multiTextButtonBackgroundColor);
        g.fillRect (getLocalBounds());

        g.setColour (Colours::textButtonFrameColor);
        g.drawRect (getLocalBounds());
    }

    void resized() override
    {
        using namespace juce;

        Rectangle<int> area (getLocalBounds());
        const auto buttonWidth =
            textButtonArray.size() > 0
                ? area.toFloat().getWidth() / static_cast<float> (textButtonArray.size())
                : area.toFloat().getWidth();
        for (juce::TextButton* tb : textButtonArray)
            tb->setBounds (area.removeFromLeft (roundToInt (buttonWidth)).reduced (1));
    }

private:
    juce::OwnedArray<MultiTextButtonComponent<color>> textButtonArray;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTextButton)
};
} // namespace AAGuiComponents
