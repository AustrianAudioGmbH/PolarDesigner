/*
 ==============================================================================
 PluginEditor.cpp
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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
PolarDesignerAudioProcessorEditor::PolarDesignerAudioProcessorEditor (PolarDesignerAudioProcessor& p,
                                                          AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), loadingFile(false), processor (p), valueTreeState(vts),
    dEQ (p), alOverlayError(AlertOverlay::Type::errorMessage),
    alOverlayDisturber(AlertOverlay::Type::disturberTracking),
    alOverlaySignal(AlertOverlay::Type::signalTracking)
{
    nActiveBands = processor.getNBands();
    syncChannelIdx = processor.getSyncChannelIdx();
    
    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("PolarDesigner"));
    title.setFont (globalLaF.avenirMedium,globalLaF.avenirRegular);
    
    addAndMakeVisible (&footer);
    
    addAndMakeVisible (&alOverlayError);
    alOverlayError.setVisible(false);
    alOverlayError.setColour(AlertWindow::backgroundColourId, globalLaF.AAGrey);
    alOverlayError.setColour(TextButton::buttonColourId, globalLaF.AARed);
    
    addAndMakeVisible (&alOverlayDisturber);
    alOverlayDisturber.setVisible(false);
    alOverlayDisturber.setColour(AlertWindow::backgroundColourId, globalLaF.AAGrey);
    alOverlayDisturber.setColour(TextButton::buttonColourId , globalLaF.AARed);
    alOverlayDisturber.setTitle("acquiring target!");
    alOverlayDisturber.setMessage("Make sure playback of an undesired target signal (spill) is active. Terminate to apply polar patterns with minimum spill energy. Also track a desired signal to be able to maximize the target-to-spill ratio.");
    
    addAndMakeVisible (&alOverlaySignal);
    alOverlaySignal.setVisible(false);
    alOverlaySignal.setColour(AlertWindow::backgroundColourId, globalLaF.AAGrey);
    alOverlaySignal.setColour(TextButton::buttonColourId , globalLaF.AARed);
    alOverlaySignal.setTitle("acquiring target!");
    alOverlaySignal.setMessage("Make sure playback of a desired target signal is active. Stop signal tracking to apply polar patterns with maximum signal energy. Also track an undesired spill target to be able to maximize the target-to-spill ratio.");

    // groups
    addAndMakeVisible (&grpEq);
    grpEq.setText ("equalization control");
    grpEq.setTextLabelPosition (Justification::centredLeft);
    
    addAndMakeVisible (&grpBands);
    grpBands.setText ("band control");
    grpBands.setTextLabelPosition (Justification::centredLeft);
    
    addAndMakeVisible (&grpPreset);
    grpPreset.setText ("preset control");
    grpPreset.setTextLabelPosition (Justification::centredLeft);
    
    addAndMakeVisible (&grpDstC);
    grpDstC.setText ("terminator control");
    grpDstC.setTextLabelPosition (Justification::centredLeft);
    
    addAndMakeVisible (&grpProxComp);
    grpProxComp.setText ("proximity control");
    grpProxComp.setTextLabelPosition (Justification::centredLeft);
    
    addAndMakeVisible (&grpSync);
    grpSync.setText ("sync to channel");
    grpSync.setTextLabelPosition (Justification::centredLeft);
    
    Colour eqColours[5] = {
        Colour(0xFDBA4949),
        Colour(0xFDBA6F49),
        Colour(0xFDBAAF49),
        Colour(0xFD8CBA49),
        Colour(0xFD49BA64)
    };
    
    // directivity eq
    addAndMakeVisible (&dEQ);
    
    for (int i = 0; i < nBands; ++i)
    {
        // buttons
        msbSolo[i].setType (MuteSoloButton::Type::solo);
        addAndMakeVisible (&msbSolo[i]);
        msbSoloAtt[i] = new ButtonAttachment (valueTreeState, "solo" + String(i+1), msbSolo[i]);
        msbSolo[i].addListener (this);
        msbSolo[i].setAlwaysOnTop (true);
        
        msbMute[i].setType (MuteSoloButton::Type::mute);
        addAndMakeVisible (&msbMute[i]);
        msbMuteAtt[i] = new ButtonAttachment (valueTreeState, "mute" + String(i+1), msbMute[i]);
        msbMute[i].addListener (this);
        msbMute[i].setAlwaysOnTop (true);
        
        // sliders
        addAndMakeVisible (&slDir[i]);
        slDirAtt[i] = new SliderAttachment (valueTreeState, "alpha" + String(i+1), slDir[i]);
        slDir[i].setColour (Slider::thumbColourId, eqColours[i]); // colour of knob
        slDir[i].addListener (this);
        slDir[i].setTooltipEditable (true);
        
        addAndMakeVisible (&slBandGain[i]);
        slBandGainAtt[i] = new ReverseSlider::SliderAttachment (valueTreeState, "gain" + String(i+1), slBandGain[i]);
        slBandGain[i].setSliderStyle (Slider::LinearHorizontal);
        slBandGain[i].setColour (Slider::rotarySliderOutlineColourId, eqColours[i]);
        slBandGain[i].setColour (Slider::thumbColourId, eqColours[i]);
        slBandGain[i].setTextBoxStyle (Slider::TextBoxAbove, false, 50, 15);
        slBandGain[i].addListener (this);
        
        // directivity visualizer
        addAndMakeVisible (&dv[i]);
        dv[i].setDirWeight (slDir[i].getValue());
        dv[i].setMuteSoloButtons (&msbSolo[i], &msbMute[i]);
        dv[i].setColour (eqColours[i]);
        
        dEQ.addSliders (eqColours[i], &slDir[i], (i > 0) ? &slXover[i - 1] : nullptr, (i < nBands - 1) ? &slXover[i] : nullptr, &msbSolo[i], &msbMute[i]);
        
        if (i == nBands - 1)
            break; // there is one slXover less than bands
        
        addAndMakeVisible (&slXover[i]);
        slXoverAtt[i] = new ReverseSlider::SliderAttachment (valueTreeState, "xOverF" + String(i+1), slXover[i]);
        slXover[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slXover[i].addListener(this);
        slXover[i].setVisible(false);
    }
    
    dEQ.initValueBox();
    
    addAndMakeVisible (&tbLoadFile);
    tbLoadFile.setButtonText ("load preset");
    tbLoadFile.addListener (this);
    
    addAndMakeVisible (&tbSaveFile);
    tbSaveFile.setButtonText ("save preset");
    tbSaveFile.addListener (this);
    
    addAndMakeVisible (&tbRecordDisturber);
    tbRecordDisturber.setButtonText ("terminate spill");
    tbRecordDisturber.addListener (this);
    
    addAndMakeVisible (&tbRecordSignal);
    tbRecordSignal.setButtonText ("maximize target");
    tbRecordSignal.addListener (this);
    
    addAndMakeVisible (&tbAllowBackwardsPattern);
    tbAllowBackwardsPatternAtt = new ButtonAttachment (valueTreeState, "allowBackwardsPattern", tbAllowBackwardsPattern);
    tbAllowBackwardsPattern.setButtonText ("allow reverse patterns");
    tbAllowBackwardsPattern.addListener (this);
    
    addAndMakeVisible (&tbEq[0]);
    tbEq[0].addListener (this);
    tbEq[0].setButtonText ("off");
    tbEq[0].setRadioGroupId(1);
    
    addAndMakeVisible (&tbEq[1]);
    tbEq[1].addListener (this);
    tbEq[1].setButtonText ("free field");
    tbEq[1].setRadioGroupId(1);
    
    addAndMakeVisible (&tbEq[2]);
    tbEq[2].addListener (this);
    tbEq[2].setButtonText ("diffuse field");
    tbEq[2].setRadioGroupId(1);
    
    addAndMakeVisible (&cbSetNrBands);
    cbSetNrBandsAtt = new ComboBoxAttachment (valueTreeState, "nrBands", cbSetNrBands);
    cbSetNrBands.setEditableText (false);
    cbSetNrBands.addItemList (juce::StringArray ({"one band","two bands",
                                                 "three bands","four bands",
                                                 "five bands"}), 1);
    cbSetNrBands.setJustificationType (Justification::centred);
    cbSetNrBands.setSelectedId (nActiveBands);
    cbSetNrBands.addListener (this);
    
    addAndMakeVisible (&cbSyncChannel);
    cbSyncChannelAtt = new ComboBoxAttachment (valueTreeState, "syncChannel", cbSyncChannel);
    cbSyncChannel.setEditableText (false);
    cbSyncChannel.addItemList (juce::StringArray ({"none","one","two","three","four"}), 1);
    cbSyncChannel.setJustificationType (Justification::centred);
    cbSyncChannel.setSelectedId (syncChannelIdx);
    cbSyncChannel.addListener (this);
    
    addAndMakeVisible (&slProximity);
    slProximityAtt = new ReverseSlider::SliderAttachment (valueTreeState, "proximity", slProximity);
    slProximity.setSliderStyle (Slider::LinearHorizontal);
    slProximity.setColour (Slider::thumbColourId, globalLaF.AARed);
    slProximity.setColour (Slider::rotarySliderOutlineColourId, globalLaF.AARed);
    slProximity.setTextBoxStyle (Slider::TextBoxRight, false, 45, 15);
    slProximity.addListener (this);
    
    addAndMakeVisible (&tbZeroDelay);
    tbZeroDelayAtt = new ButtonAttachment (valueTreeState, "zeroDelayMode", tbZeroDelay);
    tbZeroDelay.addListener (this);
    tbZeroDelay.setButtonText ("zero latency");
    tbZeroDelay.setToggleState(processor.zeroDelayModeActive(), NotificationType::dontSendNotification);
    
    dEQ.setSoloActive (getSoloActive());
    for (auto& vis : dv)
    {
        vis.setSoloActive (getSoloActive());
    }
    
    nActiveBandsChanged();
    zeroDelayModeChange();
    
    // set overlay callbacks
    alOverlayError.setOnOkayCallback ([this]() { onAlOverlayErrorOkay(); });
    
    alOverlayDisturber.setOnOkayCallback ([this]() { onAlOverlayApplyPattern(); });
    alOverlayDisturber.setOnCancelCallback ([this]() { onAlOverlayCancelRecord(); });
    alOverlayDisturber.setOnRatioCallback ([this]() { onAlOverlayMaxSigToDist(); });
    
    alOverlaySignal.setOnOkayCallback ([this]() { onAlOverlayApplyPattern(); });
    alOverlaySignal.setOnCancelCallback ([this]() { onAlOverlayCancelRecord(); });
    alOverlaySignal.setOnRatioCallback ([this]() { onAlOverlayMaxSigToDist(); });
    
    startTimer (30);
    
    setEqMode();
}

PolarDesignerAudioProcessorEditor::~PolarDesignerAudioProcessorEditor()
{
    if (alOverlayDisturber.isVisible())
        onAlOverlayCancelRecord();
    
    if (alOverlaySignal.isVisible())
        onAlOverlayCancelRecord();
    
    setLookAndFeel (nullptr);
}

//==============================================================================
void PolarDesignerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void PolarDesignerAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int dirSliderHeight = 30;
    const int linearSliderWidth = 115;
    const int linearSliderSpacing = 32;
    const int linearSliderHeight = 20;
    const int buttonHeight = 18;
    const int vSpace = 10;
    const int vSpaceMain = 30;
    const int hSpace = 30;
    const int buttonVSpace = 5;
    const int loadButtonWidth = 110;
    const int loadButtonHeight = 28;
    const int loadButtonMargin = 5;
    const int cbWidth = 140;
    const int dEqHeight = 280;
    const int dvHeight = 120;
    const int dvSpacing = 25;
    const int lbWidth = 120;
    const int sideAreaWidth = 180;
    const int sideAreaRightMargin = 20;
    const int sideVSpace = 20;
    const int grpHeight = 25;
    
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setTitleCentreX (headerArea.withLeft(sideAreaWidth).getX() + 0.5 * headerArea.withLeft(sideAreaWidth).getWidth() - 8);
    title.setBounds (headerArea);
    
    Rectangle<int> zDArea = headerArea.removeFromRight(90);
    zDArea.removeFromTop(headerHeight/2 - loadButtonHeight/2);
    tbZeroDelay.setBounds(zDArea.removeFromTop(loadButtonHeight));
    
    // --------- SIDE AREA ------------
    Rectangle<int> sideArea (area.removeFromLeft(sideAreaWidth));
    sideArea.removeFromRight(sideAreaRightMargin);
    sideArea.removeFromTop(vSpace);
    sideBorderPath.startNewSubPath(sideArea.getRight(), sideArea.getY());
    sideBorderPath.lineTo(sideArea.getRight(), sideArea.getBottom());
    sideArea.removeFromRight(leftRightMargin/2);
    
    // set Bands Combobox
    grpBands.setBounds(sideArea.removeFromTop(grpHeight));
    Rectangle<int> changeBandsArea = sideArea.removeFromTop(25);
    cbSetNrBands.setBounds(changeBandsArea.removeFromLeft(cbWidth));
    
    // load file button
    sideArea.removeFromTop(sideVSpace);
    grpPreset.setBounds(sideArea.removeFromTop(grpHeight));
    Rectangle<int> fileLoadArea = sideArea.removeFromTop(loadButtonHeight);
    fileLoadArea.removeFromLeft(loadButtonMargin);
    tbLoadFile.setBounds(fileLoadArea.removeFromLeft(loadButtonWidth));
    
    // save file button
    sideArea.removeFromTop(vSpace/2);
    Rectangle<int> fileSaveArea = sideArea.removeFromTop(loadButtonHeight);
    fileSaveArea.removeFromLeft(loadButtonMargin);
    tbSaveFile.setBounds(fileSaveArea.removeFromLeft(loadButtonWidth));
    
    // ff/df equalization
    sideArea.removeFromTop(sideVSpace);
    grpEq.setBounds(sideArea.removeFromTop(grpHeight));
    Rectangle<int> eqArea = sideArea.removeFromTop(3*buttonHeight + 2*buttonVSpace);
    tbEq[0].setBounds(eqArea.getX(), eqArea.getY(), buttonHeight + lbWidth, buttonHeight);
    tbEq[1].setBounds(eqArea.getX(), eqArea.getY() + buttonHeight, buttonHeight + lbWidth, buttonHeight);
    tbEq[2].setBounds(eqArea.getX(), eqArea.getY() + 2 * buttonHeight, buttonHeight + lbWidth, buttonHeight);
    
    // proximity compensation
    sideArea.removeFromTop(sideVSpace);
    grpProxComp.setBounds(sideArea.removeFromTop(grpHeight));
    slProximity.setBounds(sideArea.removeFromTop(linearSliderHeight));
    
    // disturber cancellation
    sideArea.removeFromTop(sideVSpace);
    grpDstC.setBounds(sideArea.removeFromTop(grpHeight));
    Rectangle<int> recordArea = sideArea.removeFromTop(buttonHeight + 2 * loadButtonHeight + vSpace);
    tbAllowBackwardsPattern.setBounds(recordArea.removeFromTop(buttonHeight).removeFromLeft(buttonHeight + lbWidth));
    recordArea.removeFromTop(vSpace/2);
    recordArea.removeFromLeft(loadButtonMargin);
    tbRecordDisturber.setBounds(recordArea.removeFromTop(loadButtonHeight).removeFromLeft(loadButtonWidth));
    recordArea.removeFromTop(vSpace/2);
    tbRecordSignal.setBounds(recordArea.removeFromTop(loadButtonHeight).removeFromLeft(loadButtonWidth));
    
    // set syncChannel
    sideArea.removeFromTop(sideVSpace);
    grpSync.setBounds(sideArea.removeFromTop(grpHeight));
    Rectangle<int> syncArea = sideArea.removeFromTop(25);
    cbSyncChannel.setBounds(syncArea.removeFromLeft(cbWidth));
    
    // -------------- MAIN AREA -------------
    Rectangle<int> mainArea = area.removeFromTop(EDITOR_HEIGHT - headerHeight - footerHeight);
    mainArea.removeFromTop(vSpaceMain);
    
    // dv
    Rectangle<int> dvRow = mainArea.removeFromTop(dvHeight);
    dvRow.removeFromLeft(hSpace);
    for (auto& vis : dv)
    {
        vis.setBounds(dvRow.removeFromLeft(dvHeight));
        dvRow.removeFromLeft(dvSpacing);
    }
    
    // dEq
    Rectangle<int> filterArea = mainArea.removeFromTop (dEqHeight + 2 * dirSliderHeight + vSpaceMain + buttonHeight);
    dEQ.setBounds (filterArea.removeFromTop(dEqHeight));
    alOverlayError.setBounds (dEQ.getX() + 120, dEQ.getY() + 50, dEQ.getWidth() - 240, dEQ.getHeight() - 100);
    alOverlayDisturber.setBounds (dEQ.getX() + 120, dEQ.getY() + 50, dEQ.getWidth() - 240, dEQ.getHeight() - 100);
    alOverlaySignal.setBounds (dEQ.getX() + 120, dEQ.getY() + 50, dEQ.getWidth() - 240, dEQ.getHeight() - 100);
    
    filterArea.removeFromTop(vSpaceMain);
    filterArea.removeFromLeft(hSpace);
    Rectangle<int> band0SliderArea = filterArea.removeFromLeft(linearSliderWidth);
    slDir[0].setBounds(band0SliderArea.removeFromTop(dirSliderHeight));
    msbSolo[0].setBounds(band0SliderArea.getX(), band0SliderArea.getY(), buttonHeight, buttonHeight);
    msbMute[0].setBounds(band0SliderArea.getRight() - buttonHeight, band0SliderArea.getY(), buttonHeight, buttonHeight);
    band0SliderArea.removeFromTop(2);
    slBandGain[0].setBounds(band0SliderArea);

    filterArea.removeFromLeft(linearSliderSpacing);
    Rectangle<int> band1SliderArea = filterArea.removeFromLeft(linearSliderWidth);
    slDir[1].setBounds(band1SliderArea.removeFromTop(dirSliderHeight));
    msbSolo[1].setBounds(band1SliderArea.getX(), band1SliderArea.getY(), buttonHeight, buttonHeight);
    msbMute[1].setBounds(band1SliderArea.getRight() - buttonHeight, band1SliderArea.getY(), buttonHeight, buttonHeight);
    band1SliderArea.removeFromTop(2);
    slBandGain[1].setBounds(band1SliderArea);
    
    filterArea.removeFromLeft(linearSliderSpacing);
    Rectangle<int> band2SliderArea = filterArea.removeFromLeft(linearSliderWidth);
    slDir[2].setBounds(band2SliderArea.removeFromTop(dirSliderHeight));
    msbSolo[2].setBounds(band2SliderArea.getX(), band2SliderArea.getY(), buttonHeight, buttonHeight);
    msbMute[2].setBounds(band2SliderArea.getRight() - buttonHeight, band2SliderArea.getY(), buttonHeight, buttonHeight);
    band2SliderArea.removeFromTop(2);
    slBandGain[2].setBounds(band2SliderArea);
    
    filterArea.removeFromLeft(linearSliderSpacing);
    Rectangle<int> band3SliderArea = filterArea.removeFromLeft(linearSliderWidth);
    slDir[3].setBounds(band3SliderArea.removeFromTop(dirSliderHeight));
    msbSolo[3].setBounds(band3SliderArea.getX(), band3SliderArea.getY(), buttonHeight, buttonHeight);
    msbMute[3].setBounds(band3SliderArea.getRight() - buttonHeight, band3SliderArea.getY(), buttonHeight, buttonHeight);
    band3SliderArea.removeFromTop(2);
    slBandGain[3].setBounds(band3SliderArea);
    
    filterArea.removeFromLeft(linearSliderSpacing);
    Rectangle<int> band4SliderArea = filterArea.removeFromLeft(linearSliderWidth);
    slDir[4].setBounds(band4SliderArea.removeFromTop(dirSliderHeight));
    msbSolo[4].setBounds(band4SliderArea.getX(), band4SliderArea.getY(), buttonHeight, buttonHeight);
    msbMute[4].setBounds(band4SliderArea.getRight() - buttonHeight, band4SliderArea.getY(), buttonHeight, buttonHeight);
    band4SliderArea.removeFromTop(2);
    slBandGain[4].setBounds(band4SliderArea);
}

void PolarDesignerAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &tbLoadFile)
    {
        loadFile();
    }
    else if (button == &tbSaveFile)
    {
        saveFile();
    }
    else if (button == &tbEq[0])
    {
        processor.setEqState(0);
    }
    else if (button == &tbEq[1])
    {
        processor.setEqState(1);
    }
    else if (button == &tbEq[2])
    {
        processor.setEqState(2);
    }
    else if (button == &tbRecordDisturber)
    {
        processor.startTracking(true);
        alOverlayDisturber.enableRatioButton(processor.getSignalRecorded());
        alOverlayDisturber.setVisible(true);
        disableMainArea();
        setSideAreaEnabled(false);
    }
    else if (button == &tbRecordSignal)
    {
        processor.startTracking(false);
        alOverlaySignal.enableRatioButton(processor.getDisturberRecorded());
        alOverlaySignal.setVisible(true);
        disableMainArea();
        setSideAreaEnabled(false);
    }
    else if (button == &tbAllowBackwardsPattern)
    {
        return;
    }
    else if (button == &tbZeroDelay)
    {
        bool isToggled = button->getToggleState();
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
    }
    else // muteSoloButton!
    {
        dEQ.setSoloActive(getSoloActive());
        dEQ.repaint();
        for (auto& vis : dv)
        {
            vis.setSoloActive(getSoloActive());
            vis.repaint();
        }
    }
}

bool PolarDesignerAudioProcessorEditor::getSoloActive()
{
    bool active = false;
    for (auto& but : msbSolo)
    {
        if (but.getToggleState())
        {
            active = true;
            break;
        }
    }
    return active;
}

void PolarDesignerAudioProcessorEditor::comboBoxChanged (ComboBox* cb)
{
    nActiveBands = cb->getSelectedId();
}

void PolarDesignerAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &slXover[0] || slider == &slXover[1] || slider == &slXover[2] || slider == &slXover[3])
    {
        // xOverSlider
        return;
    }
    else
    {
        // dirSlider
        for (int i = 0; i < 5; i++)
        {
            if (slider == &slDir[i])
                dv[i].setDirWeight(slider->getValue());
        }
    }
}

void PolarDesignerAudioProcessorEditor::loadFile()
{
    FileChooser myChooser ("Select Preset File",
                           processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToOpen())
    {
        loadingFile = true;
        File presetFile (myChooser.getResult());
        processor.setLastDir(presetFile.getParentDirectory());
        Result result = processor.loadPreset (presetFile);
        if (!result.wasOk()) {
            errorMessage = result.getErrorMessage();
            alOverlayError.setTitle("preset load error!");
            alOverlayError.setMessage(errorMessage);
            alOverlayError.setVisible(true);
            disableMainArea();
            setSideAreaEnabled(false);
        }
        else
        {
            setEqMode();
        }
        loadingFile = false;
    }
}

void PolarDesignerAudioProcessorEditor::saveFile()
{
    FileChooser myChooser ("Save Preset File",
                           processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToSave (true))
    {
        File presetFile (myChooser.getResult());
        processor.setLastDir(presetFile.getParentDirectory());
        Result result = processor.savePreset (presetFile);
        if (!result.wasOk()) {
            errorMessage = result.getErrorMessage();
            alOverlayError.setTitle("preset save error!");
            alOverlayError.setMessage(errorMessage);
            alOverlayError.setVisible(true);
            disableMainArea();
            setSideAreaEnabled(false);
        }
    }
}

void PolarDesignerAudioProcessorEditor::nActiveBandsChanged()
{
    nActiveBands = processor.getNBands();
    for (int i = 0; i < 5; i++)
    {
        if (i < nActiveBands)
        {
            slDir[i].setEnabled(true);
            slBandGain[i].setEnabled(true);
            msbSolo[i].setEnabled(true);
            msbMute[i].setEnabled(true);
            dv[i].setActive(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            msbSolo[i].setEnabled(false);
            msbSolo[i].setToggleState(false, NotificationType::sendNotification);
            msbMute[i].setEnabled(false);
            msbMute[i].setToggleState(false, NotificationType::sendNotification);
            dv[i].setActive(false);
        }
    }

    dEQ.resetTooltipTexts();
    dEQ.repaint();
        
}

void PolarDesignerAudioProcessorEditor::timerCallback()
{
    if (processor.repaintDEQ.get())
    {
        processor.repaintDEQ = false;
        dEQ.repaint();
    }
    if (processor.nActiveBandsChanged.get())
    {
        processor.nActiveBandsChanged = false;
        nActiveBandsChanged();
    }
    if (processor.zeroDelayModeChanged.get())
    {
        processor.zeroDelayModeChanged = false;
        zeroDelayModeChange();
    }
    if (processor.ffDfEqChanged.get())
    {
        processor.ffDfEqChanged = false;
        setEqMode();
    }
}

void PolarDesignerAudioProcessorEditor::zeroDelayModeChange()
{
    tbZeroDelay.setToggleState(processor.zeroDelayModeActive(), NotificationType::dontSendNotification);
    
    int nActive = nActiveBands;
    
    if (processor.zeroDelayModeActive())
        nActive = 1;
    
    setSideAreaEnabled(!processor.zeroDelayModeActive());
    
    for (int i = 0; i < 5; i++)
    {
        if (i < nActive)
        {
            slDir[i].setEnabled(true);
            slBandGain[i].setEnabled(true);
            msbSolo[i].setEnabled(true);
            msbMute[i].setEnabled(true);
            dv[i].setActive(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            msbSolo[i].setEnabled(false);
            msbSolo[i].setToggleState(false, NotificationType::sendNotification);
            msbMute[i].setEnabled(false);
            msbMute[i].setToggleState(false, NotificationType::sendNotification);
            dv[i].setActive(false);
        }
    }
    
    dEQ.resetTooltipTexts();
    dEQ.repaint();
}

void PolarDesignerAudioProcessorEditor::disableMainArea()
{
    dEQ.setActive(false);
    for (int i = 0; i < 5; i++)
    {
        slDir[i].setEnabled(false);
        slBandGain[i].setEnabled(false);
        msbSolo[i].setEnabled(false);
        msbMute[i].setEnabled(false);
        dv[i].setActive(false);
    }
    tbZeroDelay.setEnabled(false);
}

void PolarDesignerAudioProcessorEditor::onAlOverlayErrorOkay()
{
    disableOverlay();
}

void PolarDesignerAudioProcessorEditor::onAlOverlayApplyPattern()
{
    disableOverlay();
    processor.stopTracking(1);
}

void PolarDesignerAudioProcessorEditor::onAlOverlayCancelRecord()
{
    disableOverlay();
    processor.stopTracking(0);
}

void PolarDesignerAudioProcessorEditor::onAlOverlayMaxSigToDist()
{
    disableOverlay();
    processor.stopTracking(2);
}

void PolarDesignerAudioProcessorEditor::setSideAreaEnabled(bool set)
{
    cbSetNrBands.setEnabled(set);
    cbSyncChannel.setEnabled(set);
    tbLoadFile.setEnabled(set);
    tbSaveFile.setEnabled(set);
    tbEq[0].setEnabled(set);
    tbEq[1].setEnabled(set);
    tbEq[2].setEnabled(set);
    tbAllowBackwardsPattern.setEnabled(set);
    tbRecordDisturber.setEnabled(set);
    tbRecordSignal.setEnabled(set);
    slProximity.setEnabled(set);
}

void PolarDesignerAudioProcessorEditor::setEqMode()
{
    int activeIdx = processor.getEqState();
    tbEq[activeIdx].setToggleState(true, NotificationType::sendNotification);
}

void PolarDesignerAudioProcessorEditor::disableOverlay()
{
    alOverlayError.setVisible(false);
    alOverlayDisturber.setVisible(false);
    alOverlaySignal.setVisible(false);
    dEQ.setActive(true);
    nActiveBandsChanged();
    setSideAreaEnabled(true);
    tbZeroDelay.setEnabled(true);
}

// implement this for AAX automation shortchut
int PolarDesignerAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &dEQ.getBandlimitPathComponent(0) && nActiveBands > 1)
        return 0;
    else if (&control == &dEQ.getBandlimitPathComponent(1) && nActiveBands > 2)
        return 1;
    else if (&control == &dEQ.getBandlimitPathComponent(2) && nActiveBands > 3)
        return 2;
    else if (&control == &dEQ.getBandlimitPathComponent(3) && nActiveBands > 4)
        return 3;
    else if (&control == &slDir[0] || &control == &dEQ.getDirPathComponent(0))
        return 4;
    else if ((&control == &slDir[1] || &control == &dEQ.getDirPathComponent(1)) && nActiveBands > 1)
        return 5;
    else if ((&control == &slDir[2] || &control == &dEQ.getDirPathComponent(2)) && nActiveBands > 2)
        return 6;
    else if ((&control == &slDir[3] || &control == &dEQ.getDirPathComponent(3)) && nActiveBands > 3)
        return 7;
    else if ((&control == &slDir[4] || &control == &dEQ.getDirPathComponent(4)) && nActiveBands > 4)
        return 8;
    else if (&control == &msbSolo[0])
        return 9;
    else if (&control == &msbSolo[1] && nActiveBands > 1)
        return 10;
    else if (&control == &msbSolo[2] && nActiveBands > 2)
        return 11;
    else if (&control == &msbSolo[3] && nActiveBands > 3)
        return 12;
    else if (&control == &msbSolo[4] && nActiveBands > 4)
        return 13;
    else if (&control == &msbMute[0])
        return 14;
    else if (&control == &msbMute[1] && nActiveBands > 1)
        return 15;
    else if (&control == &msbMute[2] && nActiveBands > 2)
        return 16;
    else if (&control == &msbMute[3] && nActiveBands > 3)
        return 17;
    else if (&control == &msbMute[4] && nActiveBands > 4)
        return 18;
    else if (&control == &slBandGain[0])
        return 19;
    else if (&control == &slBandGain[1] && nActiveBands > 1)
        return 20;
    else if (&control == &slBandGain[2] && nActiveBands > 2)
        return 21;
    else if (&control == &slBandGain[3] && nActiveBands > 3)
        return 22;
    else if (&control == &slBandGain[4] && nActiveBands > 4)
        return 23;
    else if (&control == &slProximity)
        return 26;
    
    return -1;
}

