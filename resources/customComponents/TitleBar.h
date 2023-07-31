/*
 This file is based on TitleBar.h of the IEM plug-in suite.
 Modifications by Thomas Deppisch.
*/

/*
 ==============================================================================
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

/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "TitleBarPaths.h"
#include "ImgPaths.h"
#include "../resources/lookAndFeel/MainLookAndFeel.h"

#ifdef JUCE_OSC_H_INCLUDED
#include "OSCStatus.h"
#endif

class  AlertSymbol :  public Component
{
public:
    AlertSymbol() : Component()
    {
        warningSign.loadPathFromData (WarningSignData, sizeof (WarningSignData));
        setBufferedToImage(true);
    };
    ~AlertSymbol() {};
    void paint (Graphics& g) override
    {
        warningSign.applyTransform(warningSign.getTransformToScaleToFit(getLocalBounds().toFloat(), true, Justification::centred));
        g.setColour(Colours::yellow);
        g.fillPath(warningSign);
    };
private:
    Path warningSign;
};

class IOWidget : public Component
{
public:
    IOWidget() : Component()
    {
        addChildComponent(alert);
        alert.setBounds(15, 15, 15, 15);
    };

    ~IOWidget() {};
    virtual const int getComponentSize() = 0;
    virtual void setMaxSize (int maxSize) {};

    void setBusTooSmall (bool isBusTooSmall)
    {
        busTooSmall = isBusTooSmall;
        alert.setVisible(isBusTooSmall);
    }

    bool isBusTooSmall ()
    {
        return busTooSmall;
    }

private:
    AlertSymbol alert;
    bool busTooSmall = false;
};

class  NoIOWidget :  public IOWidget
{
public:
    NoIOWidget() : IOWidget() {};
    ~NoIOWidget() {};
    const int getComponentSize() override { return 0; }
    //void paint (Graphics& g) override {};
};

class  BinauralIOWidget :  public IOWidget
{
public:
    BinauralIOWidget() : IOWidget() {
        BinauralPath.loadPathFromData (BinauralPathData, sizeof (BinauralPathData));
        setBufferedToImage(true);
    };

    ~BinauralIOWidget() {};
    const int getComponentSize() override { return 30; }
    void setMaxSize (int maxSize) override {};
    void paint (Graphics& g) override
    {
        BinauralPath.applyTransform(BinauralPath.getTransformToScaleToFit(0, 0, 30, 30, true,Justification::centred));
        g.setColour((Colours::white).withMultipliedAlpha(0.5));
        g.fillPath(BinauralPath);

    };

private:
    Path BinauralPath;
};

class  AALogo :  public IOWidget
{
public:
    AALogo() : IOWidget() {
        aaLogoPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));
    };
    
    ~AALogo() {};
    const int getComponentSize() override { return 40; }
    void setMaxSize (int maxSize) override {};

    void setLogoColour(Colour logoColour) {
        logoColour_ = logoColour;
        repaint();
    }

    void paint (Graphics& g) override
    {
        aaLogoPath.applyTransform (aaLogoPath.getTransformToScaleToFit (getLocalBounds().toFloat(), true, Justification::centred));
        g.setColour (logoColour_);
        g.strokePath (aaLogoPath, PathStrokeType (0.1f));
        g.fillPath (aaLogoPath);
    };
    
private:
    Path aaLogoPath;
    Colour logoColour_ = Colour(Colours::white);
};


template <int maxChannels, bool selectable = true>
class  AudioChannelsIOWidget :  public IOWidget
{
public:
    AudioChannelsIOWidget() : IOWidget() {
        WaveformPath.loadPathFromData (WaveformPathData, sizeof (WaveformPathData));
        setBufferedToImage(true);

        if (selectable) {
            cbChannels = new ComboBox();
            addAndMakeVisible(cbChannels);
            cbChannels->setJustificationType(Justification::centred);
            cbChannels->addSectionHeading("Number of channels");
            cbChannels->addItem("Auto", 1);
            for (int i=1; i<=maxChannels; ++i)
                cbChannels->addItem(String(i), i+1);
            cbChannels->setBounds(35, 8, 70, 15);
        }
    };
    ~AudioChannelsIOWidget() {};

    const int getComponentSize() override { return selectable ? 110 : 75; }

    void setMaxSize (int maxPossibleNumberOfChannels) override
    {
        if (selectable)
        {
            if (maxPossibleNumberOfChannels > 0) cbChannels->changeItemText(1, "Auto (" + String(maxPossibleNumberOfChannels) + ")");
            else cbChannels->changeItemText(1, "(Auto)");
            int currId = cbChannels->getSelectedId();
            if (currId == 0) currId = 1; //bad work around
            int i;
            for (i = 1; i <= maxPossibleNumberOfChannels; ++i)
            {
                cbChannels->changeItemText(i+1, String(i));
            }
            for (i = maxPossibleNumberOfChannels+1; i<=maxChannels; ++i)
            {
                cbChannels->changeItemText(i+1, String(i) + " (bus too small)");
            }
            if (maxPossibleNumberOfChannels < cbChannels->getSelectedId() - 1)
                setBusTooSmall(true);
            else
                setBusTooSmall(false);

            cbChannels->setText(cbChannels->getItemText(cbChannels->indexOfItemId((currId))));
        }
        else
        {
            if (maxPossibleNumberOfChannels < channelSizeIfNotSelectable)
            {
                displayTextIfNotSelectable = String(channelSizeIfNotSelectable) + " (bus too small)";
                setBusTooSmall(true);
            }
            else
            {
                displayTextIfNotSelectable = String(channelSizeIfNotSelectable);
                setBusTooSmall(false);
            }
            repaint();
        }
        availableChannels = maxPossibleNumberOfChannels;
    }

    void setSizeIfUnselectable (int newSize)
    {
        if (! selectable)
        {
            channelSizeIfNotSelectable = newSize;
            setMaxSize(availableChannels);
            repaint();
        }
    }

    ComboBox* getChannelsCbPointer()
    {
        if (selectable) return cbChannels;
        return nullptr;
    }

    void paint (Graphics& g) override
    {
        WaveformPath.applyTransform(WaveformPath.getTransformToScaleToFit(0, 0, 30, 30, true,Justification::centred));
        g.setColour((Colours::white).withMultipliedAlpha(0.5));
        g.fillPath(WaveformPath);

        if (!selectable)
        {
            g.setColour((Colours::white).withMultipliedAlpha(0.5));
            g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 1)));
            g.setFont(15.0f);
            g.drawFittedText(displayTextIfNotSelectable, 35, 0, 40, 30, Justification::centredLeft, 2);
        }
    };

private:
    std::unique_ptr<ComboBox> cbChannels;
    Path WaveformPath;
    int availableChannels {64};
    int channelSizeIfNotSelectable = maxChannels;
    String displayTextIfNotSelectable = String(maxChannels);
};

class  DirectivityIOWidget :  public IOWidget
{
public:
    DirectivityIOWidget() : IOWidget() {
        DirectivityPath.loadPathFromData (DirectivityPathData, sizeof (DirectivityPathData));
        setBufferedToImage(true);
        orderStrings[0] = String("0th");
        orderStrings[1] = String("1st");
        orderStrings[2] = String("2nd");
        orderStrings[3] = String("3rd");
        orderStrings[4] = String("4th");
        orderStrings[5] = String("5th");
        orderStrings[6] = String("6th");
        orderStrings[7] = String("7th");

        addAndMakeVisible(&cbOrder);
        cbOrder.setJustificationType(Justification::centred);
        cbOrder.addSectionHeading("Directivity Order");
        cbOrder.addItem("Auto", 1);
        cbOrder.addItem("0th", 2);
        cbOrder.addItem("1st", 3);
        cbOrder.addItem("2nd", 4);
        cbOrder.addItem("3rd", 5);
        cbOrder.addItem("4th", 6);
        cbOrder.addItem("5th", 7);
        cbOrder.addItem("6th", 8);
        cbOrder.addItem("7th", 9);
        cbOrder.setBounds(35, 15, 70, 15);

        addAndMakeVisible(&cbNormalization);
        cbNormalization.setJustificationType(Justification::centred);
        cbNormalization.addSectionHeading("Normalization");
        cbNormalization.addItem("N3D", 1);
        cbNormalization.addItem("SN3D", 2);
        cbNormalization.setBounds(35, 0, 70, 15);
    };

    ~DirectivityIOWidget() {};

    const int getComponentSize() override { return 110; }

    void setMaxSize (int maxPossibleOrder) override
    {
        if (maxPossibleOrder > -1) cbOrder.changeItemText(1, "Auto (" + orderStrings[maxPossibleOrder] + ")");
        else cbOrder.changeItemText(1, "(Auto)");
        int currId = cbOrder.getSelectedId();
        if (currId == 0) currId = 1; //bad work around
        int i;
        for (i = 1; i <= maxPossibleOrder; ++i)
        {
            cbOrder.changeItemText(i+2, orderStrings[i]);
        }
        for (i = maxPossibleOrder+1; i<=7; ++i)
        {
            cbOrder.changeItemText(i+2, orderStrings[i] + " (bus too small)");
        }
        cbOrder.setText(cbOrder.getItemText(cbOrder.indexOfItemId((currId))));
        if (currId - 2> maxPossibleOrder)
            setBusTooSmall(true);
        else
            setBusTooSmall(false);

    }

    ComboBox* getNormCbPointer() { return &cbNormalization; }
    ComboBox* getOrderCbPointer() { return &cbOrder; }

    void paint (Graphics& g) override
    {
        DirectivityPath.applyTransform(DirectivityPath.getTransformToScaleToFit(0, 0, 30, 30, true,Justification::centred));
        g.setColour((Colours::white).withMultipliedAlpha(0.5));
        g.fillPath(DirectivityPath);
    };

private:
    String orderStrings[8];
    ComboBox cbNormalization, cbOrder;
    Path DirectivityPath;
};

class  TitleBarPDText : public Component
{
public:
    TitleBarPDText() {};
    ~TitleBarPDText() {};

    void setTitle(String newRegularText) {
        regularText = newRegularText;
    }

    void setFont(Typeface::Ptr newRegularFont) {
        regularFont = newRegularFont;
    }

    void resized() override
    {
        repaint();
    }

    void paint(Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        regularFont.setHeight(bounds.getHeight()*0.75f);

        g.setColour(isEnabled() ? mainLaF.mainTextColor : mainLaF.mainTextDisabledColor);
        g.setFont(regularFont);
        g.drawFittedText(regularText, bounds.toNearestInt(), Justification::left, 1);
    };

private:
    Font regularFont = Font(22.f);
    juce::String regularText = "Regular";
    MainLookAndFeel mainLaF;
};

class  TitleBarTextLabel : public Component
{
public:
    TitleBarTextLabel() {};
    ~TitleBarTextLabel() {};

    void setTitle(String newRegularText) {
        regularText = newRegularText;
    }

    String &getTitle() {
        return regularText;
    }

    void setFont(Typeface::Ptr newRegularFont) {
        regularFont = newRegularFont;
    }

    void resized() override
    {
        repaint();
    }

    void paint(Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        int fontSize = getTopLevelComponent()->getHeight() * 0.025f;
        regularFont.setHeight(fontSize);

        g.setColour(isEnabled() ? mainLaF.mainTextColor : mainLaF.mainTextDisabledColor);
        g.setFont(regularFont);
        g.drawFittedText(regularText, bounds.toNearestInt(), Justification::right, 1);
    };

private:
    Font regularFont = Font(16.f);
    juce::String regularText = "Regular";
    MainLookAndFeel mainLaF;
};

class IEMLogo : public Component
{
public:
    IEMLogo() : Component()
    {
        IEMPath.loadPathFromData (IEMpathData, sizeof (IEMpathData));
//        url = URL("https://plugins.iem.at/");
    }
    ~IEMLogo() {};

    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        bounds.removeFromBottom (3);
        bounds.removeFromLeft (1);
        IEMPath.applyTransform(IEMPath.getTransformToScaleToFit(bounds.reduced(2, 2).toFloat(), true, Justification::bottomLeft));

//        if (isMouseOver())
//        {
//            g.setColour(Colour::fromRGB(52, 88, 165));
//            g.fillAll();
//        }

        g.setColour(Colours::white.withMultipliedAlpha(0.5));
        g.fillPath(IEMPath);
    }

//    void mouseEnter(const MouseEvent &event) override
//    {
//        setMouseCursor(MouseCursor(MouseCursor::PointingHandCursor));
//        repaint();
//    }
//
//    void mouseExit(const MouseEvent &event) override
//    {
//        setMouseCursor(MouseCursor(MouseCursor::NormalCursor));
//        repaint();
//    }
//
//    void mouseUp(const MouseEvent &event) override
//    {
//        if (url.isWellFormed())
//            url.launchInDefaultBrowser();
//    }

private:
    Path IEMPath;
//    URL url;
};

class  Footer :  public Component
{
public:
    Footer() : Component()
    {
        addAndMakeVisible(&iemLogo);
    };
    ~Footer() {};

    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        g.setColour(Colours::white.withAlpha(0.5f));
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 0)));
        g.setFont(14.0f);
        String versionString = "v";

    #if JUCE_DEBUG
        versionString = "DEBUG - v";
    #endif
        versionString.append(JucePlugin_VersionString, 6);

        g.drawText(versionString, 0, 0, bounds.getWidth() - 8,bounds.getHeight()-2, Justification::bottomRight);
    };

    void resized () override
    {
        iemLogo.setBounds(0, 0, 40, getHeight());

    }

private:
    IEMLogo iemLogo;
};

#ifdef JUCE_OSC_H_INCLUDED
class  OSCFooter :  public Component
{
public:
    OSCFooter (OSCReceiverPlus& oscReceiver) : oscStatus (oscReceiver)
    {
        addAndMakeVisible (footer);
        addAndMakeVisible (oscStatus);
    };
    ~OSCFooter() {};

    void resized () override
    {
        auto bounds = getLocalBounds();
        footer.setBounds (bounds);

        bounds.removeFromBottom (2);
        bounds = bounds.removeFromBottom (16);
        bounds.removeFromLeft (50);
        oscStatus.setBounds (bounds.removeFromLeft (80));
    }

private:
    OSCStatus oscStatus;
    Footer footer;
};
#endif
