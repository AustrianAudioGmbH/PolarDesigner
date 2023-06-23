/*
  ==============================================================================

    AnimatedLabel.h
    Created: 23 Jun 2023 10:32:56am
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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
        startTimer(100);
        fontHeight = 14.f;
        addAndMakeVisible(&animatedRectangle);

    }

    ~AnimatedLabel() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        fontHeight = getTopLevelComponent()->getHeight() * 0.018f;
        textArea = getLocalBounds().reduced(getLocalBounds().getWidth() * 0.06f, (getLocalBounds().getHeight() - fontHeight)/2);
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);

        g.setColour (juce::Colours::white);
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
    const char* destString = "12345678910";
    String animatedString = "";
    int it;
    int fontHeight;
    Font textFont;

    void timerCallback() override
    {
        animatedString.append(&destString[it], 1);
        it++;
        if (it > strlen(destString))
        {
            it = 0;
            animatedString.clear();
        }
        repaint();
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
                g.fillAll(juce::Colour(255, 255, 255));

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
    } animatedRectangle;

    Rectangle<int> textArea;
    Rectangle<int> rectArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedLabel)
};
