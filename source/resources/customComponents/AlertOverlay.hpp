/*
 ==============================================================================
 AlertOverlay.h
 Author: Thomas Deppisch
 
 Copyright (c) 2019 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
*/

#pragma once
#include "PolarPatternVisualizer.h"

class AlertOverlay : public Component, private Button::Listener, private Timer
{
public:
    enum Type
    {
        errorMessage,
        disturberTracking,
        signalTracking
    };

    AlertOverlay (Type newType) : Component(), type (newType)
    {
        setAlwaysOnTop (true);
        Colour buttonColour = findColour (TextButton::buttonColourId); //Colour(155,35,35);

        if (type == Type::errorMessage)
        {
            tbOkay = new TextButton();
            addAndMakeVisible (tbOkay);
            tbOkay->setButtonText ("ok");
            tbOkay->addListener (this);
            tbOkay->setColour (TextButton::buttonColourId, buttonColour);
        }
        else if (type == Type::disturberTracking)
        {
            tbOkay = new TextButton();
            addAndMakeVisible (tbOkay);
            tbOkay->setButtonText ("terminate");
            tbOkay->addListener (this);
            tbOkay->setColour (TextButton::buttonColourId, buttonColour);
            tbOkay->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            tbRatio = new TextButton();
            addAndMakeVisible (tbRatio);
            tbRatio->setButtonText ("max target-to-spill");
            tbRatio->addListener (this);
            tbRatio->setColour (TextButton::buttonColourId, buttonColour);
            tbRatio->setEnabled (false);
            tbRatio->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            tbCancel = new TextButton();
            addAndMakeVisible (tbCancel);
            tbCancel->setButtonText ("cancel");
            tbCancel->addListener (this);
            tbCancel->setColour (TextButton::buttonColourId, buttonColour);
            tbCancel->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            ppVisualizer = new PolarPatternVisualizer();
            addAndMakeVisible (ppVisualizer);
            ppVisualizer->setColour (buttonColour);
            ppVisualizer->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            startTimer (30);
        }
        else if (type == Type::signalTracking)
        {
            tbOkay = new TextButton();
            addAndMakeVisible (tbOkay);
            tbOkay->setButtonText ("maximize");
            tbOkay->addListener (this);
            tbOkay->setColour (TextButton::buttonColourId, buttonColour);
            tbOkay->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            tbRatio = new TextButton();
            addAndMakeVisible (tbRatio);
            tbRatio->setButtonText ("max target-to-spill");
            tbRatio->addListener (this);
            tbRatio->setColour (TextButton::buttonColourId, buttonColour);
            tbRatio->setEnabled (false);
            tbRatio->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            tbCancel = new TextButton();
            addAndMakeVisible (tbCancel);
            tbCancel->setButtonText ("cancel");
            tbCancel->addListener (this);
            tbCancel->setColour (TextButton::buttonColourId, buttonColour);
            tbCancel->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            ppVisualizer = new PolarPatternVisualizer();
            addAndMakeVisible (ppVisualizer);
            ppVisualizer->setColour (buttonColour);
            ppVisualizer->setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            setMouseCursor (MouseCursor::StandardCursorType::CrosshairCursor);

            startTimer (30);
        }
    }

    ~AlertOverlay() override
    {
        delete tbOkay;
        if (type == Type::disturberTracking || type == Type::signalTracking)
        {
            delete tbCancel;
            delete tbRatio;
            stopTimer();
            delete ppVisualizer;
        }
    }

    void paint (Graphics& g) override
    {
        int height = getHeight();
        int width = getWidth();
        const int messageHeight = height - titleHeight - mT - mB - buttonHeight;

        g.setColour ((findColour (AlertWindow::backgroundColourId)).withAlpha (0.95f));
        g.fillRoundedRectangle (0, 0, width * 1.0f, height * 1.0f, 5.0f);

        g.setColour (Colours::black);
        g.setFont (getLookAndFeel().getAlertWindowTitleFont());
        g.drawFittedText (title, mL, mT, width - mL - mR, titleHeight, Justification::topLeft, 1);

        g.setFont (getLookAndFeel().getAlertWindowMessageFont());
        if (type == errorMessage)
        {
            g.drawFittedText (message,
                              mL + horSpace,
                              mT + titleHeight + textMargin,
                              width - mL - mR,
                              messageHeight,
                              Justification::topLeft,
                              4,
                              1.0f);
        }
        else if (type == disturberTracking || type == Type::signalTracking)
        {
            g.drawFittedText (message,
                              mL + dvWidth + horSpace,
                              mT + titleHeight + textMargin,
                              width - mL - mR - dvWidth,
                              messageHeight,
                              Justification::topLeft,
                              5,
                              1.0f);
        }
    }

    void resized() override
    {
        int height = getHeight();
        int width = getWidth();
        const int errorButtonWidth = 80;
        const int trackingButtonWidth = 94;
        const int buttonMargin = 15;

        if (type == Type::errorMessage)
        {
            tbOkay->setBounds (width / 2 - errorButtonWidth / 2,
                               height - buttonHeight - mB,
                               errorButtonWidth,
                               buttonHeight);
        }
        else if (type == Type::disturberTracking || type == Type::signalTracking)
        {
            tbOkay->setBounds (mL + dvWidth + horSpace,
                               height - buttonHeight - mB,
                               trackingButtonWidth,
                               buttonHeight);
            tbRatio->setBounds (mL + dvWidth + trackingButtonWidth + buttonMargin + horSpace,
                                height - buttonHeight - mB,
                                trackingButtonWidth,
                                buttonHeight);
            tbCancel->setBounds (mL + dvWidth + 2 * trackingButtonWidth + 2 * buttonMargin
                                     + horSpace,
                                 height - buttonHeight - mB,
                                 trackingButtonWidth,
                                 buttonHeight);
            ppVisualizer->setBounds (mL, mT + titleHeight + textMargin - 10, dvWidth, dvWidth);
        }
    }

    void buttonClicked (Button* button) override
    {
        if (button == tbOkay)
        {
            if (type == Type::disturberTracking || type == Type::signalTracking)
                stopTimer();

            onOkayCallback();
        }
        else if (button == tbCancel)
        {
            if (type == Type::disturberTracking || type == Type::signalTracking)
                stopTimer();

            onCancelCallback();
        }
        else if (button == tbRatio)
        {
            if (type == Type::disturberTracking || type == Type::signalTracking)
                stopTimer();

            onRatioCallback();
        }
    }

    void visibilityChanged() override
    {
        if (type == Type::disturberTracking || type == Type::signalTracking)
            isVisible() ? startTimer (30) : stopTimer();
    }

    void setOnOkayCallback (std::function<void()> cb) { onOkayCallback = cb; }

    void setOnCancelCallback (std::function<void()> cb) { onCancelCallback = cb; }

    void setOnRatioCallback (std::function<void()> cb) { onRatioCallback = cb; }

    void setTitle (String newTitle) { title = newTitle; }
    void setMessage (String newMessage) { message = newMessage; }

    void enableRatioButton (bool enable)
    {
        if (tbRatio != nullptr)
        {
            tbRatio->setEnabled (enable);
        }
    }

    void colourChanged() override
    {
        Colour buttonColour = findColour (TextButton::buttonColourId);
        if (type == Type::errorMessage)
        {
            tbOkay->setColour (TextButton::buttonColourId, buttonColour);
        }
        else if (type == Type::disturberTracking || type == Type::signalTracking)
        {
            tbOkay->setColour (TextButton::buttonColourId, buttonColour);
            tbRatio->setColour (TextButton::buttonColourId, buttonColour);
            tbCancel->setColour (TextButton::buttonColourId, buttonColour);
            ppVisualizer->setColour (buttonColour);
        }
    }

    void timerCallback() override
    {
        if (ppVisualizer != nullptr)
        {
            if (dirWeight >= 1.0f)
                goDown = true;
            else if (dirWeight <= 0.0f)
                goDown = false;

            if (goDown)
                dirWeight -= 0.01f;
            else
                dirWeight += 0.01f;

            ppVisualizer->setDirWeight (dirWeight);
        }
    }

private:
    // margins
    const int mL = 20;
    const int mR = 20;
    const int mT = 20;
    const int mB = 15;

    const int titleHeight = 20;
    const int buttonHeight = 30;
    const int dvWidth = 100;
    const int textMargin = 20;
    const int horSpace = 10;

    TextButton* tbOkay;
    TextButton* tbCancel;
    TextButton* tbRatio;

    PolarPatternVisualizer* ppVisualizer;

    Type type;
    String title;
    String message;

    std::function<void()> onOkayCallback;
    std::function<void()> onCancelCallback;
    std::function<void()> onRatioCallback;

    float dirWeight = 0.0f;
    bool goDown = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlertOverlay)
};
