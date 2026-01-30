/*
  ==============================================================================

    AnimatedLabel.h
    Created: 23 Jun 2023 10:32:56am
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include "../lookAndFeel/MainLookAndFeel.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
class AnimatedLabel : public juce::Component, private juce::Timer
{
public:
    AnimatedLabel() : textFont (juce::FontOptions())
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

    void startAnimation (const juce::String primText, const juce::String secText = {})
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
        using namespace juce;

        const auto* editor = findParentComponentOfClass<AudioProcessorEditor>();
        fontHeight =
            static_cast<float> (editor ? editor->getHeight() : getTopLevelComponent()->getHeight())
            * 0.018f;
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
        using namespace juce;

        const auto* editor = findParentComponentOfClass<AudioProcessorEditor>();
        fontHeight =
            static_cast<float> (editor ? editor->getHeight() : getTopLevelComponent()->getHeight())
            * 0.018f;

        Font font (FontOptions { fontHeight });
        textArea = getLocalBounds().reduced (
            static_cast<int> (static_cast<float> (getLocalBounds().getWidth()) * 0.06f),
            (getLocalBounds().getHeight() - static_cast<int> (fontHeight)) / 2);
        auto centredTextArea =
            juce::Rectangle<int> (textArea.getX(),
                                  textArea.getY() - static_cast<int> (fontHeight) / 4,
                                  textArea.getWidth(),
                                  static_cast<int> (fontHeight));
        int equalSignWidth =
            static_cast<int> (static_cast<float> (getLocalBounds().getWidth()) * 0.042f);
        float rowProportion =
            static_cast<float> (GlyphArrangement::getStringWidth (font, animatedString))
            / static_cast<float> (centredTextArea.getWidth());

        if (animatedString.length() > 0)
        {
            auto currStringLength = animatedString.length();
            equalSignWidth =
                static_cast<int> (GlyphArrangement::getStringWidth (font, animatedString)
                                  / static_cast<float> (animatedString.length()));
            if (GlyphArrangement::getStringWidth (font, animatedString)
                > static_cast<float> (centredTextArea.getWidth()))
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
    juce::String animatedString;
    int it;
    float fontHeight;
    juce::Font textFont;
    int timerBypassedPeriods;
    bool repaintBypassed;
    juce::String text;
    juce::String primaryText;
    juce::String secondaryText;
    int counter;

    void setLabelText (const juce::String primText, const juce::String secText = {})
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

    juce::Rectangle<int> textArea;
    juce::Rectangle<int> rectArea;
    MainLookAndFeel mainLaF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedLabel)
};
