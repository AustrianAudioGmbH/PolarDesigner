/*
  ==============================================================================

    TextMultiButton.h
    Created: 30 May 2023 7:42:57pm
    Author:  Mikolaj Cikuj

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../resources/lookAndFeel/MainLookAndFeel.h"

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
        setLookAndFeel(nullptr);
        textButtonArray.clear();
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
