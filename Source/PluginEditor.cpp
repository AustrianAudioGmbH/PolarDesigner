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

#ifdef AA_INCLUDE_MELATONIN
#include "melatonin_inspector/melatonin/helpers/timing.h"
#endif


#define TRIMSLIDER_RESET_VALUE (0.0f)


//==============================================================================
PolarDesignerAudioProcessorEditor::PolarDesignerAudioProcessorEditor (PolarDesignerAudioProcessor& p,
                                                          AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), loadingFile(false),
    presetListVisible(false), polarDesignerProcessor (p),
    valueTreeState(vts),
    directivityEqualiser (p),
      uiTerminatorAnimationWindowIsVisible (false),
      uiMaxToSpillWindowIsVisible (false),
      uiTargetAquisitionWindowIsVisible (false),
      uiMaximizeTargetWindowIsVisible (false),
      uiMaxTargetToSpillFlowStarted (false),
    termStage(PolarDesignerAudioProcessorEditor::terminatorStage::DISABLED)
{

    nActiveBands = polarDesignerProcessor.getNProcessorBands();
    syncChannelIdx = polarDesignerProcessor.getSyncChannelIdx();

    setResizable(true, true);
    setResizeLimits(EDITOR_MIN_WIDTH, EDITOR_MIN_HEIGHT, EDITOR_MAX_WIDTH, EDITOR_MAX_HEIGHT);
    float ratio = static_cast<float>(EDITOR_MIN_WIDTH) / static_cast<float>(EDITOR_MIN_HEIGHT);
    getConstrainer()->setFixedAspectRatio(ratio);
    setSize(EDITOR_MIN_WIDTH, EDITOR_MIN_HEIGHT);

    setLookAndFeel(&mainLaF);

    addAndMakeVisible(sharedTooltipWindow);

    addAndMakeVisible(&tbLogoAA);
    tbLogoAA.setButtonText("Austrian Audio Logo");
    tbLogoAA.setTooltip(String(AA_BUILD_TAG) + String("-") + String(AA_BUILD_COMMIT_HASH) + String::formatted(" (JUCE:%s)", AA_BUILD_JUCE_VERSION) + String::formatted(" (%p)", this) );
    Logger::writeToLog("PolarDesigner3 Build:" + String(AA_BUILD_TAG) + String("-") + String(AA_BUILD_COMMIT_HASH) + String::formatted(" (JUCE:%s)", AA_BUILD_JUCE_VERSION));

    addAndMakeVisible(&titleCompare);
    titleCompare.setTitle(String("Compare"));
    titleCompare.setFont(mainLaF.normalFont);

    addAndMakeVisible(&tmbABButton);
    tmbABButton.setButtonsNumber(2);
    tmbABButton.setInterceptsMouseClicks(true, true);

    tmbABButton[0].setClickingTogglesState(true);
    tmbABButton[0].setRadioGroupId(3344);
    tmbABButton[0].setButtonText("A");
    tmbABButton[0].addListener(this);

    tmbABButton[1].setClickingTogglesState(true);
    tmbABButton[1].setRadioGroupId(3344);
    tmbABButton[1].setButtonText("B");
    tmbABButton[1].addListener(this);

    // Initialize A/B button states
    updateABButtonState();

    // Add listener for valueTreeState changes
    valueTreeState.state.addListener(this);

    addAndMakeVisible(&tbZeroLatency);
    tbZeroLatencyAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment(valueTreeState, "zeroLatencyMode", tbZeroLatency));
    tbZeroLatency.addListener(this);
    tbZeroLatency.setButtonText("Zero latency");
    tbZeroLatency.setToggleable(true);
    tbZeroLatency.setClickingTogglesState(true);
    tbZeroLatency.setToggleState(polarDesignerProcessor.zeroLatencyModeActive(), NotificationType::sendNotification);

    addAndMakeVisible(&titlePreset);
    titlePreset.setTitle(String("Preset"));
    titlePreset.setFont(mainLaF.normalFont);
    titlePreset.setInterceptsMouseClicks(false, true);

    addAndMakeVisible(&titlePresetUndoButton);
    titlePresetUndoButton.setButtonText("Preset undo");
    titlePresetUndoButton.addListener(this);

    addAndMakeVisible(&tbLoad);
    tbLoad.addListener(this);
    tbLoad.setButtonText("Load");
    tbLoad.setToggleState(false, NotificationType::dontSendNotification);

    addAndMakeVisible(&tbSave);
    tbSave.addListener(this);
    tbSave.setButtonText("Save");
    tbSave.setToggleState(false, NotificationType::dontSendNotification);

    addAndMakeVisible(&tmbNrBandsButton);
    tmbNrBandsButton.setButtonsNumber(static_cast<int> (MAX_EDITOR_BANDS));
    tmbNrBandsButton.setAlwaysOnTop(true);

    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i)
    {
        tmbNrBandsButton[i].setClickingTogglesState(true);
        tmbNrBandsButton[i].setRadioGroupId(34567);

        tmbNrBandsButton[i].setButtonText(String(i + 1));
        tmbNrBandsButton[i].addListener(this);

        if (i == (nActiveBands - 1)) {
            tmbNrBandsButton[i].setToggleState (true, NotificationType::dontSendNotification);
        }
    }

    addAndMakeVisible (&footer);
    footer.setInterceptsMouseClicks(false, true);

    // groups
    addAndMakeVisible (&grpBands);
    grpBands.setText ("Number of bands");

    addAndMakeVisible(&grpEq);
    grpEq.setText("Equalization control");

    addAndMakeVisible(&ibEqCtr[0]);
    ibEqCtr[0].setButtonText("Free Field");
    ibEqCtr[0].addListener(this);

    addAndMakeVisible(&ibEqCtr[1]);
    ibEqCtr[1].setButtonText("Diffuse Field");
    ibEqCtr[1].addListener(this);

    addAndMakeVisible(&grpProxComp);
    grpProxComp.setText("Proximity control");

    addAndMakeVisible(&tgbProxCtr);
    tgbProxCtrAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment(valueTreeState, "proximityOnOff", tgbProxCtr));
    tgbProxCtr.addListener(this);

    addAndMakeVisible(&slProximity);
    slProximityAtt = std::unique_ptr<SliderAttachment>(new SliderAttachment(valueTreeState, "proximity", slProximity));
    slProximity.setSliderStyle(Slider::LinearHorizontal);
    slProximity.setTextBoxStyle(Slider::TextBoxRight, false, 45, 15);
    slProximity.setEnabled(tgbProxCtr.getToggleState());
    slProximity.addListener(this);

    addAndMakeVisible(&grpTerminatorControl);
    grpTerminatorControl.setText("Terminator control");

    addAndMakeVisible(&tbCloseTerminatorControl);
    tbCloseTerminatorControl.setComponentID("5721");
    tbCloseTerminatorControl.setToggleState(false, NotificationType::dontSendNotification);
    tbCloseTerminatorControl.addListener(this);

    addAndMakeVisible(polarDesignerProcessor.termControlWaveform);
    polarDesignerProcessor.termControlWaveform.setColours(mainLaF.labelBackgroundColor, mainLaF.textButtonActiveRedFrameColor);

    addAndMakeVisible(&albPlaybackSpill);

    addAndMakeVisible(&albAcquiringTarget);

    addAndMakeVisible(&grpSync);
    grpSync.setText("Sync group");

    addAndMakeVisible(&tmbSyncChannelButton);
    tmbSyncChannelButton.setButtonsNumber(4);
    tmbSyncChannelButton.setAlwaysOnTop(true);

    for (uint32 i = 0; i < 4; ++i)
    {
        tmbSyncChannelButton[i].setClickingTogglesState(true);

        tmbSyncChannelButton[i].setButtonText(String(i+1));
        tmbSyncChannelButton[i].addListener(this);
        //Set ID manually for drawing colors purpose
        tmbSyncChannelButton[i].setComponentID(String("552" + String(i + 1)));

        if (i == (syncChannelIdx - 1)) {
            tmbSyncChannelButton[i].setToggleState(true, NotificationType::dontSendNotification);
        }
    }

    eqColours[0] = mainLaF.polarVisualizerRed;
    eqColours[1] = mainLaF.polarVisualizerOrange;
    eqColours[2] = mainLaF.polarVisualizerYellow;
    eqColours[3] = mainLaF.polarVisualizerGreen;
    eqColours[4] = mainLaF.polarVisualizerGreenDark;

    // directivity eq
    addAndMakeVisible (&directivityEqualiser);

    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i)
    {
        // SOLO button
        addAndMakeVisible (&tgbSolo[i]);
        tgbSoloAtt[i] = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "solo" + String(i+1), tgbSolo[i]));
        tgbSolo[i].addListener (this);
        tgbSolo[i].setButtonText("S");
        tgbSolo[i].setAlwaysOnTop (true);

        // MUTE button
        addAndMakeVisible (&tgbMute[i]);
        tgbMuteAtt[i] = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "mute" + String(i+1), tgbMute[i]));
        tgbMute[i].addListener (this);
        tgbMute[i].setButtonText("M");
        tgbMute[i].setAlwaysOnTop (true);

        // Direction slider
        addAndMakeVisible (&slDir[i]);
        slDirAtt[i] = std::unique_ptr<SliderAttachment>(new SliderAttachment (valueTreeState, "alpha" + String(i+1), slDir[i]));
        slDir[i].setColour (Slider::thumbColourId, eqColours[i]); // colour of knob
        slDir[i].addListener (this);
        slDir[i].setTooltipEditable (true);
        slDir[i].setInterceptsMouseClicks(false, true);

        // Band Gain slider
        addAndMakeVisible (&slBandGain[i]);
        slBandGainAtt[i] = std::unique_ptr<SliderAttachment>(new SliderAttachment (valueTreeState, "gain" + String(i+1), slBandGain[i]));
        slBandGain[i].setSliderStyle (Slider::LinearVertical);
        slBandGain[i].setTextValueSuffix(" dB");
        slBandGain[i].setTextBoxStyle(Slider::TextBoxLeft, true, -1, -1);
        slBandGain[i].addListener (this);

        // First-Order directivity visualizer (The "O"verhead view)
        addAndMakeVisible (&polarPatternVisualizers[i]);
        polarPatternVisualizers[i].setActive(true);
        polarPatternVisualizers[i].addListener(this);
        polarPatternVisualizers[i].setDirWeight (static_cast<float>(slDir[i].getValue()));
        polarPatternVisualizers[i].setMuteSoloButtons (&tgbSolo[i], &tgbMute[i]);
        polarPatternVisualizers[i].setColour (eqColours[i]);

        // main directivity Equaliser section
        directivityEqualiser.addSliders (eqColours[i], &slDir[i], (i > 0) ? &slCrossoverPosition[i - 1] : nullptr, (i < (MAX_EDITOR_BANDS - 1)) ? &slCrossoverPosition[i] : nullptr, &tgbSolo[i], &tgbMute[i], &slBandGain[i], &polarPatternVisualizers[i]);

        if (i == MAX_EDITOR_BANDS - 1)
            break; // there is one slCrossoverPosition less than bands

        addAndMakeVisible (&slCrossoverPosition[i]);
        slCrossoverAtt[i] = std::unique_ptr<ReverseSlider::SliderAttachment>(new ReverseSlider::SliderAttachment (valueTreeState, "xOverF" + String(i+1), slCrossoverPosition[i]));
        slCrossoverPosition[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slCrossoverPosition[i].addListener(this);
        slCrossoverPosition[i].setVisible(false);
        slCrossoverPosition[i].setInterceptsMouseClicks(false, true);
    }

    directivityEqualiser.initValueBox();

    addAndMakeVisible (&tbTerminateSpill);
    tbTerminateSpill.setButtonText ("Terminate Spill");
    tbTerminateSpill.addListener (this);

    addAndMakeVisible (&tbMaximizeTarget);
    tbMaximizeTarget.setButtonText ("Maximize Target");
    tbMaximizeTarget.addListener (this);

    addAndMakeVisible(&tbMaxTargetToSpill);
    tbMaxTargetToSpill.setButtonText("Max Target-to-spill");
    tbMaxTargetToSpill.addListener(this);

    addAndMakeVisible(&terminatorLabelNr1);
    terminatorLabelNr1.setButtonText("01");
    terminatorLabelNr1.setToggleState(false, NotificationType::dontSendNotification);
    addAndMakeVisible(&terminatorLabelSpillMain);
    terminatorLabelSpillMain.setButtonText("Terminate spill");
    addAndMakeVisible(&terminatorLabelSpillSub);
    juce::String termSpillText;
    termSpillText << "Click on the button below to apply polar\n";
    termSpillText << "patterns with minimum spill energy";
    terminatorLabelSpillSub.setButtonText(termSpillText);

    addAndMakeVisible(&tbBeginTerminate);
    tbBeginTerminate.setButtonText("Begin Terminate");
    tbBeginTerminate.addListener(this);

    addAndMakeVisible(&terminatorLabelNr2);
    terminatorLabelNr2.setButtonText("02");
    terminatorLabelNr2.setToggleState(false, NotificationType::dontSendNotification);
    addAndMakeVisible(&terminatorLabelMaxMain);
    terminatorLabelMaxMain.setButtonText("Maximize target");
    addAndMakeVisible(&terminatorLabelMaxSub);
    juce::String termMaxText;
    termMaxText << "Click on the button below to apply polar\n";
    termMaxText << "patterns with maximum signal energy";
    terminatorLabelMaxSub.setButtonText(termMaxText);

    addAndMakeVisible(&tbBeginMaximize);
    tbBeginMaximize.setButtonText("Begin Maximize");
    tbBeginMaximize.addListener(this);

    addAndMakeVisible(&terminatorLabelNr3);
    terminatorLabelNr3.setButtonText("03");
    terminatorLabelNr3.setToggleState(false, NotificationType::dontSendNotification);
    addAndMakeVisible(&terminatorLabelMaxToSpillMain);
    terminatorLabelMaxToSpillMain.setButtonText("Max Target-to-Spill");
    addAndMakeVisible(&terminatorLabelMaxToSpillSub);
    juce::String termMaxToSpillText;
    termMaxToSpillText << "Find best compromise between reduction\n";
    termMaxToSpillText << "of spill and maximizing target signal";
    terminatorLabelMaxToSpillSub.setButtonText(termMaxToSpillText);

    addAndMakeVisible(&tbApplyMaxTargetToSpill);
    tbApplyMaxTargetToSpill.setButtonText("Apply Max Target-to-Spill");
    tbApplyMaxTargetToSpill.addListener(this);

    for (int i = 0; i < 8; i++)
    {
        addAndMakeVisible(&terminatorStageLine[i]);
        terminatorStageLine[i].setButtonText("TermLine" + String(i));
    }

    addAndMakeVisible (&tbAllowBackwardsPattern);
    tbAllowBackwardsPatternAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "allowBackwardsPattern", tbAllowBackwardsPattern));
    tbAllowBackwardsPattern.setButtonText ("allow reverse patterns");
    tbAllowBackwardsPattern.addListener (this);

    directivityEqualiser.setSoloActive(getSoloActive());
    directivityEqualiser.setInterceptsMouseClicks(false, true);

    for (auto& vis : polarPatternVisualizers)
    {
        vis.setSoloActive (getSoloActive());
    }

    addAndMakeVisible(&grpPresetList);
    grpPresetList.setText("Preset");

    addAndMakeVisible(&tbClosePresetList);
    tbClosePresetList.addListener(this);
    tbClosePresetList.setComponentID("5621");
    tbClosePresetList.setToggleState(false, NotificationType::dontSendNotification);

    addAndMakeVisible(&tbOpenFromFile);
    tbOpenFromFile.setClickingTogglesState(true);
    tbOpenFromFile.setButtonText("Open from file");
    tbOpenFromFile.addListener(this);

    addAndMakeVisible(&lbUserPresets);
    lbUserPresets.setHeaderText("User Presets");
    lbUserPresets.addChangeListener(this);

    addAndMakeVisible(&lbFactoryPresets);
    lbFactoryPresets.setHeaderText("Factory Presets");
    lbFactoryPresets.addChangeListener(this);

    nEditorBandsChanged();
    activateEditingForZeroLatency();

    trimSlider.sliderValueSet = [this] { setTrimValue((int) nActiveBands); };
    trimSlider.sliderReset = [this] { resetTrim((int) nActiveBands); };

    addAndMakeVisible(&trimSlider);
    trimSlider.addListener(this);

    addAndMakeVisible(&tbTrimSliderCenterPointer);
    tbTrimSliderCenterPointer.setButtonText("Trim Slider Pointer");

    loadSavedPresetsToList();

    initializeSavedStates();

    // Save initial state for the current layer
    uint32 currentLayer = polarDesignerProcessor.abLayerState;
    saveLayerState(currentLayer);

    // Prime the UI
    nEditorBandsChanged();
    activateEditingForZeroLatency();

    startTimer (60);

    setEqMode();
}


void PolarDesignerAudioProcessorEditor::initializeSavedStates() {
    nActiveBands = polarDesignerProcessor.getNProcessorBands();
    isStateSaved.fill(false);
    for (unsigned long layer = 0; layer < 2; ++layer) {
        savedStates[layer].nrBandsValue = nActiveBands;
        savedStates[layer].dirValues.fill(0.0f);
        savedStates[layer].bandGainValues.fill(0.0f);
        savedStates[layer].soloStates.fill(false);
        savedStates[layer].muteStates.fill(false);
        savedStates[layer].crossoverValues.fill(1000.0f); // Safe default
        savedStates[layer].proxCtrState = false;
        savedStates[layer].proxValue = 0.0f;
        savedStates[layer].eqState = polarDesignerProcessor.getEqState();
    }
    DBG("Initialized savedStates: nrBands=" << std::to_string(nActiveBands));
}

// Handle the trimSlider increment/decrement calls
void PolarDesignerAudioProcessorEditor::setTrimValue(int nBands) {
    //Check if slider incrementing/decrementing
    bool lockBandsOnTop = false;
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].getValue() == 1.f)
        {
            lockBandsOnTop = true;
            break;
        }
        else if (exactlyEqual(slDir[i].getValue(), -0.5))
        {
            lockBandsOnTop = false;
            break;
        }
    }
    calculateLockedBands(nBands, lockBandsOnTop);

    float currPos = trimSlider.getCurrentSliderValue() - trimSliderPrevPos;
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].isEnabled() && !bandLockedOnMinMax[i])
        {
            slDir[i].setValue(slDir[i].getValue() + currPos);
        }
    }
    trimSliderPrevPos = trimSlider.getCurrentSliderValue();

    directivityEqualiser.setDirSliderLastChangedByDrag(false);
    directivityEqualiser.resetTooltipTexts();
}


void PolarDesignerAudioProcessorEditor::resetTrim(int nBands) {
    trimSliderPrevPos = 0.22f;
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].isEnabled())
            slDir[i].setValue(TRIMSLIDER_RESET_VALUE);
    }
    directivityEqualiser.resetTooltipTexts();
}

// Update A/B button states
void PolarDesignerAudioProcessorEditor::updateABButtonState()
{
//    ScopedLock lock(polarDesignerProcessor.abLayerLock); // Ensure thread-safety
    bool isLayerB = (polarDesignerProcessor.abLayerState == COMPARE_LAYER_B);
    tmbABButton[COMPARE_LAYER_A].setToggleState(!isLayerB, NotificationType::dontSendNotification);
    tmbABButton[COMPARE_LAYER_A].setAlpha(getABButtonAlphaFromLayerState(!isLayerB));

    tmbABButton[COMPARE_LAYER_B].setToggleState(isLayerB, NotificationType::dontSendNotification);
    tmbABButton[COMPARE_LAYER_B].setAlpha(getABButtonAlphaFromLayerState(isLayerB));
}


PolarDesignerAudioProcessorEditor::~PolarDesignerAudioProcessorEditor()
{
    DBG("PolarDesignerAudioProcessorEditor destructor called");

    sharedTooltipWindow.setVisible(false);

    valueTreeState.state.removeListener(this);
    directivityEqualiser.clearSliders();

    stopTimer();

    for (uint32 i = 0; i < 2; ++i) tmbABButton[i].removeListener(this);

    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i) {
        tmbNrBandsButton[i].removeListener(this);
        tgbSolo[i].removeListener(this);
        tgbMute[i].removeListener(this);
        slDir[i].removeListener(this);
        slBandGain[i].removeListener(this);
        polarPatternVisualizers[i].removeListener(this);
        if (i < MAX_EDITOR_BANDS - 1) slCrossoverPosition[i].removeListener(this);
    }
    tbLoad.removeListener(this);
    tbSave.removeListener(this);
    tbClosePresetList.removeListener(this);
    tbOpenFromFile.removeListener(this);
    tbCloseTerminatorControl.removeListener(this);
    tbTerminateSpill.removeListener(this);
    tbMaximizeTarget.removeListener(this);
    tbMaxTargetToSpill.removeListener(this);
    tbBeginTerminate.removeListener(this);
    tbBeginMaximize.removeListener(this);
    tbApplyMaxTargetToSpill.removeListener(this);
    tbAllowBackwardsPattern.removeListener(this);
    tgbProxCtr.removeListener(this);
    tbZeroLatency.removeListener(this);
    ibEqCtr[0].removeListener(this);
    ibEqCtr[1].removeListener(this);
    for (uint32 i = 0; i < 4; ++i) tmbSyncChannelButton[i].removeListener(this);
    lbUserPresets.removeChangeListener(this);
    lbFactoryPresets.removeChangeListener(this);
    trimSlider.removeListener(this);
    tbZeroLatencyAtt.reset();
    tgbProxCtrAtt.reset();
    slProximityAtt.reset();
    tbAllowBackwardsPatternAtt.reset();
    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i) {
        tgbSoloAtt[i].reset();
        tgbMuteAtt[i].reset();
        slDirAtt[i].reset();
        slBandGainAtt[i].reset();
        if (i < MAX_EDITOR_BANDS - 1) slCrossoverAtt[i].reset();
    }
    setLookAndFeel(nullptr);
}

//==============================================================================
void PolarDesignerAudioProcessorEditor::paint (Graphics& g)
{
#ifdef AA_INCLUDE_MELATONIN
    melatonin::ComponentTimer timer { this };
#endif

    g.fillAll (mainLaF.mainBackground);
#ifdef AA_DO_DEBUG_PATH
    g.strokePath (debugPath, PathStrokeType (15.0f));
#endif
}

void PolarDesignerAudioProcessorEditor::resized()
{
    Rectangle<int> area(getLocalBounds());

    juce::FlexBox mainfb;
    mainfb.flexDirection = FlexBox::Direction::row;
    mainfb.justifyContent = juce::FlexBox::JustifyContent::center;
    mainfb.alignContent = juce::FlexBox::AlignContent::center;

    juce::FlexBox subfb;
    subfb.flexDirection = FlexBox::Direction::column;
    subfb.justifyContent = juce::FlexBox::JustifyContent::center;
    subfb.alignContent = juce::FlexBox::AlignContent::center;

    juce::FlexBox topComponent;
    topComponent.flexDirection = FlexBox::Direction::row;
    topComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    topComponent.alignContent = juce::FlexBox::AlignContent::center;

//    const float marginFlex = 0.022f;
//    const float topComponentTitleFlex = 0.4f;
//    const float topComponentButtonsFlex = 0.035f;
//    const float topComponentSpacingFlex = topComponentButtonsFlex / 2;
//    const float topComponentButtonsMargin = 5;
//    const float radioButonsFlex = 0.18f;
//    const float radioButonsSpaceFlex = 0.025f;

    topComponent.items.add(juce::FlexItem().withFlex(0.022f));
    if (!presetListVisible)
    {
        topComponent.items.add(juce::FlexItem(tbLogoAA).withFlex(0.243f));
    }

    topComponent.items.add(juce::FlexItem(titleCompare).withFlex(0.063f));
    topComponent.items.add(juce::FlexItem().withFlex(0.016f));
    topComponent.items.add(juce::FlexItem(tmbABButton).withFlex(0.077f).withMargin(2));
    topComponent.items.add(juce::FlexItem().withFlex(0.042f));
    topComponent.items.add(juce::FlexItem(tbZeroLatency).withFlex(0.1f).withMargin(2));
    topComponent.items.add(juce::FlexItem().withFlex(0.1f));
    topComponent.items.add(juce::FlexItem(titlePreset).withFlex(0.11f));
    if (presetLoaded)
    {
        topComponent.items.add(juce::FlexItem().withFlex(0.01f));
        topComponent.items.add(juce::FlexItem(titlePresetUndoButton).withFlex(0.01f));
    }
    topComponent.items.add(juce::FlexItem().withFlex(0.02f));
    topComponent.items.add(juce::FlexItem(tbLoad).withFlex(0.072f).withMargin(2));
    topComponent.items.add(juce::FlexItem().withFlex(0.01f));
    topComponent.items.add(juce::FlexItem(tbSave).withFlex(0.072f).withMargin(2));
    topComponent.items.add(juce::FlexItem().withFlex(0.034f));

    juce::FlexBox bandNumbersComponent;
    bandNumbersComponent.flexDirection = FlexBox::Direction::row;
    bandNumbersComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    bandNumbersComponent.alignContent = juce::FlexBox::AlignContent::center;

    juce::FlexBox syncChannelComponent;
    syncChannelComponent.flexDirection = FlexBox::Direction::row;
    syncChannelComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    syncChannelComponent.alignContent = juce::FlexBox::AlignContent::center;

    juce::FlexBox sideComponent;
    sideComponent.flexDirection = FlexBox::Direction::column;
    sideComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    sideComponent.alignContent = juce::FlexBox::AlignContent::center;
    sideComponent.items.add(juce::FlexItem(/*placeholder for grpBands*/).withFlex(0.14f));
    sideComponent.items.add(juce::FlexItem().withFlex(0.02f));
    sideComponent.items.add(juce::FlexItem(/*placeholder for grpEq*/).withFlex(0.22f));
    sideComponent.items.add(juce::FlexItem().withFlex(0.02f));
    sideComponent.items.add(juce::FlexItem(/*placeholder for grpProxComp*/).withFlex(0.14f));
    sideComponent.items.add(juce::FlexItem().withFlex(0.02f));
    sideComponent.items.add(juce::FlexItem(/*placeholder for grpTermControl*/).withFlex(0.28f));
    sideComponent.items.add(juce::FlexItem().withFlex(0.02f));
    sideComponent.items.add(juce::FlexItem(/*placeholder for grpSync*/).withFlex(0.14f));

    // Margins are fixed value because DirectivityEQ component has fixed margins
    const float polarVisualizersComponentLeftMargin = directivityEqualiser.proportionOfHeight(0.13f) * 1.0f;
    const float polarVisualizersComponentRightMargin = proportionOfHeight(0.057f) * 1.0f;

    juce::FlexBox polarVisualizersComponent;
    polarVisualizersComponent.flexDirection = FlexBox::Direction::row;
    polarVisualizersComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    polarVisualizersComponent.alignContent = juce::FlexBox::AlignContent::center;
    polarVisualizersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentLeftMargin));

    juce::FlexBox muteSoloModule;
    muteSoloModule.flexDirection = FlexBox::Direction::row;
    muteSoloModule.justifyContent = juce::FlexBox::JustifyContent::center;
    muteSoloModule.alignContent = juce::FlexBox::AlignContent::center;
    muteSoloModule.items.add(juce::FlexItem().withWidth(polarVisualizersComponentLeftMargin));

    juce::FlexBox muteSoloComponent[5];
    auto muteSoloButtonSize = area.getHeight() * 0.04f;
    auto muteSoloButtonSpace = area.getHeight() * 0.025f;
    for (int i = 0; i < 5; i++)
    {
        muteSoloComponent[i].flexDirection = FlexBox::Direction::row;
        muteSoloComponent[i].justifyContent = juce::FlexBox::JustifyContent::center;
        muteSoloComponent[i].alignContent = juce::FlexBox::AlignContent::center;
        muteSoloComponent[i].items.add(juce::FlexItem().withFlex(0.4f));
        muteSoloComponent[i].items.add(juce::FlexItem(tgbSolo[i]).withMinWidth(muteSoloButtonSize).withHeight(muteSoloButtonSize));
        muteSoloComponent[i].items.add(juce::FlexItem().withWidth(muteSoloButtonSpace));
        muteSoloComponent[i].items.add(juce::FlexItem(tgbMute[i]).withMinWidth(muteSoloButtonSize).withHeight(muteSoloButtonSize));
        muteSoloComponent[i].items.add(juce::FlexItem().withFlex(0.4f));
    }

    juce::FlexBox dirSlidersComponent;
    dirSlidersComponent.flexDirection = FlexBox::Direction::row;
    dirSlidersComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    dirSlidersComponent.alignContent = juce::FlexBox::AlignContent::center;
    dirSlidersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentLeftMargin));

    juce::FlexBox gainBandSlidersComponent;
    gainBandSlidersComponent.flexDirection = FlexBox::Direction::row;
    gainBandSlidersComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    gainBandSlidersComponent.alignContent = juce::FlexBox::AlignContent::center;

    juce::FlexBox gainBandSlidersComponentWrapper;
    gainBandSlidersComponentWrapper.flexDirection = FlexBox::Direction::row;
    gainBandSlidersComponentWrapper.justifyContent = juce::FlexBox::JustifyContent::center;
    gainBandSlidersComponentWrapper.alignContent = juce::FlexBox::AlignContent::center;
    gainBandSlidersComponentWrapper.items.add(juce::FlexItem().withWidth(polarVisualizersComponentLeftMargin));

    //Dynamic layout for polarVisualizers and dirSlider components
    //offsetPolVis are fixed values because DirectivityEQ component has fixed margins
    const float offsetPolVis = 40;

    const float dirEqSize = directivityEqualiser.getEqWidth() * 1.0f;
    auto bandLimitWidth = getBandLimitWidthVector(dirEqSize, offsetPolVis);

    //bandFlex - value used for components spacing across given area i.e 0.65 (maximum 1.0 means full space)
    float bandFlex = 0;
    float prevPolarVisFlex = 0;
    float polarVisFlexSum = 0;
    float prevGainSliderFlex = 0;
    float gainSliderFlexSum = 0;
    int polarRightMarginDiff = 31;
    int gainRightMarginDiff = 15;

    if (nActiveBands < 2)
    {
        if (polarPatternVisualizers[0].isPvisActive())
        {
            bandFlex = bandLimitWidth[0] / dirEqSize;
            dirSlidersComponent.items.add(juce::FlexItem(slDir[0]).withFlex(bandFlex));
            muteSoloModule.items.add(juce::FlexItem(muteSoloComponent[0]).withFlex(bandFlex));

            //Calculate polar visualizer position and size
            auto polarVisHalfWidth = (dirEqSize * 0.135f) / 2;
            float polarVisHalfWidthFlex = polarVisHalfWidth / (dirEqSize + polarRightMarginDiff);
            float bandFlexRelToPolarVisComp = bandLimitWidth[0] / (dirEqSize + polarRightMarginDiff);
            float polarVisFlex = bandFlexRelToPolarVisComp / 2 + polarVisHalfWidthFlex;
            polarVisualizersComponent.items.add(juce::FlexItem(polarPatternVisualizers[0]).withFlex(polarVisFlex));
            polarVisualizersComponent.items.add(juce::FlexItem().withFlex(1.f - polarVisFlex));

            //Calculate gain slider position and size
            int pixe1lLine = 1;
            auto gainSliderHalfWidth = pixe1lLine + getLookAndFeel().getSliderLayout(slBandGain[0]).sliderBounds.getWidth() / 2;
            float gainSliderHalfWidthFlex = gainSliderHalfWidth / dirEqSize;
            float gainSliderFlex = bandFlex / 2 + gainSliderHalfWidthFlex;
            gainBandSlidersComponent.items.add(juce::FlexItem(slBandGain[0]).withFlex(gainSliderFlex));
            gainBandSlidersComponent.items.add(juce::FlexItem().withFlex(1.f - gainSliderFlex));
        }
    }
    else
    {
        for (uint32 i = 0; i < nActiveBands; i++)
        {
            if (polarPatternVisualizers[i].isPvisActive())
            {
                bandFlex = bandLimitWidth[i+1] / dirEqSize;
                dirSlidersComponent.items.add(juce::FlexItem(slDir[i]).withFlex(bandFlex));
                muteSoloModule.items.add(juce::FlexItem(muteSoloComponent[i]).withFlex(bandFlex));

                //Calculate polar visualizers position and size
                auto polarVisHalfWidth = (dirEqSize * 0.135f) / 2;
                if (nActiveBands >= 4)
                    polarVisHalfWidth = (dirEqSize * 0.115f) / 2;
                float polarVisHalfWidthFlex = polarVisHalfWidth / (dirEqSize + polarRightMarginDiff);
                float bandFlexRelToPolarVisComp = bandLimitWidth[i + 1] / (dirEqSize + polarRightMarginDiff);
                float polarVisFlex = bandFlexRelToPolarVisComp / 2 + prevPolarVisFlex / 2;
                //Add polarVisHalfWidthFlex only on first iteration
                if (i == 0)
                    polarVisFlex = bandFlexRelToPolarVisComp / 2 + prevPolarVisFlex / 2 + polarVisHalfWidthFlex;
                polarVisualizersComponent.items.add(juce::FlexItem(polarPatternVisualizers[i]).withFlex(polarVisFlex));
                //Helpers to calculate position of next polar visualizer
                polarVisFlexSum += polarVisFlex;
                prevPolarVisFlex = bandFlexRelToPolarVisComp;
                //Calculate of the remaining space
                if (i == nActiveBands - 1)
                    polarVisualizersComponent.items.add(juce::FlexItem().withFlex(1 - polarVisFlexSum));

                //Gain sliders position calculate to fit textbox when bandwidth is narrow
                int pixe1lLine = 1;
                auto gainSliderHalfWidth = getLookAndFeel().getSliderLayout(slBandGain[i]).sliderBounds.getWidth() / 2;
                float gainSliderHalfWidthFlex = gainSliderHalfWidth / dirEqSize;
                float gainSliderFlex = bandFlex / 2 + prevGainSliderFlex / 2;
                //Add gainSliderHalfWidthFlex only on first iteration
                if (i == 0)
                    gainSliderFlex = bandFlex / 2 + prevGainSliderFlex / 2 + pixe1lLine / dirEqSize + gainSliderHalfWidthFlex;
                gainBandSlidersComponent.items.add(juce::FlexItem(slBandGain[i]).withFlex(gainSliderFlex));
                //Helpers to calculate position of next gain slider
                gainSliderFlexSum += gainSliderFlex;
                prevGainSliderFlex = bandFlex;
                //Calculate of the remaining space
                if (i == nActiveBands - 1)
                    gainBandSlidersComponent.items.add(juce::FlexItem().withFlex(1 - gainSliderFlexSum));
            }
        }
    }
    int gainRightMargin = tbTrimSliderCenterPointer.getWidth() + trimSlider.getWidth() + gainRightMarginDiff;
    polarVisualizersComponent.items.add(juce::FlexItem().withWidth((gainRightMargin - polarRightMarginDiff) * 1.0f));
    dirSlidersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentRightMargin));
    muteSoloModule.items.add(juce::FlexItem().withWidth(gainRightMargin * 1.0f));

    juce::FlexBox tbTrimSliderComponent;
    tbTrimSliderComponent.flexDirection = FlexBox::Direction::column;
    tbTrimSliderComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    tbTrimSliderComponent.alignContent = juce::FlexBox::AlignContent::center;
    tbTrimSliderComponent.items.add(juce::FlexItem().withFlex(0.09f));
    tbTrimSliderComponent.items.add(juce::FlexItem(trimSlider).withFlex(0.85f));
    tbTrimSliderComponent.items.add(juce::FlexItem().withFlex(0.06f));

    juce::FlexBox tbTrimSliderCenterPointerComponent;
    tbTrimSliderCenterPointerComponent.flexDirection = FlexBox::Direction::column;
    tbTrimSliderCenterPointerComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    tbTrimSliderCenterPointerComponent.alignContent = juce::FlexBox::AlignContent::center;
    tbTrimSliderCenterPointerComponent.items.add(juce::FlexItem().withFlex(0.11f));
    tbTrimSliderCenterPointerComponent.items.add(juce::FlexItem(tbTrimSliderCenterPointer).withFlex(0.85f));
    tbTrimSliderCenterPointerComponent.items.add(juce::FlexItem().withFlex(0.06f));

    juce::FlexBox directivityEqualiserComponent;
    directivityEqualiserComponent.flexDirection = FlexBox::Direction::row;
    directivityEqualiserComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    directivityEqualiserComponent.alignContent = juce::FlexBox::AlignContent::center;
    directivityEqualiserComponent.items.add(juce::FlexItem(directivityEqualiser).withFlex(0.94f));
    directivityEqualiserComponent.items.add(juce::FlexItem(tbTrimSliderCenterPointerComponent).withFlex(0.02f));
    directivityEqualiserComponent.items.add(juce::FlexItem(tbTrimSliderComponent).withFlex(0.04f));

    gainBandSlidersComponentWrapper.items.add(juce::FlexItem(gainBandSlidersComponent).withFlex(1.f));
    gainBandSlidersComponentWrapper.items.add(juce::FlexItem().withWidth(gainRightMargin * 1.0f));

    juce::FlexBox middleComponent;
    middleComponent.flexDirection = FlexBox::Direction::column;
    middleComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    middleComponent.alignContent = juce::FlexBox::AlignContent::center;

    // With bands 4 and 5, polar visualizers need to be smaller to fit in narrow bands
    if (nActiveBands >= 4)
    {
        middleComponent.items.add(juce::FlexItem().withFlex(0.045f));
        middleComponent.items.add(juce::FlexItem(polarVisualizersComponent).withFlex(0.15f));
        middleComponent.items.add(juce::FlexItem().withFlex(0.045f));
        middleComponent.items.add(juce::FlexItem(directivityEqualiserComponent).withFlex(0.56f));
    }
    else
    {
        middleComponent.items.add(juce::FlexItem().withFlex(0.03f));
        middleComponent.items.add(juce::FlexItem(polarVisualizersComponent).withFlex(0.18f));
        middleComponent.items.add(juce::FlexItem().withFlex(0.03f));
        middleComponent.items.add(juce::FlexItem(directivityEqualiserComponent).withFlex(0.56f));
    }

    middleComponent.items.add(juce::FlexItem().withFlex(0.04f));
    middleComponent.items.add(juce::FlexItem(muteSoloModule).withFlex(0.07f));
    middleComponent.items.add(juce::FlexItem(gainBandSlidersComponentWrapper).withFlex(0.25f));
    middleComponent.items.add(juce::FlexItem().withFlex(0.03f));

    juce::FlexBox mainComponent;
    mainComponent.flexDirection = FlexBox::Direction::row;
    mainComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    mainComponent.alignContent = juce::FlexBox::AlignContent::center;
    mainComponent.items.add(juce::FlexItem().withFlex(0.021f));
    if (!presetListVisible)
    {
        mainComponent.items.add(juce::FlexItem(sideComponent).withFlex(0.21f));
        mainComponent.items.add(juce::FlexItem().withFlex(0.027f));
    }
    mainComponent.items.add(juce::FlexItem(middleComponent).withFlex(0.86f));
    mainComponent.items.add(juce::FlexItem().withFlex(0.027f));

    subfb.items.add(juce::FlexItem().withFlex(0.03f));
    subfb.items.add(juce::FlexItem(topComponent).withFlex(0.046f));
    subfb.items.add(juce::FlexItem().withFlex(0.05f));
    subfb.items.add(juce::FlexItem(mainComponent).withFlex(0.84f));
    subfb.items.add(juce::FlexItem(footer).withFlex(0.03f));

    if (!presetListVisible)
    {
        mainfb.items.add(juce::FlexItem(subfb).withFlex(1.f));
    }
    else
    {
        mainfb.items.add(juce::FlexItem(subfb).withFlex(0.76f));
        mainfb.items.add(juce::FlexItem(/*placeholder for preset list */).withFlex(0.24f));
    }

    mainfb.performLayout(area);

    // Number of bands Group
    juce::FlexBox fbNrBandsOutComp;
    fbNrBandsOutComp.items.add(juce::FlexItem{ grpBands }.withFlex(1.0f));
    fbNrBandsOutComp.performLayout(sideComponent.items[0].currentBounds);

    juce::FlexBox fbNrBandsInComp;
    fbNrBandsInComp.flexDirection = juce::FlexBox::Direction::column;
    fbNrBandsInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbNrBandsInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbNrBandsInComp.items.add(juce::FlexItem{  }.withFlex(0.45f));
    fbNrBandsInComp.items.add(juce::FlexItem{ tmbNrBandsButton }.withFlex(0.4f));
    fbNrBandsInComp.items.add(juce::FlexItem{  }.withFlex(0.15f));

    auto outerBounds = fbNrBandsOutComp.items[0].currentBounds;
    auto inCompWidth = outerBounds.getWidth();
    fbNrBandsInComp.performLayout(outerBounds.reduced(inCompWidth * 0.06f, 0));

    // Equalization control Group
    juce::FlexBox fbEqCtrOutComp;
    fbEqCtrOutComp.items.add(juce::FlexItem{ grpEq }.withFlex(1.0f));
    fbEqCtrOutComp.performLayout(sideComponent.items[2].currentBounds);

    juce::FlexBox fbFields;
    fbFields.flexDirection = juce::FlexBox::Direction::row;
    fbFields.justifyContent = juce::FlexBox::JustifyContent::center;
    fbFields.alignContent = juce::FlexBox::AlignContent::center;
    fbFields.items.add(juce::FlexItem{ }.withFlex(0.08f));
    fbFields.items.add(juce::FlexItem{ ibEqCtr[0] }.withFlex(0.4f));
    fbFields.items.add(juce::FlexItem{ }.withFlex(0.04f));
    fbFields.items.add(juce::FlexItem{ ibEqCtr[1] }.withFlex(0.4f));
    fbFields.items.add(juce::FlexItem{ }.withFlex(0.08f));

    juce::FlexBox fbEqCtrInComp;
    fbEqCtrInComp.flexDirection = juce::FlexBox::Direction::column;
    fbEqCtrInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbEqCtrInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbEqCtrInComp.items.add(juce::FlexItem{  }.withFlex(0.3f));
    fbEqCtrInComp.items.add(juce::FlexItem{ fbFields }.withFlex(0.58f));
    fbEqCtrInComp.items.add(juce::FlexItem{  }.withFlex(0.12f));

    outerBounds = fbEqCtrOutComp.items[0].currentBounds;
    inCompWidth = outerBounds.getWidth();
    fbEqCtrInComp.performLayout(outerBounds);

    // Proximity control Group
    juce::FlexBox fbProximityControlOutComp;
    fbProximityControlOutComp.items.add(juce::FlexItem{ grpProxComp }.withFlex(1.0f));
    fbProximityControlOutComp.performLayout(sideComponent.items[4].currentBounds);

    juce::FlexBox fbProximityControlToggleButton;
    fbProximityControlToggleButton.flexDirection = juce::FlexBox::Direction::row;
    fbProximityControlToggleButton.justifyContent = juce::FlexBox::JustifyContent::center;
    fbProximityControlToggleButton.alignContent = juce::FlexBox::AlignContent::center;
    fbProximityControlToggleButton.items.add(juce::FlexItem{  }.withFlex(0.75f));
    fbProximityControlToggleButton.items.add(juce::FlexItem{ tgbProxCtr }.withFlex(0.25f));

    juce::FlexBox fbProximityControlInComp;
    fbProximityControlInComp.flexDirection = juce::FlexBox::Direction::column;
    fbProximityControlInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbProximityControlInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbProximityControlInComp.items.add(juce::FlexItem{ fbProximityControlToggleButton }.withFlex(0.5f));
    fbProximityControlInComp.items.add(juce::FlexItem{ slProximity }.withFlex(0.45f));
    fbProximityControlInComp.items.add(juce::FlexItem{  }.withFlex(0.05f));

    outerBounds = fbProximityControlOutComp.items[0].currentBounds;
    inCompWidth = outerBounds.getWidth();
    fbProximityControlInComp.performLayout(outerBounds);

    // Terminator control Group
    juce::FlexBox fbTerminatorControlOutComp;
    fbTerminatorControlOutComp.items.add(juce::FlexItem{ grpTerminatorControl }.withFlex(1.0f));
    fbTerminatorControlOutComp.performLayout(sideComponent.items[6].currentBounds);

    juce::FlexBox fbTerminatorControlInComp;
    fbTerminatorControlInComp.flexDirection = juce::FlexBox::Direction::column;
    fbTerminatorControlInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbTerminatorControlInComp.alignContent = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbTerminatorControlCloseComp;
    fbTerminatorControlCloseComp.flexDirection = juce::FlexBox::Direction::row;
    fbTerminatorControlCloseComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbTerminatorControlCloseComp.alignContent = juce::FlexBox::AlignContent::center;
    fbTerminatorControlCloseComp.items.add(juce::FlexItem{  }.withFlex(0.88f));
    fbTerminatorControlCloseComp.items.add(juce::FlexItem{ tbCloseTerminatorControl }.withFlex(0.12f));

    //Terminator control max-to-spill flow sub flexboxes
    juce::FlexBox fbTermLbSpill;
    fbTermLbSpill.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbSpill.items.add(juce::FlexItem{ terminatorLabelNr1 }.withFlex(0.08f));
    fbTermLbSpill.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbSpill.items.add(juce::FlexItem{ terminatorLabelSpillMain }.withFlex(0.88f));

    juce::FlexBox fbTermLbSpillDesc;
    fbTermLbSpillDesc.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbSpillDesc.items.add(juce::FlexItem{ terminatorStageLine[0] }.withFlex(0.08f));
    fbTermLbSpillDesc.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbSpillDesc.items.add(juce::FlexItem{ terminatorLabelSpillSub }.withFlex(0.88f));

    juce::FlexBox fbTermLbSpillBegin;
    fbTermLbSpillBegin.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbSpillBegin.items.add(juce::FlexItem{ terminatorStageLine[1] }.withFlex(0.08f));
    fbTermLbSpillBegin.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbSpillBegin.items.add(juce::FlexItem{ tbBeginTerminate }.withFlex(0.88f));

    juce::FlexBox fbTermLbMax;
    fbTermLbMax.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbMax.items.add(juce::FlexItem{ terminatorLabelNr2 }.withFlex(0.08f));
    fbTermLbMax.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbMax.items.add(juce::FlexItem{ terminatorLabelMaxMain }.withFlex(0.88f));

    juce::FlexBox fbTermLbMaxDesc;
    fbTermLbMaxDesc.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbMaxDesc.items.add(juce::FlexItem{ terminatorStageLine[2] }.withFlex(0.08f));
    fbTermLbMaxDesc.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbMaxDesc.items.add(juce::FlexItem{ terminatorLabelMaxSub }.withFlex(0.88f));

    juce::FlexBox fbTermLbMaxBegin;
    fbTermLbMaxBegin.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbMaxBegin.items.add(juce::FlexItem{ terminatorStageLine[3] }.withFlex(0.08f));
    fbTermLbMaxBegin.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbMaxBegin.items.add(juce::FlexItem{ tbBeginMaximize }.withFlex(0.88f));

    juce::FlexBox fbTermLbMaxToSpill;
    fbTermLbMaxToSpill.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbMaxToSpill.items.add(juce::FlexItem{ terminatorLabelNr3 }.withFlex(0.08f));
    fbTermLbMaxToSpill.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbMaxToSpill.items.add(juce::FlexItem{ terminatorLabelMaxToSpillMain }.withFlex(0.88f));

    juce::FlexBox fbTermLbMaxToSpillDesc;
    fbTermLbMaxToSpillDesc.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbMaxToSpillDesc.items.add(juce::FlexItem{  }.withFlex(0.08f));
    fbTermLbMaxToSpillDesc.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbMaxToSpillDesc.items.add(juce::FlexItem{ terminatorLabelMaxToSpillSub }.withFlex(0.88f));

    juce::FlexBox fbTermLbMaxToSpillApply;
    fbTermLbMaxToSpillApply.flexDirection = juce::FlexBox::Direction::row;
    fbTermLbMaxToSpillApply.items.add(juce::FlexItem{  }.withFlex(0.08f));
    fbTermLbMaxToSpillApply.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbTermLbMaxToSpillApply.items.add(juce::FlexItem{ tbApplyMaxTargetToSpill }.withFlex(0.88f));

    juce::FlexBox fbEmptyLine1;
    fbEmptyLine1.flexDirection = juce::FlexBox::Direction::row;
    fbEmptyLine1.items.add(juce::FlexItem{ terminatorStageLine[4] }.withFlex(0.08f));
    fbEmptyLine1.items.add(juce::FlexItem{  }.withFlex(0.92f));

    juce::FlexBox fbEmptyLine2;
    fbEmptyLine2.flexDirection = juce::FlexBox::Direction::row;
    fbEmptyLine2.items.add(juce::FlexItem{ terminatorStageLine[5] }.withFlex(0.08f));
    fbEmptyLine2.items.add(juce::FlexItem{  }.withFlex(0.92f));

    juce::FlexBox fbEmptyLine3;
    fbEmptyLine3.flexDirection = juce::FlexBox::Direction::row;
    fbEmptyLine3.items.add(juce::FlexItem{ terminatorStageLine[6] }.withFlex(0.08f));
    fbEmptyLine3.items.add(juce::FlexItem{  }.withFlex(0.92f));

    juce::FlexBox fbEmptyLine4;
    fbEmptyLine4.flexDirection = juce::FlexBox::Direction::row;
    fbEmptyLine4.items.add(juce::FlexItem{ terminatorStageLine[7] }.withFlex(0.08f));
    fbEmptyLine4.items.add(juce::FlexItem{  }.withFlex(0.92f));

    if (uiTerminatorAnimationWindowIsVisible)
    {
        tbCloseTerminatorControl.setVisible(true);
        tbCloseTerminatorControl.setVisible(true);
        albPlaybackSpill.setVisible(!uiTargetAquisitionWindowIsVisible);
        albAcquiringTarget.setVisible(uiTargetAquisitionWindowIsVisible);
        polarDesignerProcessor.termControlWaveform.setVisible(true);
        tbTerminateSpill.setVisible(false);
        tbMaximizeTarget.setVisible(false);
        tbMaxTargetToSpill.setVisible(false);

        fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withHeight(10));
        fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTerminatorControlCloseComp }.withFlex(0.12f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{ uiTargetAquisitionWindowIsVisible ? albAcquiringTarget : albPlaybackSpill }.withFlex(0.22f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{ polarDesignerProcessor.termControlWaveform }.withFlex(0.46f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
    }
    else
    {
        if (uiMaxToSpillWindowIsVisible)
        {
            tbCloseTerminatorControl.setVisible(true);
            tbTerminateSpill.setVisible(false);
            tbMaximizeTarget.setVisible(false);
            tbMaxTargetToSpill.setVisible(false);
            albPlaybackSpill.setVisible(false);
            albAcquiringTarget.setVisible(false);
            polarDesignerProcessor.termControlWaveform.setVisible(false);

            switch (termStage)
            {
            case terminatorStage::DISABLED:
                break;
            case terminatorStage::TERMINATE:
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withHeight(10));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTerminatorControlCloseComp }.withFlex(0.12f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbSpill }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine1 }.withFlex(0.01f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbSpillDesc }.withFlex(0.14f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine2 }.withFlex(0.01f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbSpillBegin }.withFlex(0.16f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine3 }.withFlex(0.06f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMax }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine4 }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxToSpill }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
                break;
            case terminatorStage::MAXIMIZE:
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withHeight(10));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTerminatorControlCloseComp }.withFlex(0.12f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbSpill }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine1 }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMax }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine2 }.withFlex(0.01f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxDesc }.withFlex(0.14f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine3 }.withFlex(0.01f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxBegin }.withFlex(0.16f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine4 }.withFlex(0.06f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxToSpill }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
                break;
            case terminatorStage::MAXTOSPILL:
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withHeight(10));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTerminatorControlCloseComp }.withFlex(0.12f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbSpill }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine1 }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMax }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbEmptyLine2 }.withFlex(0.02f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxToSpill }.withFlex(0.1f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxToSpillDesc }.withFlex(0.14f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTermLbMaxToSpillApply }.withFlex(0.16f));
                fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
                break;
            default:
                break;
            }
        }
        else
        {
            tbCloseTerminatorControl.setVisible(false);
            albPlaybackSpill.setVisible(false);
            albAcquiringTarget.setVisible(false);
            polarDesignerProcessor.termControlWaveform.setVisible(false);
            tbTerminateSpill.setVisible(true);
            tbMaximizeTarget.setVisible(true);
            tbMaxTargetToSpill.setVisible(true);

            fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.25f));
            fbTerminatorControlInComp.items.add(juce::FlexItem{ tbTerminateSpill }.withFlex(0.22f));
            fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
            fbTerminatorControlInComp.items.add(juce::FlexItem{ tbMaximizeTarget }.withFlex(0.22f));
            fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
            fbTerminatorControlInComp.items.add(juce::FlexItem{ tbMaxTargetToSpill }.withFlex(0.22f));
            fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
        }
    }

    outerBounds = fbTerminatorControlOutComp.items[0].currentBounds;
    inCompWidth = outerBounds.getWidth();
    fbTerminatorControlInComp.performLayout(outerBounds.reduced(inCompWidth * 0.06f, 0));

    // Sync channel Group
    juce::FlexBox fbSyncChannelOutComp;
    fbSyncChannelOutComp.items.add(juce::FlexItem{ grpSync }.withFlex(1.0f));
    fbSyncChannelOutComp.performLayout(sideComponent.items[8].currentBounds);

    juce::FlexBox fbSyncChannelInComp;
    fbSyncChannelInComp.flexDirection = juce::FlexBox::Direction::column;
    fbSyncChannelInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbSyncChannelInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbSyncChannelInComp.items.add(juce::FlexItem{  }.withFlex(0.45f));
    fbSyncChannelInComp.items.add(juce::FlexItem{ tmbSyncChannelButton }.withFlex(0.4f));
    fbSyncChannelInComp.items.add(juce::FlexItem{  }.withFlex(0.15f));

    outerBounds = fbSyncChannelOutComp.items[0].currentBounds;
    inCompWidth = outerBounds.getWidth();
    fbSyncChannelInComp.performLayout(outerBounds.reduced(inCompWidth * 0.06f, 0));

    // Preset List Group
    juce::FlexBox fbPresetListOutComp;
    fbPresetListOutComp.items.add(juce::FlexItem{ grpPresetList }.withFlex(1.0f));
    fbPresetListOutComp.performLayout(mainfb.items[1].currentBounds);

    juce::FlexBox fbPresetListSub1InComp;
    fbPresetListSub1InComp.flexDirection = juce::FlexBox::Direction::row;
    fbPresetListSub1InComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbPresetListSub1InComp.items.add(juce::FlexItem{  }.withFlex(0.9f));
    fbPresetListSub1InComp.items.add(juce::FlexItem{ tbClosePresetList }.withFlex(0.1f));

    juce::FlexBox fbPresetListInComp;
    fbPresetListInComp.flexDirection = juce::FlexBox::Direction::column;
    fbPresetListInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbPresetListInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbPresetListInComp.items.add(juce::FlexItem{ fbPresetListSub1InComp }.withFlex(0.03f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.02f));
    fbPresetListInComp.items.add(juce::FlexItem{ tbOpenFromFile }.withFlex(0.04f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
    fbPresetListInComp.items.add(juce::FlexItem{ lbUserPresets }.withFlex(0.41f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
    fbPresetListInComp.items.add(juce::FlexItem{ lbFactoryPresets }.withFlex(0.41f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.03f));

    outerBounds = fbPresetListOutComp.items[0].currentBounds;
    inCompWidth = outerBounds.getWidth();
    fbPresetListInComp.performLayout(outerBounds.reduced(inCompWidth * 0.06f, 0));

    presetArea = mainfb.items[1].currentBounds;
}


void PolarDesignerAudioProcessorEditor::buttonStateChanged (juce::Button* button)
{
    (void)button; // UNUSED
}

void PolarDesignerAudioProcessorEditor::buttonClicked (Button* button)
{
    if (presetListVisible)
    {
        if (lbUserPresets.isRowDoubleClicked() || lbFactoryPresets.isRowDoubleClicked())
        {
            showPresetList(false);
        }
    }
    if ((button == &tmbNrBandsButton[0]) && (button->getToggleState()))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((0)));
    }
    if ((button == &tmbNrBandsButton[1]) && (button->getToggleState()))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((1)));
    }
    if ((button == &tmbNrBandsButton[2]) && (button->getToggleState()))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((2)));
    }
    if ((button == &tmbNrBandsButton[3]) && (button->getToggleState()))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((3)));
    }
    if ((button == &tmbNrBandsButton[4]) && (button->getToggleState()))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((4)));
    }

    if ((button == &tmbSyncChannelButton[0]) && (button->getToggleState()))
    {   // disable tmbABButton group if syncChannels are enabled, as AB is inapplicable in that case
        tmbABButton.setEnabled(false);
        tmbSyncChannelButton.disableAllButtonsExcept (0);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((1)));
    }
    if ((button == &tmbSyncChannelButton[1]) && (button->getToggleState()))
    {
        tmbABButton.setEnabled(false);
        tmbSyncChannelButton.disableAllButtonsExcept (1);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((2)));
    }
    if ((button == &tmbSyncChannelButton[2]) && (button->getToggleState()))
    {
        tmbABButton.setEnabled(false);
        tmbSyncChannelButton.disableAllButtonsExcept (2);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((3)));
    }
    if ((button == &tmbSyncChannelButton[3]) && (button->getToggleState()))
    {
        tmbABButton.setEnabled(false);
        tmbSyncChannelButton.disableAllButtonsExcept (3);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((4)));
    }

    // Re-enable the tmbABButtons if tmbSyncChannelButton is de-selected
    if ( ( (button == &tmbSyncChannelButton[0]) ||
           (button == &tmbSyncChannelButton[1]) ||
           (button == &tmbSyncChannelButton[2]) ||
           (button == &tmbSyncChannelButton[3]) ) && (!button->getToggleState())) {
        tmbABButton.setEnabled(true);
        tmbSyncChannelButton.disableAllButtons();
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(0);
    }

    if (button == &tbLoad)
    {
        if (!presetListVisible)
        {
            showPresetList(!button->getToggleState());
        }
    }
    else if (button == &tbOpenFromFile)
    {
        loadFile();
    }
    else if (button == &tbClosePresetList)
    {
        showPresetList(false);
        polarDesignerProcessor.undoManager.undo();
    }
    else if (button == &tbSave)
    {
        saveFile();
    }
    else if (button == &ibEqCtr[0])
    {
        ibEqCtr[0].setToggleState(!ibEqCtr[0].getToggleState(), NotificationType::dontSendNotification);
        ibEqCtr[1].setToggleState(false, juce::NotificationType::dontSendNotification);

        if (!ibEqCtr[0].getToggleState() && !ibEqCtr[1].getToggleState())
            polarDesignerProcessor.setEqState(0);
        else
            polarDesignerProcessor.setEqState(1);
    }
    else if (button == &ibEqCtr[1])
    {
        ibEqCtr[1].setToggleState(!ibEqCtr[1].getToggleState(), NotificationType::dontSendNotification);
        ibEqCtr[0].setToggleState(false, juce::NotificationType::dontSendNotification);

        if(!ibEqCtr[0].getToggleState() && !ibEqCtr[1].getToggleState())
            polarDesignerProcessor.setEqState(0);
        else
            polarDesignerProcessor.setEqState(2);
    }
    else if (button == &tbTerminateSpill)
    {
        uiTerminatorAnimationWindowIsVisible = true;
        uiMaxToSpillWindowIsVisible = false;
        uiMaximizeTargetWindowIsVisible = false;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        albPlaybackSpill.startAnimation("PLAYBACK SPILL  ");
        resized();
    }
    else if (button == &tbCloseTerminatorControl)
    {
        uiTerminatorAnimationWindowIsVisible = false;
        uiMaxToSpillWindowIsVisible = false;
        uiMaximizeTargetWindowIsVisible = false;
        uiMaxTargetToSpillFlowStarted = false;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        setMainAreaEnabled(true);
        activateMainUI (true);
    }
    else if (button == &tbMaximizeTarget)
    {
        uiTerminatorAnimationWindowIsVisible = true;
        uiMaxToSpillWindowIsVisible = false;
        uiMaximizeTargetWindowIsVisible = true;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        albPlaybackSpill.startAnimation("PLAYBACK SOURCE  ");
        resized();
    }
    else if (button == &tbMaxTargetToSpill)
    {
        uiTerminatorAnimationWindowIsVisible = false;
        uiMaxToSpillWindowIsVisible = true;
        uiMaxTargetToSpillFlowStarted = true;
        termStage = terminatorStage::TERMINATE;
        showActiveTerminatorStage(termStage);
        resized();
    }
    else if (button == &tbBeginTerminate)
    {

        uiTerminatorAnimationWindowIsVisible = true;
        termStage = terminatorStage::MAXIMIZE;
        uiMaxToSpillWindowIsVisible = false;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        albPlaybackSpill.startAnimation("PLAYBACK SPILL  ");
        resized();
    }
    else if (button == &tbBeginMaximize)
    {
        uiMaxToSpillWindowIsVisible = false;
        termStage = terminatorStage::MAXTOSPILL;
        showActiveTerminatorStage(terminatorStage::MAXIMIZE);
        tbMaximizeTarget.triggerClick();
        resized();
    }
    else if (button == &tbApplyMaxTargetToSpill)
    {
        uiTerminatorAnimationWindowIsVisible = false;
        uiMaxToSpillWindowIsVisible = false;
        uiMaxTargetToSpillFlowStarted = false;
        uiTargetAquisitionWindowIsVisible = false;
        activateMainUI (true);
        setMainAreaEnabled(true);
        polarDesignerProcessor.stopTracking(2);
        termStage = terminatorStage::DISABLED;
        showActiveTerminatorStage(termStage);
        resized();
    }
    else if (button == &tbAllowBackwardsPattern)
    {
        return;
    }
    else if (button == &tgbProxCtr)
    {
        slProximity.setEnabled(tgbProxCtr.getToggleState());
    }
    else if (button == &tbZeroLatency)
    {
        bool newState = button->getToggleState();
        uint32 currentLayer = polarDesignerProcessor.abLayerState;

        if (newState && !polarDesignerProcessor.zeroLatencyModeActive()) {
            // Save state before enabling zero latency
            saveLayerState(currentLayer);
        }
        // Update the parameter to reflect the new state
        valueTreeState.getParameter("zeroLatencyMode")->setValueNotifyingHost(newState);
        // Trigger UI update
        activateEditingForZeroLatency();
    }
    else if (button == &tmbABButton[COMPARE_LAYER_A] && button->getToggleState())
    {
//        ScopedLock lock(polarDesignerProcessor.abLayerLock);
        // Save current layer state (B) before switching to A
        if (polarDesignerProcessor.abLayerState == COMPARE_LAYER_B) {
            saveLayerState(COMPARE_LAYER_B);
        }
        polarDesignerProcessor.changeABLayerState(COMPARE_LAYER_A);
        if (!polarDesignerProcessor.zeroLatencyModeActive()) {
            restoreLayerState(COMPARE_LAYER_A);
        }
        updateABButtonState();
    }
    else if (button == &tmbABButton[COMPARE_LAYER_B] && button->getToggleState())
    {
//        ScopedLock lock(polarDesignerProcessor.abLayerLock);
        // Save current layer state (A) before switching to B
        if (polarDesignerProcessor.abLayerState == COMPARE_LAYER_A) {
            saveLayerState(COMPARE_LAYER_A);
        }
        polarDesignerProcessor.changeABLayerState(COMPARE_LAYER_B);
        if (!polarDesignerProcessor.zeroLatencyModeActive()) {
            restoreLayerState(COMPARE_LAYER_B);
        }
        updateABButtonState();
    }
    else if (button == &polarPatternVisualizers[0])
    {
        bool isToggled = button->getToggleState();
        setBandEnabled(0, isToggled);
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
        repaint();
    }
    else if (button == &polarPatternVisualizers[1])
    {
        bool isToggled = button->getToggleState();
        setBandEnabled(1, isToggled);
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
        repaint();
    }
    else if (button == &polarPatternVisualizers[2])
    {
        bool isToggled = button->getToggleState();
        setBandEnabled(2, isToggled);
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
        repaint();
    }
    else if (button == &polarPatternVisualizers[3])
    {
        bool isToggled = button->getToggleState();
        setBandEnabled(3, isToggled);
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
        repaint();
    }
    else if (button == &polarPatternVisualizers[4])
    {
        bool isToggled = button->getToggleState();
        setBandEnabled(4, isToggled);
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
        repaint();
    }
    else if (button == &titlePresetUndoButton)
    {
        polarDesignerProcessor.undoManager.undo();
        titlePresetUndoButton.setVisible(false);
        presetLoaded = false;
        titlePreset.setTitle(String("Preset"));
        resized();
    }
    else // muteSoloButton!
    {
        directivityEqualiser.setSoloActive(getSoloActive());
        directivityEqualiser.repaint();
        for (auto& vis : polarPatternVisualizers)
        {
            vis.setSoloActive(getSoloActive());
            vis.repaint();
        }
    }
    notifyPresetLabelChange();
}

float PolarDesignerAudioProcessorEditor::getABButtonAlphaFromLayerState(int layerState)
{
    return layerState * 0.7f + 0.3f;
}

std::vector<float> PolarDesignerAudioProcessorEditor::getBandLimitWidthVector(float dirEqSize, float offsetPolVis)
{
    (void)offsetPolVis;
    //First calculate bandLimit vector
    std::vector<float> bandLimit;
    bandLimit.push_back(0);
    for (uint32 i = 0; i < (nActiveBands - 1); i++)
    {
        bandLimit.push_back(static_cast<float> (directivityEqualiser.getBandWidth (static_cast<int> (i))));
    }
    bandLimit.push_back(dirEqSize);
    //Next calculate width of each band
    std::vector<float>::iterator it;
    int i = 1;
    std::vector<float> bandLimitWidth;
    bandLimitWidth.push_back(dirEqSize);
    for (it = bandLimit.begin() + 1; it != bandLimit.end(); it++, i++) {
        bandLimitWidth.push_back(bandLimit[static_cast<unsigned long> (i)] - bandLimit[static_cast<unsigned long> (i - 1)]);
    }

    return bandLimitWidth;
}

bool PolarDesignerAudioProcessorEditor::getSoloActive()
{
    bool active = false;
    for (auto& but : tgbSolo)
    {
        if (but.getToggleState())
        {
            active = true;
            break;
        }
    }
    return active;
}


void PolarDesignerAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &trimSlider) {
        return;
    }
    else if (slider == &slCrossoverPosition[0] || slider == &slCrossoverPosition[1] ||
        slider == &slCrossoverPosition[2] || slider == &slCrossoverPosition[3])
    {
        // xOverSlider
        repaintPending = true; // Mark repaint as pending
        notifyPresetLabelChange();
        return;
    }
    else
    {
        // Check for direction sliders
        for (uint32 i = 0; i < MAX_EDITOR_BANDS; i++)
        {
            if (slider == &slDir[i])
            {
                polarPatternVisualizers[i].setDirWeight(static_cast<float>(slider->getValue()));
                repaintPending = true; // Mark repaint as pending
                notifyPresetLabelChange();
                return;
            }
        }
        // Check for band gain sliders
        for (uint32 i = 0; i < MAX_EDITOR_BANDS; i++)
        {
            if (slider == &slBandGain[i])
            {
                // Update directivityEqualiser if needed (e.g., gain visualization)
                repaintPending = true; // Mark repaint as pending
                notifyPresetLabelChange();
                return;
            }
        }
    }
}

void PolarDesignerAudioProcessorEditor::loadFile()
{
    FileChooser myChooser("Select Preset File",
        polarDesignerProcessor.getLastDir().exists() ? polarDesignerProcessor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory),
        "*.json");
    if (myChooser.browseForFileToOpen())
    {
        loadingFile = true;
        File presetFile(myChooser.getResult());
        polarDesignerProcessor.setLastDir(presetFile.getParentDirectory());

        // Reset saved states
//        ScopedLock lock(polarDesignerProcessor.abLayerLock);

        isStateSaved.fill(false);

        Result result = polarDesignerProcessor.loadPreset(presetFile);
        if (!result.wasOk())
        {
            errorMessage = result.getErrorMessage();
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Preset Load Failed", errorMessage);
            setMainAreaEnabled(true); // Recover UI
            activateMainUI(true);
        }
        else
        {
            setEqMode();
            // Save current state for the active layer
            uint32 currentLayer = polarDesignerProcessor.abLayerState;
            saveLayerState(currentLayer);
        }
        loadingFile = false;

        initializeSavedStates();
    }
}

void PolarDesignerAudioProcessorEditor::saveFile()
{
    FileChooser myChooser ("Save Preset File",
        polarDesignerProcessor.getLastDir().exists() ? polarDesignerProcessor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToSave (true))
    {
        File presetFile (myChooser.getResult());
        polarDesignerProcessor.setLastDir(presetFile.getParentDirectory());
        Result result = polarDesignerProcessor.savePreset (presetFile);
        if (!result.wasOk()) {
            errorMessage = result.getErrorMessage();
            setMainAreaEnabled(false);
            activateMainUI (false);
        }
        else
        {
            lbUserPresets.AddNewPresetToList(presetFile.getFileNameWithoutExtension());
        }
    }
}

void PolarDesignerAudioProcessorEditor::loadSavedPresetsToList()
{
    File presetDir(polarDesignerProcessor.getLastDir().exists() ? polarDesignerProcessor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory));
    auto presetsArray = presetDir.findChildFiles(File::findFiles, false, "*.json");

    String jsonString;
    for (File preset : presetsArray)
    {
        jsonString = preset.loadFileAsString();

        if (jsonString.contains("Austrian Audio PolarDesigner"))
        {
            if (jsonString.contains("Factory Preset"))
            {
                lbFactoryPresets.AddNewPresetToList(preset.getFileNameWithoutExtension());
            }
            else
            {
                lbUserPresets.AddNewPresetToList(preset.getFileNameWithoutExtension());
            }
        }
    }
}

void PolarDesignerAudioProcessorEditor::nEditorBandsChanged()
{
    nActiveBands = polarDesignerProcessor.getNProcessorBands();

    // Set nrbands button state when preset load
    tmbNrBandsButton[nActiveBands - 1].setToggleState(1, NotificationType::dontSendNotification);

    for (uint32 i = 0; i < 5; i++)
    {
        if (i < nActiveBands)
        {
            setBandEnabled(static_cast<int> (i), true);
            polarPatternVisualizers[i].setActive(true);
            polarPatternVisualizers[i].setVisible(true);
            polarPatternVisualizers[i].setToggleState(false, NotificationType::dontSendNotification);

            slDir[i].setVisible(true);
            slBandGain[i].setVisible(true);
            tgbSolo[i].setVisible(true);
            tgbMute[i].setVisible(true);
        }
        else
        {
            setBandEnabled(static_cast<int> (i), false);
            tgbSolo[i].setToggleState(false, NotificationType::sendNotification);
            tgbMute[i].setToggleState(false, NotificationType::sendNotification);
            polarPatternVisualizers[i].setActive(false);
            polarPatternVisualizers[i].setVisible(false);
            polarPatternVisualizers[i].setToggleState(false, NotificationType::dontSendNotification);

            slDir[i].setVisible(false);
            slBandGain[i].setVisible(false);
            tgbSolo[i].setVisible(false);
            tgbMute[i].setVisible(false);
        }
    }
    maxIt = 0;
    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
    polarDesignerProcessor.recomputeAllFilterCoefficients = true;
}

void PolarDesignerAudioProcessorEditor::timerCallback()
{
    //TRACE_COMPONENT();

    if (isRestoringState) return;

    bool needsRepaint = false;
    double currentTime = Time::getMillisecondCounterHiRes() / 1000.0; // Current time in seconds

#if 0
    // !J! TODO: refactor this, as it interferes with Logic Pro
    // timer must wait until recomputeAllFilterCoefficients is false
    if (polarDesignerProcessor.recomputeAllFilterCoefficients.get())
    {
        return;
    }
#endif

//    ScopedLock lock(polarDesignerProcessor.abLayerLock);

    // Handle debounced repaint
    if (repaintPending && (currentTime - lastRepaintTime >= repaintDebounceInterval))
    {
        directivityEqualiser.repaint();
        repaintPending = false;
        lastRepaintTime = currentTime;
        needsRepaint = true;
    }

    if (polarDesignerProcessor.repaintDEQ.get())
    {
        polarDesignerProcessor.repaintDEQ = false;
        needsRepaint = true;
    }

    if (polarDesignerProcessor.activeBandsChanged.get())
    {
        polarDesignerProcessor.activeBandsChanged = false;
        nEditorBandsChanged();
        needsRepaint = true;
    }

    if (polarDesignerProcessor.zeroLatencyModeChanged.get())
    {
        activateEditingForZeroLatency();
        polarDesignerProcessor.zeroLatencyModeChanged = false;
        needsRepaint = true;
    }

    if (polarDesignerProcessor.ffDfEqChanged.get())
    {
        polarDesignerProcessor.ffDfEqChanged = false;
        setEqMode();
        needsRepaint = true;
    }

    if (needsRepaint && !repaintPending)
    {
        directivityEqualiser.repaint();
        lastRepaintTime = currentTime;
    }

    if (uiTerminatorAnimationWindowIsVisible)
    {

            if (polarDesignerProcessor.playHeadPosition.getIsPlaying())
            {
                if (!uiTargetAquisitionWindowIsVisible)
                {
                    uiTargetAquisitionWindowIsVisible = true;
                    activateMainUI (false);
                    setMainAreaEnabled(false);
                    polarDesignerProcessor.startTracking(uiMaximizeTargetWindowIsVisible ? false : true);
                    albAcquiringTarget.startAnimation("ACQUIRING TARGET   ",
                    uiMaximizeTargetWindowIsVisible ? "STOP PLAYBACK WHEN READY TO MAXIMIZE  "
                                                      : "STOP PLAYBACK WHEN READY TO TERMINATE  ");
                    albPlaybackSpill.stopAnimation();
                    resized();
                }
            }
            else
            {
                if (uiTargetAquisitionWindowIsVisible)
                {
                    uiTargetAquisitionWindowIsVisible = false;
                    activateMainUI (true);
                    setMainAreaEnabled(true);
                    polarDesignerProcessor.stopTracking(1);
                    albAcquiringTarget.stopAnimation();
                    albPlaybackSpill.stopAnimation();

                    if (uiMaxTargetToSpillFlowStarted)
                    {
                        uiMaxToSpillWindowIsVisible = true;
                        uiTerminatorAnimationWindowIsVisible = false;
                        showActiveTerminatorStage(termStage);
                    }
                    else
                    {
                        uiTerminatorAnimationWindowIsVisible = false;
                    }
                    resized();
                }
            }

    }
    else
    {
        if (uiTargetAquisitionWindowIsVisible)
        {
            uiTargetAquisitionWindowIsVisible = false;
            activateMainUI (true);
            setMainAreaEnabled(true);
            polarDesignerProcessor.stopTracking(0);
            albAcquiringTarget.stopAnimation();
            albPlaybackSpill.stopAnimation();
            resized();
        }
    }
}


void PolarDesignerAudioProcessorEditor::activateMainUI (bool shouldBeActive)
{
    tmbNrBandsButton[0].setEnabled (shouldBeActive);
    tmbNrBandsButton[1].setEnabled (shouldBeActive);
    tmbNrBandsButton[2].setEnabled (shouldBeActive);
    tmbNrBandsButton[3].setEnabled (shouldBeActive);
    tmbNrBandsButton[4].setEnabled (shouldBeActive);
    tmbSyncChannelButton[0].setEnabled (shouldBeActive);
    tmbSyncChannelButton[1].setEnabled (shouldBeActive);
    tmbSyncChannelButton[2].setEnabled (shouldBeActive);
    tmbSyncChannelButton[3].setEnabled (shouldBeActive);

    tbLoad.setEnabled (shouldBeActive);
    tbSave.setEnabled (shouldBeActive);
    ibEqCtr[0].setEnabled (shouldBeActive);
    ibEqCtr[1].setEnabled (shouldBeActive);
    ibEqCtr[0].setVisible (shouldBeActive);
    ibEqCtr[1].setVisible (shouldBeActive);

    tbAllowBackwardsPattern.setEnabled (shouldBeActive);
    tbTerminateSpill.setEnabled (shouldBeActive);
    tbMaximizeTarget.setEnabled (shouldBeActive);
    tbMaxTargetToSpill.setEnabled (shouldBeActive);
    tgbProxCtr.setEnabled (shouldBeActive);
    if (tgbProxCtr.getToggleState())
    {
        slProximity.setEnabled (shouldBeActive);
    }
    grpEq.setEnabled (shouldBeActive);
    grpPreset.setEnabled (shouldBeActive);
    grpProxComp.setEnabled (shouldBeActive);
    grpBands.setEnabled (shouldBeActive);
    grpSync.setEnabled (shouldBeActive);
    grpPresetList.setEnabled (shouldBeActive);

    if (polarDesignerProcessor.zeroLatencyModeActive()) {
        grpTerminatorControl.setEnabled (shouldBeActive);
        tmbABButton.setEnabled(false);
        tmbABButton.setVisible(false);
        tbZeroLatency.setToggleState(true, NotificationType::sendNotification);
    }
    else {
        grpTerminatorControl.setEnabled (true);
        tmbABButton.setEnabled(shouldBeActive);
        tmbABButton.setVisible(shouldBeActive);
        tbZeroLatency.setToggleState(false, NotificationType::sendNotification);
    }

    if (tmbSyncChannelButton.getSelectedButton() != -1) {
        tmbABButton.setEnabled(false);
    }

    titlePreset.setEnabled(shouldBeActive);
    titleCompare.setEnabled(shouldBeActive);

    repaint();
}

void PolarDesignerAudioProcessorEditor::activateEditingForZeroLatency()
{
//    ScopedLock lock(polarDesignerProcessor.abLayerLock); // Add lock
    bool zlIsActive = polarDesignerProcessor.zeroLatencyModeActive();
    uint32 currentLayer = polarDesignerProcessor.abLayerState;

    if (!zlIsActive) {
        if (isStateSaved[currentLayer]) {
            restoreLayerState(currentLayer);
        }
        activateMainUI(true);
        tmbABButton.setEnabled(tmbSyncChannelButton.getSelectedButton() == -1);
        tmbABButton.setVisible(true);
        updateABButtonState();
        return;
    }

    if (!isStateSaved[currentLayer]) {
        saveLayerState(currentLayer);
    }

    nActiveBands = 1;
    valueTreeState.getParameter("nrBands")->setValueNotifyingHost(
        valueTreeState.getParameter("nrBands")->convertTo0to1(0));

    for (uint32 i = 0; i < MAX_EDITOR_BANDS; i++) {
        if (i < nActiveBands) {
            slDir[i].setEnabled(true);
            slBandGain[i].setEnabled(true);
            slBandGain[i].setVisible(true);
            tgbSolo[i].setEnabled(true);
            tgbSolo[i].setVisible(true);
            tgbMute[i].setEnabled(true);
            tgbMute[i].setVisible(true);
            polarPatternVisualizers[i].setActive(true);
            polarPatternVisualizers[i].setVisible(true);
        } else {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            slBandGain[i].setVisible(false);
            tgbSolo[i].setEnabled(false);
            tgbSolo[i].setToggleState(false, NotificationType::sendNotificationSync);
            tgbMute[i].setEnabled(false);
            tgbMute[i].setToggleState(false, NotificationType::sendNotificationSync);
            tgbMute[i].setVisible(false);
            polarPatternVisualizers[i].setActive(false);
            polarPatternVisualizers[i].setVisible(false);
        }
    }

    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
    activateMainUI(false);
    tmbABButton.setEnabled(false);
    tmbABButton.setVisible(false);
}

void PolarDesignerAudioProcessorEditor::showPresetList(bool shouldShow)
{
    lbUserPresets.deselectAll();
    lbFactoryPresets.deselectAll();
    presetListVisible = shouldShow;
    tbLogoAA.setVisible(!shouldShow);

    if(shouldShow) {
        polarDesignerProcessor.undoManager.beginNewTransaction("Loading preset");
    }

    resized();
}

void PolarDesignerAudioProcessorEditor::setBandEnabled(int bandNr, bool enable)
{
    directivityEqualiser.getDirPathComponent(bandNr).setEnabled(enable);
    slDir[bandNr].setEnabled(enable);
    tgbSolo[bandNr].setEnabled(enable);
    tgbMute[bandNr].setEnabled(enable);
    slBandGain[bandNr].setEnabled(enable);
    repaint();
}

void PolarDesignerAudioProcessorEditor::showActiveTerminatorStage(terminatorStage stage)
{
    if (stage == terminatorStage::DISABLED)
    {
        terminatorLabelNr1.setVisible(false);;
        terminatorLabelSpillMain.setVisible(false);
        terminatorLabelSpillSub.setVisible(false);
        tbBeginTerminate.setVisible(false);

        terminatorLabelNr2.setVisible(false);
        terminatorLabelMaxMain.setVisible(false);
        terminatorLabelMaxSub.setVisible(false);
        tbBeginMaximize.setVisible(false);

        terminatorLabelNr3.setVisible(false);
        terminatorLabelMaxToSpillMain.setVisible(false);
        terminatorLabelMaxToSpillSub.setVisible(false);
        tbApplyMaxTargetToSpill.setVisible(false);

        for (auto &line : terminatorStageLine)
        {
            line.setVisible(false);
        }

        terminatorLabelNr1.setToggleState(false, NotificationType::dontSendNotification);
        terminatorLabelNr2.setToggleState(false, NotificationType::dontSendNotification);
        terminatorLabelNr3.setToggleState(false, NotificationType::dontSendNotification);
    }
    else if (stage == terminatorStage::TERMINATE)
    {
        terminatorLabelNr1.setVisible(true);;
        terminatorLabelSpillMain.setVisible(true);
        terminatorLabelSpillSub.setVisible(true);
        tbBeginTerminate.setVisible(true);

        terminatorLabelNr1.setEnabled(true);
        terminatorLabelSpillMain.setEnabled(true);
        terminatorLabelSpillSub.setEnabled(true);
        tbBeginTerminate.setEnabled(true);

        terminatorLabelNr2.setVisible(true);
        terminatorLabelMaxMain.setVisible(true);
        terminatorLabelNr2.setEnabled(false);
        terminatorLabelMaxMain.setEnabled(false);

        terminatorLabelNr3.setVisible(true);
        terminatorLabelMaxToSpillMain.setVisible(true);
        terminatorLabelNr3.setEnabled(false);
        terminatorLabelMaxToSpillMain.setEnabled(false);

        for (auto& line : terminatorStageLine)
        {
            line.setVisible(true);
        }
        terminatorStageLine[2].setVisible(false);
        terminatorStageLine[3].setVisible(false);

        terminatorLabelNr1.setToggleState(false, NotificationType::dontSendNotification);
        terminatorLabelNr2.setToggleState(false, NotificationType::dontSendNotification);
        terminatorLabelNr3.setToggleState(false, NotificationType::dontSendNotification);
    }
    else if (stage == terminatorStage::MAXIMIZE)
    {
        terminatorLabelNr1.setVisible(true);;
        terminatorLabelSpillMain.setVisible(true);
        terminatorLabelSpillSub.setVisible(false);
        tbBeginTerminate.setVisible(false);
        terminatorLabelNr1.setEnabled(false);;
        terminatorLabelSpillMain.setEnabled(false);

        terminatorLabelNr2.setVisible(true);
        terminatorLabelMaxMain.setVisible(true);
        terminatorLabelMaxSub.setVisible(true);
        tbBeginMaximize.setVisible(true);
        terminatorLabelNr2.setEnabled(true);
        terminatorLabelMaxMain.setEnabled(true);
        terminatorLabelMaxSub.setEnabled(true);
        tbBeginMaximize.setEnabled(true);

        terminatorLabelNr3.setVisible(true);
        terminatorLabelMaxToSpillMain.setVisible(true);
        terminatorLabelNr3.setEnabled(false);;
        terminatorLabelMaxToSpillMain.setEnabled(false);

        for (auto& line : terminatorStageLine)
        {
            line.setVisible(true);
        }
        terminatorStageLine[0].setVisible(false);
        terminatorStageLine[1].setVisible(false);

        terminatorLabelNr1.setToggleState(true, NotificationType::dontSendNotification);
        terminatorLabelNr2.setToggleState(false, NotificationType::dontSendNotification);
        terminatorLabelNr3.setToggleState(false, NotificationType::dontSendNotification);
    }
    else if (stage == terminatorStage::MAXTOSPILL)
    {
        terminatorLabelNr1.setVisible(true);;
        terminatorLabelSpillMain.setVisible(true);
        terminatorLabelSpillSub.setVisible(false);
        tbBeginTerminate.setVisible(false);
        terminatorLabelNr1.setEnabled(false);;
        terminatorLabelSpillMain.setEnabled(false);

        terminatorLabelNr2.setVisible(true);
        terminatorLabelMaxMain.setVisible(true);
        terminatorLabelMaxSub.setVisible(false);
        tbBeginMaximize.setVisible(false);
        terminatorLabelNr2.setEnabled(false);
        terminatorLabelMaxMain.setEnabled(false);

        terminatorLabelNr3.setVisible(true);
        terminatorLabelMaxToSpillMain.setVisible(true);
        terminatorLabelMaxToSpillSub.setVisible(true);
        tbApplyMaxTargetToSpill.setVisible(true);
        terminatorLabelNr3.setEnabled(true);
        terminatorLabelMaxToSpillMain.setEnabled(true);
        terminatorLabelMaxToSpillSub.setEnabled(true);
        tbApplyMaxTargetToSpill.setEnabled(true);

        terminatorStageLine[0].setVisible(false);
        terminatorStageLine[1].setVisible(false);
        terminatorStageLine[2].setVisible(false);
        terminatorStageLine[3].setVisible(false);
        terminatorStageLine[4].setVisible(true);
        terminatorStageLine[5].setVisible(true);
        terminatorStageLine[6].setVisible(false);
        terminatorStageLine[7].setVisible(false);

        terminatorLabelNr1.setToggleState(true, NotificationType::dontSendNotification);
        terminatorLabelNr2.setToggleState(true, NotificationType::dontSendNotification);
        terminatorLabelNr3.setToggleState(false, NotificationType::dontSendNotification);
    }
}

void PolarDesignerAudioProcessorEditor::notifyPresetLabelChange()
{
    if (presetLoaded)
    {
        titlePresetUndoButton.setVisible(false);
        presetLoaded = false;
        titlePreset.setTitle(titlePreset.getTitle()+"*");
    }
}

void PolarDesignerAudioProcessorEditor::mouseDown(const MouseEvent& event)
{
    //Event for hiding preset panel when clicking outside
    if (presetListVisible && !presetArea.contains(event.mouseDownPosition))
    {
        showPresetList(false);
        polarDesignerProcessor.undoManager.undo();
    }

    for (int i = 0; i < 5; i++)
    {
        polarPatternVisualizers[i].setToggleState(false, NotificationType::dontSendNotification);
        setBandEnabled(i, true);
    }
}

void PolarDesignerAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == &lbUserPresets)
    {
        File presetDir(polarDesignerProcessor.getLastDir().exists() ? polarDesignerProcessor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory));
        auto selectedPreset = lbUserPresets.getSelectedPresetName();
        auto presetFile = presetDir.findChildFiles(File::findFiles, false, String(selectedPreset + ".json"));

        if (presetFile.size() == 1)
        {
            // Reset saved states
//            ScopedLock lock(polarDesignerProcessor.abLayerLock);

            isStateSaved.fill(false);

            polarDesignerProcessor.loadPreset(presetFile.getFirst());
            lbFactoryPresets.deselectAll();
            if (lbUserPresets.isRowDoubleClicked())
            {
                titlePreset.setTitle(String("Preset: " + selectedPreset));
                showPresetList(false);
                presetLoaded = true;
                titlePresetUndoButton.setVisible(true);
                // Save current state for the active layer
                uint32 currentLayer = polarDesignerProcessor.abLayerState;
                saveLayerState(currentLayer);
            }
        }
    }
    else if (source == &lbFactoryPresets)
    {
        File presetDir(polarDesignerProcessor.getLastDir().exists() ? polarDesignerProcessor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory));
        auto selectedPreset = lbFactoryPresets.getSelectedPresetName();
        auto presetFile = presetDir.findChildFiles(File::findFiles, false, String(selectedPreset + ".json"));

        if (presetFile.size() == 1)
        {
            // Reset saved states
//            ScopedLock lock(polarDesignerProcessor.abLayerLock);

            isStateSaved.fill(false);

            polarDesignerProcessor.loadPreset(presetFile.getFirst());
            lbUserPresets.deselectAll();
            if (lbFactoryPresets.isRowDoubleClicked())
            {
                titlePreset.setTitle(String("Preset: " + selectedPreset));
                showPresetList(false);
                presetLoaded = true;
                titlePresetUndoButton.setVisible(true);
                // Save current state for the active layer
                uint32 currentLayer = polarDesignerProcessor.abLayerState;
                saveLayerState(currentLayer);
            }
        }
    }
}


void PolarDesignerAudioProcessorEditor::setMainAreaEnabled(bool enable)
{
    directivityEqualiser.setActive(enable);
    directivityEqualiser.setEnabled(enable);

    for (uint32 i = 0; i < nActiveBands; i++)
    {
        directivityEqualiser.getDirPathComponent(static_cast<int> (i)).setEnabled(enable);
        slDir[i].setEnabled(enable);
        slBandGain[i].setEnabled(enable);
        tgbSolo[i].setEnabled(enable);
        tgbMute[i].setEnabled(enable);
        polarPatternVisualizers[i].setEnabled(enable);
        polarPatternVisualizers[i].setActive(enable);
        if (polarPatternVisualizers[i].getToggleState())
            polarPatternVisualizers[i].setToggleState(false, NotificationType::dontSendNotification);
    }
    tbZeroLatency.setEnabled(enable);
    titlePreset.setEnabled(enable);
    repaint();
}

void PolarDesignerAudioProcessorEditor::setEqMode()
{
    int activeIdx = polarDesignerProcessor.getEqState();
    if (activeIdx == 0)
    {
        ibEqCtr[0].setToggleState(false, NotificationType::dontSendNotification);
        ibEqCtr[1].setToggleState(false, NotificationType::dontSendNotification);
    }
    else if (activeIdx == 1)
    {
        ibEqCtr[0].setToggleState(true, NotificationType::dontSendNotification);
        ibEqCtr[1].setToggleState(false, NotificationType::dontSendNotification);
    }
    else if (activeIdx == 2)
    {
        ibEqCtr[0].setToggleState(false, NotificationType::dontSendNotification);
        ibEqCtr[1].setToggleState(true, NotificationType::dontSendNotification);
    }
    repaint();
}

void PolarDesignerAudioProcessorEditor::calculateLockedBands(int nBands, bool trimSliderIncr)
{
    // First check if any of the band reach min/max value
    int minIt = -1;
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].isEnabled() &&
            (exactlyEqual(slDir[i].getValue(), (trimSliderIncr ? 1.0 : -0.5))))
        {
            minIt = i;
            break;
        }
    }
    // Count locked bands for future use
    int counter = 0;
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].isEnabled() && bandLockedOnMinMax[i]) counter++;
    }
    // Leave maxIt band always unlocked
    if (counter < 4)
    {
        for (int i = 0; i < nBands; i++)
        {
            if (trimSliderIncr)
            {
                if (slDir[i].isEnabled() && (slDir[i].getValue() < slDir[maxIt].getValue()))
                    maxIt = i;
            }
            else
            {
                if (slDir[i].isEnabled() && (slDir[i].getValue() > slDir[maxIt].getValue()))
                    maxIt = i;
            }
        }
    }
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].isEnabled())
        {
            if (exactlyEqual(slDir[i].getValue(), (trimSliderIncr ? 1 : -0.5)) && (i != maxIt))
            {
                bandLockedOnMinMax[i] = true;
            }
            else
            {
                bandLockedOnMinMax[i] = false;
            }
        }
    }
    // Detect change of band slider when drag only one band
    if (minIt == -1 || directivityEqualiser.isDirSliderLastChangedByDrag())
    {
        minBandValueDistancesSet = false;
    }
    // Calculate distances between min/max value and rest of bands
    if (minIt != -1 && !minBandValueDistancesSet)
    {
        for (int i = 0; i < nBands; i++)
        {
            if (slDir[i].isEnabled())
            {
                minBandValueDistances[i] = static_cast<float> (slDir[i].getValue() - slDir[minIt].getValue());
            }
        }
        minBandValueDistancesSet = true;
    }
    // Unlock bands that reach given distance from min.max value
    if (minIt != -1 && minBandValueDistancesSet)
    {
        for (int i = 0; i < nBands; i++)
        {
            if (slDir[i].isEnabled())
            {
                if (trimSliderIncr)
                {
                    float sliderVal = (static_cast<float> (1.f - std::abs (slDir[maxIt].getValue())));
                    if (slDir[maxIt].getValue() < 0)
                    {
                        sliderVal = (static_cast<float> (1.f + std::abs (slDir[maxIt].getValue())));
                    }
                    if (sliderVal >= std::abs(minBandValueDistances[maxIt]) + minBandValueDistances[i])
                    {
                        bandLockedOnMinMax[i] = false;
                    }
                }
                else
                {
                    float sliderVal = static_cast<float> (std::abs (0.5f + slDir[maxIt].getValue()));
                    if (sliderVal >= minBandValueDistances[maxIt] - minBandValueDistances[i])
                    {
                        bandLockedOnMinMax[i] = false;
                    }
                }
            }
        }
    }
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
    else if (&control == &tgbSolo[0])
        return 9;
    else if (&control == &tgbSolo[1] && nActiveBands > 1)
        return 10;
    else if (&control == &tgbSolo[2] && nActiveBands > 2)
        return 11;
    else if (&control == &tgbSolo[3] && nActiveBands > 3)
        return 12;
    else if (&control == &tgbSolo[4] && nActiveBands > 4)
        return 13;
    else if (&control == &tgbMute[0])
        return 14;
    else if (&control == &tgbMute[1] && nActiveBands > 1)
        return 15;
    else if (&control == &tgbMute[2] && nActiveBands > 2)
        return 16;
    else if (&control == &tgbMute[3] && nActiveBands > 3)
        return 17;
    else if (&control == &tgbMute[4] && nActiveBands > 4)
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


void PolarDesignerAudioProcessorEditor::saveLayerState(uint32 layer)
{
//    ScopedLock lock(polarDesignerProcessor.abLayerLock);

    LayerState& state = savedStates[layer];
    state.nrBandsValue = polarDesignerProcessor.getNProcessorBands();

    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i) {
        state.dirValues[i] = static_cast<float>(slDir[i].getValue());
        state.bandGainValues[i] = static_cast<float>(slBandGain[i].getValue());
        state.soloStates[i] = tgbSolo[i].getToggleState();
        state.muteStates[i] = tgbMute[i].getToggleState();
        if (i < MAX_EDITOR_BANDS - 1) {
            state.crossoverValues[i] = static_cast<float>(slCrossoverPosition[i].getValue());
        }
    }

    state.proxCtrState = tgbProxCtr.getToggleState();
    state.proxValue = static_cast<float>(slProximity.getValue());
    state.eqState = polarDesignerProcessor.getEqState();

    isStateSaved[layer] = true;
    DBG("Saved state for layer " + String(layer) + ": nrBands=" + String(state.nrBandsValue));
}


void PolarDesignerAudioProcessorEditor::restoreLayerState(uint32 layer)
{
    if (!isStateSaved[layer]) {
        DBG("No state saved for layer " + String(layer));
        return;
    }

    isRestoringState = true;
//    ScopedLock lock(polarDesignerProcessor.abLayerLock);
    const LayerState& state = savedStates[layer];

    // Validate nrBandsValue
    if (state.nrBandsValue < 1 || state.nrBandsValue > MAX_EDITOR_BANDS) {
        DBG("Invalid nrBandsValue " << std::to_string(state.nrBandsValue) << " for layer " + String(layer) + "; resetting to default");
        initializeSavedStates(); // Reset to safe state
        isStateSaved[layer] = false;
        isRestoringState = false;
        return;
    }

    DBG("Restoring state for layer " + String(layer) + ": nrBands=" << std::to_string(state.nrBandsValue));

    // Begin batch update
    valueTreeState.getParameter("nrBands")->beginChangeGesture();
    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i) {
        valueTreeState.getParameter("alpha" + String(i+1))->beginChangeGesture();
        valueTreeState.getParameter("gain" + String(i+1))->beginChangeGesture();
        valueTreeState.getParameter("solo" + String(i+1))->beginChangeGesture();
        valueTreeState.getParameter("mute" + String(i+1))->beginChangeGesture();
        if (i < MAX_EDITOR_BANDS - 1) {
            valueTreeState.getParameter("xOverF" + String(i+1))->beginChangeGesture();
        }
    }
    valueTreeState.getParameter("proximityOnOff")->beginChangeGesture();
    valueTreeState.getParameter("proximity")->beginChangeGesture();

    // Set number of bands
    polarDesignerProcessor.setNProcessorBands(state.nrBandsValue);
    valueTreeState.getParameter("nrBands")->setValueNotifyingHost(
        valueTreeState.getParameter("nrBands")->convertTo0to1(state.nrBandsValue - 1));
    nActiveBands = state.nrBandsValue;

    // Restore band parameters
    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i) {
        if (i < state.nrBandsValue) {
            valueTreeState.getParameter("alpha" + String(i+1))->setValueNotifyingHost(
                valueTreeState.getParameter("alpha" + String(i+1))->convertTo0to1(state.dirValues[i]));
            valueTreeState.getParameter("gain" + String(i+1))->setValueNotifyingHost(
                valueTreeState.getParameter("gain" + String(i+1))->convertTo0to1(state.bandGainValues[i]));
            valueTreeState.getParameter("solo" + String(i+1))->setValueNotifyingHost(
                state.soloStates[i] ? 1.0f : 0.0f);
            valueTreeState.getParameter("mute" + String(i+1))->setValueNotifyingHost(
                state.muteStates[i] ? 1.0f : 0.0f);
            if (i < MAX_EDITOR_BANDS - 1) {
                valueTreeState.getParameter("xOverF" + String(i+1))->setValueNotifyingHost(
                    valueTreeState.getParameter("xOverF" + String(i+1))->convertTo0to1(state.crossoverValues[i]));
            }
            polarPatternVisualizers[i].setDirWeight(state.dirValues[i]);
        }
    }

    // Restore proximity control
    valueTreeState.getParameter("proximityOnOff")->setValueNotifyingHost(state.proxCtrState ? 1.0f : 0.0f);
    valueTreeState.getParameter("proximity")->setValueNotifyingHost(
        valueTreeState.getParameter("proximity")->convertTo0to1(state.proxValue));

    // End batch update
    valueTreeState.getParameter("nrBands")->endChangeGesture();
    for (uint32 i = 0; i < MAX_EDITOR_BANDS; ++i) {
        valueTreeState.getParameter("alpha" + String(i+1))->endChangeGesture();
        valueTreeState.getParameter("gain" + String(i+1))->endChangeGesture();
        valueTreeState.getParameter("solo" + String(i+1))->endChangeGesture();
        valueTreeState.getParameter("mute" + String(i+1))->endChangeGesture();
        if (i < MAX_EDITOR_BANDS - 1) {
            valueTreeState.getParameter("xOverF" + String(i+1))->endChangeGesture();
        }
    }
    valueTreeState.getParameter("proximityOnOff")->endChangeGesture();
    valueTreeState.getParameter("proximity")->endChangeGesture();

    // Restore EQ mode
    polarDesignerProcessor.setEqState(state.eqState);
    setEqMode();

    // Update UI
    nEditorBandsChanged();
    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
    activateMainUI(true);

    isRestoringState = false;
    DBG("State restored: nrBands=" << std::to_string(polarDesignerProcessor.getNProcessorBands()));
}