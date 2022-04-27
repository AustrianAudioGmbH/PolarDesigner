/*
 ==============================================================================
 PluginEditor.cpp
 Author: Thomas Deppisch & Simon Beck
 
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
    directivityEqualiser (p), alOverlayError(AlertOverlay::Type::errorMessage),
    alOverlayDisturber(AlertOverlay::Type::disturberTracking),
    alOverlaySignal(AlertOverlay::Type::signalTracking)
{
//    openGLContext.attachTo (*getTopLevelComponent());
    
    nActiveBands = processor.getNBands();
    syncChannelIdx = processor.getSyncChannelIdx();
    
    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("PolarDesigner"));
    title.setFont (globalLaF.aaMedium,globalLaF.aaRegular);
    
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
    grpSync.setText ("sync-channel");
    grpSync.setTextLabelPosition (Justification::centredLeft);
    
    eqColours[0] = Colour(0xFDBA4949);
    eqColours[1] = Colour(0xFDBA6F49);
    eqColours[2] = Colour(0xFDBAAF49);
    eqColours[3] = Colour(0xFD8CBA49);
    eqColours[4] = Colour(0xFD49BA64);
    
    // directivity eq
    addAndMakeVisible (&directivityEqualiser);
    
    for (int i = 0; i < nBands; ++i)
    {
        // SOLO button
        msbSolo[i].setType (MuteSoloButton::Type::solo);
        addAndMakeVisible (&msbSolo[i]);
        msbSoloAtt[i] = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "solo" + String(i+1), msbSolo[i]));
        msbSolo[i].addListener (this);
        msbSolo[i].setAlwaysOnTop (true);
        
        // MUTE button
        msbMute[i].setType (MuteSoloButton::Type::mute);
        addAndMakeVisible (&msbMute[i]);
        msbMuteAtt[i] = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "mute" + String(i+1), msbMute[i]));
        msbMute[i].addListener (this);
        msbMute[i].setAlwaysOnTop (true);
        
        // Direction slider
        addAndMakeVisible (&slDir[i]);
        slDirAtt[i] = std::unique_ptr<SliderAttachment>(new SliderAttachment (valueTreeState, "alpha" + String(i+1), slDir[i]));
        slDir[i].setColour (Slider::thumbColourId, eqColours[i]); // colour of knob
        slDir[i].addListener (this);
        slDir[i].setTooltipEditable (true);
        
        // Band Gain slider
        addAndMakeVisible (&slBandGain[i]);
        slBandGainAtt[i] = std::unique_ptr<ReverseSlider::SliderAttachment>(new ReverseSlider::SliderAttachment (valueTreeState, "gain" + String(i+1), slBandGain[i]));
        slBandGain[i].setSliderStyle (Slider::LinearHorizontal);
        slBandGain[i].setColour (Slider::rotarySliderOutlineColourId, eqColours[i]);
        slBandGain[i].setColour (Slider::thumbColourId, eqColours[i]);
        slBandGain[i].setTextBoxStyle (Slider::TextBoxAbove, false, 50, 15);
        slBandGain[i].addListener (this);
        
        // First-Order directivity visualizer (The "O"verhead view)
        addAndMakeVisible (&polarPatterVisualizers[i]);
        polarPatterVisualizers[i].setDirWeight (slDir[i].getValue());
        polarPatterVisualizers[i].setMuteSoloButtons (&msbSolo[i], &msbMute[i]);
        polarPatterVisualizers[i].setColour (eqColours[i]);

        // main directivity Equaliser section
        directivityEqualiser.addSliders (eqColours[i], &slDir[i], (i > 0) ? &slCrossoverPosition[i - 1] : nullptr, (i < nBands - 1) ? &slCrossoverPosition[i] : nullptr, &msbSolo[i], &msbMute[i], &slBandGain[i], &polarPatterVisualizers[i]);
        
        if (i == nBands - 1)
            break; // there is one slCrossoverPosition less than bands
        
        addAndMakeVisible (&slCrossoverPosition[i]);
        slCrossoverAtt[i] = std::unique_ptr<ReverseSlider::SliderAttachment>(new ReverseSlider::SliderAttachment (valueTreeState, "xOverF" + String(i+1), slCrossoverPosition[i]));
        slCrossoverPosition[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slCrossoverPosition[i].addListener(this);
        slCrossoverPosition[i].setVisible(false);
    }
    
    directivityEqualiser.initValueBox();
    
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
    tbAllowBackwardsPatternAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "allowBackwardsPattern", tbAllowBackwardsPattern));
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
    
    addAndMakeVisible (&tbAbButton[0]);
    tbAbButton[0].addListener (this);
    tbAbButton[0].setButtonText("A");
    tbAbButton[0].setToggleState(processor.abLayerState, NotificationType::dontSendNotification);
    tbAbButton[0].setClickingTogglesState(true);
    tbAbButton[0].setAlpha(getABButtonAlphaFromLayerState(processor.abLayerState));
    tbAbButton[0].setRadioGroupId(2);
    
    addAndMakeVisible (&tbAbButton[1]);
    tbAbButton[1].addListener(this);
    tbAbButton[1].setButtonText("B");
    tbAbButton[1].setToggleState(!processor.abLayerState, NotificationType::dontSendNotification);
    tbAbButton[1].setClickingTogglesState(true);
    tbAbButton[1].setAlpha(getABButtonAlphaFromLayerState(!processor.abLayerState));
    tbAbButton[1].setRadioGroupId(2);

    addAndMakeVisible (&cbSetNrBands);
    cbSetNrBandsAtt = std::unique_ptr<ComboBoxAttachment>(new ComboBoxAttachment (valueTreeState, "nrBands", cbSetNrBands));
    cbSetNrBands.setEditableText (false);
    cbSetNrBands.addItemList (juce::StringArray ({"one band","two bands",
                                                 "three bands","four bands",
                                                 "five bands"}), 1);
    cbSetNrBands.setJustificationType (Justification::centred);
    cbSetNrBands.setSelectedId (nActiveBands);
    cbSetNrBands.addListener (this);
    
    addAndMakeVisible (&cbSyncChannel);
    cbSyncChannelAtt = std::unique_ptr<ComboBoxAttachment>(new ComboBoxAttachment (valueTreeState, "syncChannel", cbSyncChannel));
    cbSyncChannel.setEditableText (false);
    cbSyncChannel.addItemList (juce::StringArray ({"none","one","two","three","four"}), 1);
    cbSyncChannel.setJustificationType (Justification::centred);
    cbSyncChannel.setSelectedId (syncChannelIdx);
    cbSyncChannel.addListener (this);
    
    addAndMakeVisible (&slProximity);
    slProximityAtt = std::unique_ptr<ReverseSlider::SliderAttachment>(new ReverseSlider::SliderAttachment (valueTreeState, "proximity", slProximity));
    slProximity.setSliderStyle (Slider::LinearHorizontal);
    slProximity.setColour (Slider::thumbColourId, globalLaF.AARed);
    slProximity.setColour (Slider::rotarySliderOutlineColourId, globalLaF.AARed);
    slProximity.setTextBoxStyle (Slider::TextBoxRight, false, 45, 15);
    slProximity.addListener (this);
    
    addAndMakeVisible (&tbZeroDelay);
    tbZeroDelayAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "zeroDelayMode", tbZeroDelay));
    tbZeroDelay.addListener (this);
    tbZeroDelay.setButtonText ("zero latency");
    tbZeroDelay.setToggleState(processor.zeroDelayModeActive(), NotificationType::dontSendNotification);
    
    directivityEqualiser.setSoloActive (getSoloActive());
    for (auto& vis : polarPatterVisualizers)
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
    
    addAndMakeVisible(&trimSlider);
//    trimSlider.addListener(this); // !J! not needed, use slider inc / dec callbacks instead
    trimSlider.sliderIncremented = [this] { incrementTrim(); };
    trimSlider.sliderDecremented = [this] { decrementTrim(); };
    
    startTimer (30);
    
    setEqMode();
}

// Handle the trimSlider increment/decrement calls
void PolarDesignerAudioProcessorEditor::incrementTrim() {
    for (int i = 0; i < 5; i++)
    {
        slDir[i].setValue(slDir[i].getValue() + trimSlider.step);
    }
}

void PolarDesignerAudioProcessorEditor::decrementTrim() {
    for (int i = 0; i < 5; i++)
    {
        slDir[i].setValue(slDir[i].getValue() - trimSlider.step);
    }
}

PolarDesignerAudioProcessorEditor::~PolarDesignerAudioProcessorEditor()
{
    if (alOverlayDisturber.isVisible())
        onAlOverlayCancelRecord();
    
    if (alOverlaySignal.isVisible())
        onAlOverlayCancelRecord();
    
    setLookAndFeel (nullptr);

//    openGLContext.detach();

}

//==============================================================================
void PolarDesignerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
        
#ifdef AA_DO_DEBUG_PATH
    g.strokePath (debugPath, PathStrokeType (15.0f));
#endif
    
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
    const int pvHeight = 120;
    const int pvSpacing = 25;
    const int lbWidth = 120;
    const int sideAreaWidth = 180;
    const int sideAreaRightMargin = 20;
    const int sideVSpace = 20;
    const int grpHeight = 25;
    const int trimSliderWidth = 25;
        
    Rectangle<int> area (getLocalBounds());
   
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    
   
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    
    title.setTitleCentreX (headerArea.withLeft(sideAreaWidth).getX() + 0.5 *
                           headerArea.withLeft(sideAreaWidth).getWidth() - 8);
    title.setBounds (headerArea);
    
    Rectangle<int> zDArea = headerArea.removeFromRight(90);
    zDArea.removeFromTop(headerHeight/2 - loadButtonHeight/2);
    tbZeroDelay.setBounds(zDArea.removeFromTop(loadButtonHeight));
    
    Rectangle<int> abArea = headerArea.removeFromRight(3 * loadButtonHeight);
    abArea.removeFromTop(headerHeight/2 - loadButtonHeight/2);
    tbAbButton[0].setBounds(abArea.getX(), abArea.getY(), loadButtonHeight, loadButtonHeight);
    tbAbButton[1].setBounds(abArea.getX() + 1.5 * loadButtonHeight, abArea.getY(), loadButtonHeight, loadButtonHeight);
    
    
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

#ifdef AA_DO_DEBUG_PATH
    { // !J! for debugging purposes only
        debugPath.clear();
        debugPath.addRectangle(mainArea);
    }
#endif
   
    // polar Visualizers
    Rectangle<int> pvRow = mainArea.removeFromTop(pvHeight);
    pvRow.removeFromLeft(hSpace);
    
    for (auto& pVis : polarPatterVisualizers)
    {
        pVis.setBounds(pvRow.removeFromLeft(pvHeight));
        pvRow.removeFromLeft(pvSpacing);
    }

    // dEq
    Rectangle<int> filterArea = mainArea.removeFromTop (dEqHeight + 2 * dirSliderHeight + vSpaceMain + buttonHeight);

    Rectangle<int> trimSliderArea = filterArea.removeFromRight(trimSliderWidth);
    trimSliderArea.removeFromBottom(dirSliderHeight + buttonHeight + 2);

    directivityEqualiser.setBounds (filterArea.removeFromTop(dEqHeight));

    alOverlayError.setBounds (directivityEqualiser.getX() + 120, directivityEqualiser.getY() + 50, directivityEqualiser.getWidth() - 240, directivityEqualiser.getHeight() - 100);
    alOverlayDisturber.setBounds (directivityEqualiser.getX() + 120, directivityEqualiser.getY() + 50, directivityEqualiser.getWidth() - 240, directivityEqualiser.getHeight() - 100);
    alOverlaySignal.setBounds (directivityEqualiser.getX() + 120, directivityEqualiser.getY() + 50, directivityEqualiser.getWidth() - 240, directivityEqualiser.getHeight() - 100);

    trimSliderArea.setHeight(directivityEqualiser.getHeight());
    trimSlider.setBounds(trimSliderArea);

#ifdef AA_DO_DEBUG_PATH
    { // !J! for debugging purposes only
        debugPath.startNewSubPath(directivityEqualiser.getX(), directivityEqualiser.getY());
        debugPath.lineTo(directivityEqualiser.getRight(), directivityEqualiser.getBottom());
        debugPath.addRectangle(directivityEqualiser.getX(), directivityEqualiser.getY(), directivityEqualiser.getWidth(), directivityEqualiser.getHeight());
    }
#endif
    
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
    else if (button == &tbAbButton[0])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(0);
            button->setAlpha(getABButtonAlphaFromLayerState(isToggled));
            tbAbButton[1].setAlpha(getABButtonAlphaFromLayerState(!isToggled));
        }
    }
    else if (button == &tbAbButton[1])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(1);
            button->setAlpha(getABButtonAlphaFromLayerState(isToggled));
            tbAbButton[0].setAlpha(getABButtonAlphaFromLayerState(!isToggled));
        }
    }
    else // muteSoloButton!
    {
        directivityEqualiser.setSoloActive(getSoloActive());
        directivityEqualiser.repaint();
        for (auto& vis : polarPatterVisualizers)
        {
            vis.setSoloActive(getSoloActive());
            vis.repaint();
        }
    }
}

float PolarDesignerAudioProcessorEditor::getABButtonAlphaFromLayerState(int layerState)
{
    return layerState * 0.7f + 0.3f;
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
    if (slider == &trimSlider) {
        return;
    }
    else
    if (slider == &slCrossoverPosition[0] || slider == &slCrossoverPosition[1] || slider == &slCrossoverPosition[2] || slider == &slCrossoverPosition[3])
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
                polarPatterVisualizers[i].setDirWeight(slider->getValue());
        }
    }
    directivityEqualiser.repaint();
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
            polarPatterVisualizers[i].setActive(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            msbSolo[i].setEnabled(false);
            msbSolo[i].setToggleState(false, NotificationType::sendNotification);
            msbMute[i].setEnabled(false);
            msbMute[i].setToggleState(false, NotificationType::sendNotification);
            polarPatterVisualizers[i].setActive(false);
        }
    }

    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
        
}

void PolarDesignerAudioProcessorEditor::timerCallback()
{
    if (processor.repaintDEQ.get())
    {
        processor.repaintDEQ = false;
        directivityEqualiser.repaint();
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
    
    nActiveBands = cbSetNrBands.getSelectedId();
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
            polarPatterVisualizers[i].setActive(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            msbSolo[i].setEnabled(false);
            msbSolo[i].setToggleState(false, NotificationType::sendNotification);
            msbMute[i].setEnabled(false);
            msbMute[i].setToggleState(false, NotificationType::sendNotification);
            polarPatterVisualizers[i].setActive(false);
        }
    }
    
    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
}

void PolarDesignerAudioProcessorEditor::disableMainArea()
{
    directivityEqualiser.setActive(false);
    for (int i = 0; i < 5; i++)
    {
        slDir[i].setEnabled(false);
        slBandGain[i].setEnabled(false);
        msbSolo[i].setEnabled(false);
        msbMute[i].setEnabled(false);
        polarPatterVisualizers[i].setActive(false);
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
    directivityEqualiser.setActive(true);
    nActiveBandsChanged();
    setSideAreaEnabled(true);
    tbZeroDelay.setEnabled(true);
}

// implement this for AAX automation shortchut
int PolarDesignerAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &directivityEqualiser.getBandlimitPathComponent(0) && nActiveBands > 1)
        return 0;
    else if (&control == &directivityEqualiser.getBandlimitPathComponent(1) && nActiveBands > 2)
        return 1;
    else if (&control == &directivityEqualiser.getBandlimitPathComponent(2) && nActiveBands > 3)
        return 2;
    else if (&control == &directivityEqualiser.getBandlimitPathComponent(3) && nActiveBands > 4)
        return 3;
    else if (&control == &slDir[0] || &control == &directivityEqualiser.getDirPathComponent(0))
        return 4;
    else if ((&control == &slDir[1] || &control == &directivityEqualiser.getDirPathComponent(1)) && nActiveBands > 1)
        return 5;
    else if ((&control == &slDir[2] || &control == &directivityEqualiser.getDirPathComponent(2)) && nActiveBands > 2)
        return 6;
    else if ((&control == &slDir[3] || &control == &directivityEqualiser.getDirPathComponent(3)) && nActiveBands > 3)
        return 7;
    else if ((&control == &slDir[4] || &control == &directivityEqualiser.getDirPathComponent(4)) && nActiveBands > 4)
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

