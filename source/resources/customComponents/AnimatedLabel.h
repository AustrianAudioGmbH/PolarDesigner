/*
  ==============================================================================

    AnimatedLabel.h
    Created: 23 Jun 2023 10:32:56am
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include "../lookAndFeel/MainLookAndFeel.h"

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

//==============================================================================
/*
*/
class AnimatedLabel : public juce::Component, private juce::Timer
{
public:
    AnimatedLabel()
    {
        it = 0;
        counter = 1;
        fontHeight = 14;
        repaintBypassed = false;
        timerBypassedPeriods = 0;
        addAndMakeVisible (&animatedRectangle);
        animatedRectangle.setVisible (false);
    }

    ~AnimatedLabel() override {}

    void startAnimation (const String primText, const String secText = {})
    {
        it = 0;
        counter = 1;
        fontHeight = 14;
        repaintBypassed = false;
        timerBypassedPeriods = 0;
        animatedString.clear();
        startTimer (80);
        setLabelText (primText, secText);
        animatedRectangle.setVisible (true);
    }

    void stopAnimation()
    {
        it = 0;
        counter = 1;
        fontHeight = 14;
        repaintBypassed = false;
        timerBypassedPeriods = 0;
        animatedString.clear();
        stopTimer();
        animatedRectangle.setVisible (false);
    }

    void paint (juce::Graphics& g) override
    {
        fontHeight = static_cast<float> (getTopLevelComponent()->getHeight()) * 0.018f;
        textArea = getLocalBounds().reduced (
            static_cast<int> (static_cast<float> (getLocalBounds().getWidth()) * 0.06f),
            (getLocalBounds().getHeight() - static_cast<int> (fontHeight)) / 2);
        g.fillAll (mainLaF.labelBackgroundColor);

        g.setColour (mainLaF.mainTextColor);
        g.setFont (fontHeight);

        g.drawMultiLineText (animatedString,
                             textArea.getX(),
                             textArea.getY() + static_cast<int> (fontHeight) / 2,
                             textArea.getWidth(),
                             juce::Justification::left,
                             true);
    }

    void resized() override
    {
        fontHeight = static_cast<float> (getTopLevelComponent()->getHeight()) * 0.018f;

        Font font (fontHeight * 1.0f);
        textArea = getLocalBounds().reduced (
            static_cast<int> (static_cast<float> (getLocalBounds().getWidth()) * 0.06f),
            (getLocalBounds().getHeight() - static_cast<int> (fontHeight)) / 2);
        auto centredTextArea = Rectangle<int> (textArea.getX(),
                                               textArea.getY() - static_cast<int> (fontHeight) / 4,
                                               textArea.getWidth(),
                                               static_cast<int> (fontHeight));
        int equalSignWidth =
            static_cast<int> (static_cast<float> (getLocalBounds().getWidth()) * 0.042f);
        float rowProportion = static_cast<float> (font.getStringWidth (animatedString))
                              / static_cast<float> (centredTextArea.getWidth());

        if (animatedString.length() > 0)
        {
            auto currStringLength = animatedString.length();
            equalSignWidth = font.getStringWidth (animatedString) / animatedString.length();
            if (font.getStringWidth (animatedString) > centredTextArea.getWidth())
            {
                int nrOfSignsInNewRow =
                    static_cast<int> ((rowProportion - 1.f)
                                      * (static_cast<float> (centredTextArea.getWidth())
                                         / static_cast<float> (equalSignWidth)));
                rectArea = centredTextArea.withWidth (equalSignWidth)
                               .translated (equalSignWidth * nrOfSignsInNewRow,
                                            static_cast<int> (fontHeight));
            }
            else
            {
                rectArea = centredTextArea.withWidth (equalSignWidth)
                               .translated (equalSignWidth * currStringLength, 0);
            }
        }
        else
        {
            rectArea = centredTextArea.withWidth (equalSignWidth);
        }
        animatedRectangle.setBounds (rectArea);
    }

private:
    String animatedString;
    int it;
    float fontHeight;
    Font textFont;
    int timerBypassedPeriods;
    bool repaintBypassed;
    String text;
    String primaryText;
    String secondaryText;
    int counter;

    void setLabelText (const String primText, const String secText = {})
    {
        primaryText = primText;
        secondaryText = secText;
        text = primaryText;
    }

    void timerCallback() override
    {
        auto charStr = text.getCharPointer();

        if (! repaintBypassed && it < text.length())
        {
            animatedString += charStr[it];
            it++;
            repaint();
        }
        else
        {
            it = 0;
            repaintBypassed = true;
            timerBypassedPeriods++;
            if (timerBypassedPeriods > 30)
            {
                counter++;
                repaintBypassed = false;
                text =
                    (! secondaryText.isEmpty() && counter % 2 == 0) ? secondaryText : primaryText;
                timerBypassedPeriods = 0;
                animatedString.clear();
            }
        }
        resized();
    }

    class AnimatedRectangle : public juce::Component, private juce::Timer
    {
    public:
        AnimatedRectangle() { startTimer (300); }

        ~AnimatedRectangle() override {}

        void paint (juce::Graphics& g) override
        {
            if (show)
                g.fillAll (mainLaF.mainTextColor);
        }

        void resized() override {}

    private:
        bool show = false;
        void timerCallback() override
        {
            show = ! show;
            repaint();
        }
        MainLookAndFeel mainLaF;
    } animatedRectangle;

    Rectangle<int> textArea;
    Rectangle<int> rectArea;
    MainLookAndFeel mainLaF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedLabel)
};
