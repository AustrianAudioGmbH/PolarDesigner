/*
  ==============================================================================

    TextMultiButton.h
    Created: 30 May 2023 7:42:57pm
    Author:  Mikolaj Cikuj

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

//==============================================================================
/*
*/
class TextMultiButton  : public juce::Component
{
public:
    TextMultiButton()
    {
        setLookAndFeel(&mainLaF);
    }

    ~TextMultiButton() override
    {
        DBG("TextMultiButton destructor called");
        textButtonArray.clear();
        setLookAndFeel(nullptr);

    }

    void AddTextButton(TextButton* tb)
    {
        textButtonArray.add(tb);
        addAndMakeVisible(tb);
    }

    void setButtonsNumber(int btnNrs)
    {
        for (int i = 0; i < btnNrs; i++)
        {
            AddTextButton(new TextButton);
        }
        resized();
    }

    int getSelectedButton()
    {
        for (int i = 0; i < textButtonArray.size(); i++)
        {
            if (textButtonArray.getUnchecked(i)->getToggleState())
                return (i);
        }

        return -1;
    }

    void disableAllButtons() {
        for (int i = 0; i < textButtonArray.size(); i++)
        {
            textButtonArray.getUnchecked(i)->setToggleState(false, juce::NotificationType::dontSendNotification);
        }
    }

    void disableAllButtonsExcept (int btnNr)
    {
        for (int i = 0; i < textButtonArray.size(); i++)
        {
            if (i != btnNr)
            textButtonArray.getUnchecked(i)->setToggleState(false, juce::NotificationType::dontSendNotification);
        }
    }
  
    TextButton &operator[](int i)
    {
        return *textButtonArray.getUnchecked(i);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (mainLaF.multiTextButtonBackgroundColor);
        g.setColour(mainLaF.textButtonFrameColor);
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        Rectangle<int> area(getLocalBounds());

        juce::FlexBox fb;
        fb.flexDirection = FlexBox::Direction::row;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.alignContent = juce::FlexBox::AlignContent::center;

        float buttonFlex = 1.f / textButtonArray.size();

        for (TextButton *tb : textButtonArray)
        {
            fb.items.add(juce::FlexItem(*tb).withFlex(buttonFlex));
        }
        fb.performLayout(area);
    }

private:
    MainLookAndFeel mainLaF;
    OwnedArray<TextButton> textButtonArray;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextMultiButton)
};
