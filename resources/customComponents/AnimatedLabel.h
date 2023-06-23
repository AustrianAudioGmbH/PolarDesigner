/*
  ==============================================================================

    AnimatedLabel.h
    Created: 23 Jun 2023 10:32:56am
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../resources/lookAndFeel/MainLookAndFeel.h"
//==============================================================================
/*
*/
class AnimatedLabel  : public juce::Component,
    private juce::Timer
{
public:
    AnimatedLabel()
    {
        it = 0;
        fontHeight = 14.f;
        repaintBypassed = false;
        timerBypassedPeriods = 0;
        addAndMakeVisible(&animatedRectangle);
        animatedRectangle.setVisible(false);
    }

    ~AnimatedLabel() override
    {
    }

    void startAnimation()
    {
        it = 0;
        fontHeight = 14.f;
        repaintBypassed = false;
        timerBypassedPeriods = 0;
        animatedString.clear();
        startTimer(100);
        animatedRectangle.setVisible(true);
    }

    void stopAnimation()
    {
        it = 0;
        fontHeight = 14.f;
        repaintBypassed = false;
        timerBypassedPeriods = 0;
        animatedString.clear();
        stopTimer();
        animatedRectangle.setVisible(false);
    }

    void paint (juce::Graphics& g) override
    {
        fontHeight = getTopLevelComponent()->getHeight() * 0.018f;
        textArea = getLocalBounds().reduced(getLocalBounds().getWidth() * 0.06f, (getLocalBounds().getHeight() - fontHeight)/2);
        g.fillAll (mainLaF.labelBackgroundColor);

        g.setColour (mainLaF.mainTextColor);
        g.setFont (fontHeight);

        g.drawText (animatedString, textArea,
                    juce::Justification::left, true);
    }

    void resized() override
    {
        fontHeight = getTopLevelComponent()->getHeight() * 0.018f;

        Font font(fontHeight);
        textArea = getLocalBounds().reduced(getLocalBounds().getWidth() * 0.06f, (getLocalBounds().getHeight() - fontHeight) / 2);
        int equalSignWidth = getLocalBounds().getWidth() * 0.03f;

        if (animatedString.length() > 0)
        {
            equalSignWidth = font.getStringWidth(animatedString) / animatedString.length();
            rectArea = textArea.withWidth(equalSignWidth).translated(equalSignWidth * animatedString.length(), 0);
        }
        else
        {
            rectArea = textArea.withWidth(equalSignWidth);
        }
        animatedRectangle.setBounds(rectArea);
    }

private:
    String animatedString;
    int it;
    int fontHeight;
    Font textFont;
    int timerBypassedPeriods;
    bool repaintBypassed;

    void timerCallback() override
    {
        auto charStr =  getTitle().getCharPointer();
        if (!repaintBypassed && it < getTitle().length())
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
                repaintBypassed = false;
                timerBypassedPeriods = 0;
                animatedString.clear();
            }
        }
        resized();
    }

    class AnimatedRectangle : public juce::Component,
        private juce::Timer
    {
    public:
        AnimatedRectangle()
        {
            startTimer(300);
        }

        ~AnimatedRectangle() override
        {
        }

        void paint(juce::Graphics& g) override
        {
            if (show)
                g.fillAll(mainLaF.mainTextColor);
        }

        void resized() override
        {
        }

    private:
        bool show = false;
        void timerCallback() override
        {
            show = !show;
            repaint();
        }
        MainLookAndFeel mainLaF;
    } animatedRectangle;

    Rectangle<int> textArea;
    Rectangle<int> rectArea;
    MainLookAndFeel mainLaF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedLabel)
};
