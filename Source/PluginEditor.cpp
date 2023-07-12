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
    directivityEqualiser (p),
    presetListVisible(false),
    showTerminatorAnimationWindow(false),
    isTargetAquiring(false),
    maximizeTarget(false),
    showMaxToSpillWindow(false),
    maxTargetToSpillFlowStarted(false),
    termStage(PolarDesignerAudioProcessorEditor::terminatorStage::DISABLED)
{
//    openGLContext.attachTo (*getTopLevelComponent());

    nActiveBands = processor.getNBands();
    syncChannelIdx = processor.getSyncChannelIdx();

    setResizable(true, true);
    setSize(EDITOR_WIDTH, EDITOR_HEIGHT);
    setLookAndFeel(&mainLaF);

    logoAA.setLogoColour(mainLaF.mainTextColor);
    addAndMakeVisible(&logoAA);

    addAndMakeVisible(&titlePD);
    titlePD.setTitle(String("PolarDesigner"));
    titlePD.setFont(mainLaF.normalFont);
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
    tmbABButton[0].setToggleState(processor.abLayerState, NotificationType::dontSendNotification);

    tmbABButton[1].setClickingTogglesState(true);
    tmbABButton[1].setRadioGroupId(3344);
    tmbABButton[1].setButtonText("B");
    tmbABButton[1].addListener(this);
    tmbABButton[1].setToggleState(processor.abLayerState, NotificationType::dontSendNotification);

    addAndMakeVisible(&tbZeroDelay);
    tbZeroDelayAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment(valueTreeState, "zeroDelayMode", tbZeroDelay));
    tbZeroDelay.addListener(this);
    tbZeroDelay.setButtonText("Zero latency");
    tbZeroDelay.setToggleState(processor.zeroDelayModeActive(), NotificationType::dontSendNotification);

    addAndMakeVisible(&titlePreset);
    titlePreset.setTitle(String("Preset"));
    titlePreset.setFont(mainLaF.normalFont);
    titlePreset.setInterceptsMouseClicks(false, true);

    addAndMakeVisible(&tbLoad);
    tbLoad.addListener(this);
    tbLoad.setButtonText("Load");
    tbLoad.setToggleState(false, NotificationType::dontSendNotification);

    addAndMakeVisible(&tbSave);
    tbSave.addListener(this);
    tbSave.setButtonText("Save");
    tbSave.setToggleState(false, NotificationType::dontSendNotification);

    addAndMakeVisible(&tmbNrBandsButton);
    tmbNrBandsButton.setButtonsNumber(maxNumberBands);
    tmbNrBandsButton.setAlwaysOnTop(true);

    for (int i = 0; i < maxNumberBands; ++i)
    {
        tmbNrBandsButton[i].setClickingTogglesState(true);
        tmbNrBandsButton[i].setRadioGroupId(34567);

        tmbNrBandsButton[i].setButtonText(String(i + 1));
        tmbNrBandsButton[i].addListener(this);

        if (i == nActiveBands - 1) tmbNrBandsButton[i].setToggleState(true, NotificationType::dontSendNotification);
    }

    addAndMakeVisible (&footer);
    footer.setInterceptsMouseClicks(false, true);

    // groups
    addAndMakeVisible (&grpBands);
    grpBands.setText ("Number of bands");

    addAndMakeVisible(&grpEq);
    grpEq.setText("Equalization control");

    addAndMakeVisible(&ibEqCtr[0]);
    ibEqCtr[0].setClickingTogglesState(true);
    ibEqCtr[0].setButtonText("Free Field");
    ibEqCtr[0].addListener(this);

    addAndMakeVisible(&ibEqCtr[1]);
    ibEqCtr[1].setClickingTogglesState(true);
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

    addAndMakeVisible(&grpDstC);
    grpDstC.setText("Terminator control");

    addAndMakeVisible(&tbCloseTerminatorControl);
    tbCloseTerminatorControl.setComponentID("5721");
    tbCloseTerminatorControl.setToggleState(false, NotificationType::dontSendNotification);
    tbCloseTerminatorControl.addListener(this);

    addAndMakeVisible(processor.termControlWaveform);
    processor.termControlWaveform.setColours(mainLaF.labelBackgroundColor, mainLaF.textButtonActiveRedFrameColor);

    addAndMakeVisible(&albPlaybackSpill);

    addAndMakeVisible(&albAcquiringTarget);

    addAndMakeVisible(&grpSync);
    grpSync.setText("Sync group");

    addAndMakeVisible(&tmbSyncChannelButton);
    tmbSyncChannelButton.setButtonsNumber(4);
    tmbSyncChannelButton.setAlwaysOnTop(true);

    for (int i = 0; i < 4; ++i)
    {
        tmbSyncChannelButton[i].setClickingTogglesState(true);

        tmbSyncChannelButton[i].setButtonText(String(i+1));
        tmbSyncChannelButton[i].addListener(this);
        //Set ID manually for drawing colors purpose
        tmbSyncChannelButton[i].setComponentID(String("552" + String(i + 1)));

        if (i == syncChannelIdx - 1) tmbSyncChannelButton[i].setToggleState(true, NotificationType::dontSendNotification);
    }

    eqColours[0] = mainLaF.polarVisualizerRed;
    eqColours[1] = mainLaF.polarVisualizerOrange;
    eqColours[2] = mainLaF.polarVisualizerYellow;
    eqColours[3] = mainLaF.polarVisualizerGreen;
    eqColours[4] = mainLaF.polarVisualizerGreenDark;
    
    // directivity eq
    addAndMakeVisible (&directivityEqualiser);
    
    for (int i = 0; i < maxNumberBands; ++i)
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
        slBandGain[i].setColour (Slider::rotarySliderOutlineColourId, eqColours[i]);
        slBandGain[i].setColour (Slider::thumbColourId, eqColours[i]);
        slBandGain[i].setTextBoxStyle(Slider::TextBoxRight, false, 45, 15);
        slBandGain[i].setTextValueSuffix(" dB");
        slBandGain[i].addListener (this);

        // First-Order directivity visualizer (The "O"verhead view)
        addAndMakeVisible (&polarPatternVisualizers[i]);
        polarPatternVisualizers[i].setActive(true);
        polarPatternVisualizers[i].addListener(this);
        polarPatternVisualizers[i].setDirWeight (slDir[i].getValue());
        polarPatternVisualizers[i].setMuteSoloButtons (&tgbSolo[i], &tgbMute[i]);
        polarPatternVisualizers[i].setColour (eqColours[i]);

        // main directivity Equaliser section
        directivityEqualiser.addSliders (eqColours[i], &slDir[i], (i > 0) ? &slCrossoverPosition[i - 1] : nullptr, (i < maxNumberBands - 1) ? &slCrossoverPosition[i] : nullptr, &tgbSolo[i], &tgbMute[i], &slBandGain[i], &polarPatternVisualizers[i]);
        
        if (i == maxNumberBands - 1)
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

    nActiveBandsChanged();
    zeroDelayModeChange();

    trimSlider.sliderIncremented = [this] { incrementTrim(nActiveBands); };
    trimSlider.sliderDecremented = [this] { decrementTrim(nActiveBands); };
    
    nActiveBandsChanged();
    zeroDelayModeChange();
    
    addAndMakeVisible(&trimSlider);

    addAndMakeVisible(&tbTrimSliderCenterPointer);
    tbTrimSliderCenterPointer.setButtonText("Trim Slider Pointer");

    loadSavedPresetsToList();

    startTimer (30);
    
    setEqMode();
}

// Handle the trimSlider increment/decrement calls
void PolarDesignerAudioProcessorEditor::incrementTrim(int nBands) {
    for (int i = 0; i < nBands; i++)
    {
        if(slDir[i].isEnabled())
            slDir[i].setValue(slDir[i].getValue() + trimSlider.step);
    }
}

void PolarDesignerAudioProcessorEditor::decrementTrim(int nBands) {
    for (int i = 0; i < nBands; i++)
    {
        if (slDir[i].isEnabled())
            slDir[i].setValue(slDir[i].getValue() - trimSlider.step);
    }
}

PolarDesignerAudioProcessorEditor::~PolarDesignerAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void PolarDesignerAudioProcessorEditor::paint (Graphics& g)
{
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

    const float marginFlex = 0.022f;
    const float topComponentTitleFlex = 0.4f;
    const float topComponentButtonsFlex = 0.035f;
    const float topComponentSpacingFlex = topComponentButtonsFlex / 2;
    const float topComponentButtonsMargin = 5;
    const float radioButonsFlex = 0.18f;
    const float radioButonsSpaceFlex = 0.025f;

    topComponent.items.add(juce::FlexItem().withFlex(0.022f));
    if (!presetListVisible)
    {
        topComponent.items.add(juce::FlexItem(logoAA).withFlex(0.033f));
        topComponent.items.add(juce::FlexItem().withFlex(0.007f));
        topComponent.items.add(juce::FlexItem(titlePD).withFlex(0.14f));
        topComponent.items.add(juce::FlexItem().withFlex(0.063f));
    }

    topComponent.items.add(juce::FlexItem(titleCompare).withFlex(0.063f));
    topComponent.items.add(juce::FlexItem().withFlex(0.016f));
    topComponent.items.add(juce::FlexItem(tmbABButton).withFlex(0.077f).withMargin(2));
    topComponent.items.add(juce::FlexItem().withFlex(0.042f));
    topComponent.items.add(juce::FlexItem(tbZeroDelay).withFlex(0.1f).withMargin(2));
    topComponent.items.add(juce::FlexItem().withFlex(0.1f));
    topComponent.items.add(juce::FlexItem(titlePreset).withFlex(0.12f));
    topComponent.items.add(juce::FlexItem().withFlex(0.022f));
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
    const float polarVisualizersComponentLeftMargin = 33;
    const float polarVisualizersComponentRightMargin = 10;

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
    for (int i = 0; i < 5; i++)
    {
        muteSoloComponent[i].flexDirection = FlexBox::Direction::row;
        muteSoloComponent[i].justifyContent = juce::FlexBox::JustifyContent::center;
        muteSoloComponent[i].alignContent = juce::FlexBox::AlignContent::center;
        muteSoloComponent[i].items.add(juce::FlexItem().withFlex(0.4f));
        muteSoloComponent[i].items.add(juce::FlexItem(tgbSolo[i]).withWidth(0.04 * EDITOR_HEIGHT).withHeight(0.04 * EDITOR_HEIGHT));
        muteSoloComponent[i].items.add(juce::FlexItem().withWidth(0.025 * EDITOR_HEIGHT));
        muteSoloComponent[i].items.add(juce::FlexItem(tgbMute[i]).withWidth(0.04 * EDITOR_HEIGHT).withHeight(0.04 * EDITOR_HEIGHT));
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
    gainBandSlidersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentLeftMargin));

    //Dynamic layout for polarVisualizers and dirSlider components
    //offsetDirEQ and offsetPolVis are fixed values because DirectivityEQ component has fixed margins
    const float offsetDirEQ = 42;
    const float offsetPolVis = 29;

    const float dirEqSize = directivityEqualiser.getWidth() - offsetDirEQ;
    auto bandLimitWidth = getBandLimitWidthVector(dirEqSize, offsetPolVis);

    //pVisflex - value used for components spacing across given area i.e 0.65 (maximum 1.0 means full space)
    float pVisflex = 0;

    if (nActiveBands < 2)
    {
        if (polarPatternVisualizers[0].isPvisActive())
        {
            pVisflex = bandLimitWidth[0] / dirEqSize;
            polarVisualizersComponent.items.add(juce::FlexItem(polarPatternVisualizers[0]).withFlex(pVisflex));
            dirSlidersComponent.items.add(juce::FlexItem(slDir[0]).withFlex(pVisflex));
            muteSoloModule.items.add(juce::FlexItem(muteSoloComponent[0]).withFlex(pVisflex));
            gainBandSlidersComponent.items.add(juce::FlexItem(slBandGain[0]).withFlex(pVisflex));
        }
    }
    else
    {
        for (int i = 0; i < nActiveBands; i++)
        {
            if (polarPatternVisualizers[i].isPvisActive())
            {
                //TODO: modify the function so that there is no danger of going outside the array --> i+1
                pVisflex = bandLimitWidth[i+1] / dirEqSize;
                polarVisualizersComponent.items.add(juce::FlexItem(polarPatternVisualizers[i]).withFlex(pVisflex));
                dirSlidersComponent.items.add(juce::FlexItem(slDir[i]).withFlex(pVisflex));
                muteSoloModule.items.add(juce::FlexItem(muteSoloComponent[i]).withFlex(pVisflex));
                gainBandSlidersComponent.items.add(juce::FlexItem(slBandGain[i]).withFlex(pVisflex));
            }
        }
    }

    polarVisualizersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentRightMargin));
    dirSlidersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentRightMargin));
    muteSoloModule.items.add(juce::FlexItem().withWidth(polarVisualizersComponentRightMargin));
    gainBandSlidersComponent.items.add(juce::FlexItem().withWidth(polarVisualizersComponentRightMargin));

    juce::FlexBox middleComponent;
    middleComponent.flexDirection = FlexBox::Direction::column;
    middleComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    middleComponent.alignContent = juce::FlexBox::AlignContent::center;
    middleComponent.items.add(juce::FlexItem(polarVisualizersComponent).withFlex(0.24f));
    middleComponent.items.add(juce::FlexItem(directivityEqualiser).withFlex(0.56f));
    middleComponent.items.add(juce::FlexItem().withFlex(0.04f));
    middleComponent.items.add(juce::FlexItem(muteSoloModule).withFlex(0.07f));
    middleComponent.items.add(juce::FlexItem(gainBandSlidersComponent).withFlex(0.25f));
    middleComponent.items.add(juce::FlexItem().withFlex(0.03f));

    juce::FlexBox trimSliderComponent;
    trimSliderComponent.flexDirection = FlexBox::Direction::column;
    trimSliderComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    trimSliderComponent.alignContent = juce::FlexBox::AlignContent::center;
    trimSliderComponent.items.add(juce::FlexItem().withFlex(0.24f));
    trimSliderComponent.items.add(juce::FlexItem(trimSlider).withFlex(0.41f));
    trimSliderComponent.items.add(juce::FlexItem().withFlex(0.35f));

    juce::FlexBox trimSliderCenterPointerComponent;
    trimSliderCenterPointerComponent.flexDirection = FlexBox::Direction::column;
    trimSliderCenterPointerComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    trimSliderCenterPointerComponent.alignContent = juce::FlexBox::AlignContent::center;
    trimSliderCenterPointerComponent.items.add(juce::FlexItem().withFlex(0.24f));
    trimSliderCenterPointerComponent.items.add(juce::FlexItem(tbTrimSliderCenterPointer).withFlex(0.41f));
    trimSliderCenterPointerComponent.items.add(juce::FlexItem().withFlex(0.35f));
    
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
    mainComponent.items.add(juce::FlexItem(middleComponent).withFlex(0.66f));
    mainComponent.items.add(juce::FlexItem(trimSliderCenterPointerComponent).withFlex(0.017f));
    mainComponent.items.add(juce::FlexItem(trimSliderComponent).withFlex(0.03f));
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
    fbTerminatorControlOutComp.items.add(juce::FlexItem{ grpDstC }.withFlex(1.0f));
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

    if (showTerminatorAnimationWindow)
    {
        tbCloseTerminatorControl.setVisible(true);
        tbCloseTerminatorControl.setVisible(true);
        albPlaybackSpill.setVisible(!isTargetAquiring);
        albAcquiringTarget.setVisible(isTargetAquiring);
        processor.termControlWaveform.setVisible(true);
        tbTerminateSpill.setVisible(false);
        tbMaximizeTarget.setVisible(false);
        tbMaxTargetToSpill.setVisible(false);

        fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withHeight(10));
        fbTerminatorControlInComp.items.add(juce::FlexItem{ fbTerminatorControlCloseComp }.withFlex(0.12f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{ isTargetAquiring ? albAcquiringTarget : albPlaybackSpill }.withFlex(0.22f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{ processor.termControlWaveform }.withFlex(0.46f));
        fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
    }
    else
    {
        if (showMaxToSpillWindow)
        {
            tbCloseTerminatorControl.setVisible(true);
            tbTerminateSpill.setVisible(false);
            tbMaximizeTarget.setVisible(false);
            tbMaxTargetToSpill.setVisible(false);
            albPlaybackSpill.setVisible(false);
            albAcquiringTarget.setVisible(false);
            processor.termControlWaveform.setVisible(false);

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
            processor.termControlWaveform.setVisible(false);
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

void PolarDesignerAudioProcessorEditor::buttonStateChanged(Button* button)
{
}

void PolarDesignerAudioProcessorEditor::buttonClicked (Button* button)
{
    if (presetListVisible)
    {
        showPresetList(false);
    }
    if ((button == &tmbNrBandsButton[0]) && (button->getToggleState() > 0.5f))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((0)));
    }
    if ((button == &tmbNrBandsButton[1]) && (button->getToggleState() > 0.5f))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((1)));
    }
    if ((button == &tmbNrBandsButton[2]) && (button->getToggleState() > 0.5f))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((2)));
    }
    if ((button == &tmbNrBandsButton[3]) && (button->getToggleState() > 0.5f))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((3)));
    }
    if ((button == &tmbNrBandsButton[4]) && (button->getToggleState() > 0.5f))
    {
        valueTreeState.getParameter("nrBands")->setValueNotifyingHost(valueTreeState.getParameter("nrBands")->convertTo0to1((4)));
    }
    
    if ((button == &tmbSyncChannelButton[0]) && (button->getToggleState() > 0.5f))
    {
        tmbSyncChannelButton.disableAllButtonsApartOf(0);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((1)));
    }
    if ((button == &tmbSyncChannelButton[1]) && (button->getToggleState() > 0.5f))
    {
        tmbSyncChannelButton.disableAllButtonsApartOf(1);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((2)));
    }
    if ((button == &tmbSyncChannelButton[2]) && (button->getToggleState() > 0.5f))
    {
        tmbSyncChannelButton.disableAllButtonsApartOf(2);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((3)));
    }
    if ((button == &tmbSyncChannelButton[3]) && (button->getToggleState() > 0.5f))
    {
        tmbSyncChannelButton.disableAllButtonsApartOf(3);
        valueTreeState.getParameter("syncChannel")->setValueNotifyingHost(valueTreeState.getParameter("syncChannel")->convertTo0to1((4)));
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
    }
    else if (button == &tbSave)
    {
        saveFile();
    }
    else if (button == &ibEqCtr[0])
    {
        processor.setEqState(0);
        ibEqCtr[1].setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else if (button == &ibEqCtr[1])
    {
        processor.setEqState(1);
        ibEqCtr[0].setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else if (button == &tbTerminateSpill)
    {
        showTerminatorAnimationWindow = true;
        showMaxToSpillWindow = false;
        maximizeTarget = false;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        albPlaybackSpill.startAnimation("PLAYBACK SPILL  ");
        resized();
    }
    else if (button == &tbCloseTerminatorControl)
    {
        showTerminatorAnimationWindow = false;
        showMaxToSpillWindow = false;
        maximizeTarget = false;
        maxTargetToSpillFlowStarted = false;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        setMainAreaEnabled(true);
        setSideAreaEnabled(true);
        resized();
    }
    else if (button == &tbMaximizeTarget)
    {
        showTerminatorAnimationWindow = true;
        showMaxToSpillWindow = false;
        maximizeTarget = true;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        albPlaybackSpill.startAnimation("PLAYBACK SOURCE  ");
        resized();
    }
    else if (button == &tbMaxTargetToSpill)
    {
        showTerminatorAnimationWindow = false;
        showMaxToSpillWindow = true;
        maxTargetToSpillFlowStarted = true;
        termStage = terminatorStage::TERMINATE;
        showActiveTerminatorStage(termStage);
        resized();
    }
    else if (button == &tbBeginTerminate)
    {
        showMaxToSpillWindow = false;
        showTerminatorAnimationWindow = true;
        termStage = terminatorStage::MAXIMIZE;
        showMaxToSpillWindow = false;
        showActiveTerminatorStage(terminatorStage::DISABLED);
        albPlaybackSpill.startAnimation("PLAYBACK SPILL  ");
        resized();
    }
    else if (button == &tbBeginMaximize)
    {
        showMaxToSpillWindow = false;
        termStage = terminatorStage::MAXTOSPILL;
        showActiveTerminatorStage(terminatorStage::MAXIMIZE);
        tbMaximizeTarget.triggerClick();
        resized();
    }
    else if (button == &tbApplyMaxTargetToSpill)
    {
        //TODO
        showTerminatorAnimationWindow = false;
        showMaxToSpillWindow = false;
        maxTargetToSpillFlowStarted = false;
        isTargetAquiring = false;
        setSideAreaEnabled(true);
        setMainAreaEnabled(true);
        processor.stopTracking(2);
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
    else if (button == &tbZeroDelay)
    {
        bool isToggled = button->getToggleState();
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
    }
    else if (button == &tmbABButton[0])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(0);
            button->setAlpha(getABButtonAlphaFromLayerState(isToggled));
            tmbABButton[1].setAlpha(getABButtonAlphaFromLayerState(!isToggled));
        }
    }
    else if (button == &tmbABButton[1])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(1);
            button->setAlpha(getABButtonAlphaFromLayerState(isToggled));
            tmbABButton[0].setAlpha(getABButtonAlphaFromLayerState(!isToggled));
        }
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
}

float PolarDesignerAudioProcessorEditor::getABButtonAlphaFromLayerState(int layerState)
{
    return layerState * 0.7f + 0.3f;
}

std::vector<float> PolarDesignerAudioProcessorEditor::getBandLimitWidthVector(float dirEqSize, float offsetPolVis)
{
    //First calculate bandLimit vector
    std::vector<float> bandLimit;
    bandLimit.push_back(0);
    for (int i = 0; i < nActiveBands - 1; i++)
    {
        bandLimit.push_back(directivityEqualiser.getBandlimitPathComponent(i).getX() - offsetPolVis);
    }
    bandLimit.push_back(dirEqSize);
    //Next calculate width of each band
    std::vector<float>::iterator it;
    int i = 1;
    std::vector<float> bandLimitWidth;
    bandLimitWidth.push_back(dirEqSize);
    for (it = bandLimit.begin() + 1; it != bandLimit.end(); it++, i++) {
        bandLimitWidth.push_back(bandLimit[i] - bandLimit[i - 1]);
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
                polarPatternVisualizers[i].setDirWeight(slider->getValue());
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
            setMainAreaEnabled(false);
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
            setMainAreaEnabled(false);
            setSideAreaEnabled(false);
        }
        else
        {
            lbUserPresets.AddNewPresetToList(presetFile.getFileNameWithoutExtension());
        }
    }
}

void PolarDesignerAudioProcessorEditor::loadSavedPresetsToList()
{
    File presetDir(processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory));
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

void PolarDesignerAudioProcessorEditor::nActiveBandsChanged()
{
    nActiveBands = processor.getNBands();
    for (int i = 0; i < 5; i++)
    {
        if (i < nActiveBands)
        {
            slDir[i].setEnabled(true);
            slBandGain[i].setEnabled(true);
            tgbSolo[i].setEnabled(true);
            tgbMute[i].setEnabled(true);
            polarPatternVisualizers[i].setActive(true);
            polarPatternVisualizers[i].setVisible(true);

            slDir[i].setVisible(true);
            slBandGain[i].setVisible(true);
            tgbSolo[i].setVisible(true);
            tgbMute[i].setVisible(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            tgbSolo[i].setEnabled(false);
            tgbSolo[i].setToggleState(false, NotificationType::sendNotification);
            tgbMute[i].setEnabled(false);
            tgbMute[i].setToggleState(false, NotificationType::sendNotification);
            polarPatternVisualizers[i].setActive(false);
            polarPatternVisualizers[i].setVisible(false);
            
            slDir[i].setVisible(false);
            slBandGain[i].setVisible(false);
            tgbSolo[i].setVisible(false);
            tgbMute[i].setVisible(false);
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
    if (processor.activeBandsChanged.get())
    {
        processor.activeBandsChanged = false;
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

    if (showTerminatorAnimationWindow)
    {
        if (processor.info.isPlaying)
        {
            if (!isTargetAquiring)
            {
                DBG("PLAYING");
                isTargetAquiring = true;
                setSideAreaEnabled(false);
                setMainAreaEnabled(false);
                processor.startTracking(maximizeTarget ? false : true);
                albAcquiringTarget.startAnimation("ACQUIRING TARGET   ", 
                                                  maximizeTarget ? "STOP PLAYBACK WHEN READY TO MAXIMIZE  " 
                                                  : "STOP PLAYBACK WHEN READY TO TERMINATE  ");
                albPlaybackSpill.stopAnimation();
                resized();
            }
        }
        else
        {
            if (isTargetAquiring)
            {
                isTargetAquiring = false;
                setSideAreaEnabled(true);
                setMainAreaEnabled(true);
                processor.stopTracking(1);
                albAcquiringTarget.stopAnimation();
                albPlaybackSpill.stopAnimation();

                if (maxTargetToSpillFlowStarted)
                {
                    showMaxToSpillWindow = true;
                    showTerminatorAnimationWindow = false;
                    showActiveTerminatorStage(termStage);
                }
                else
                {
                    showTerminatorAnimationWindow = false;
                }
                resized();
            }
        }
    }
    else
    {
        if (isTargetAquiring)
        {
            isTargetAquiring = false;
            setSideAreaEnabled(true);
            setMainAreaEnabled(true);
            processor.stopTracking(0);
            albAcquiringTarget.stopAnimation();
            albPlaybackSpill.stopAnimation();
            resized();
        }
    }
}

void PolarDesignerAudioProcessorEditor::zeroDelayModeChange()
{
    tbZeroDelay.setToggleState(processor.zeroDelayModeActive(), NotificationType::dontSendNotification);
    
    nActiveBands = processor.getNBands();
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
            slBandGain[i].setVisible(true);
            tgbSolo[i].setEnabled(true);
            tgbSolo[i].setVisible(true);
            tgbMute[i].setEnabled(true);
            tgbMute[i].setVisible(true);
            polarPatternVisualizers[i].setActive(true);
            polarPatternVisualizers[i].setVisible(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            slBandGain[i].setVisible(false);
            tgbSolo[i].setEnabled(false);
            tgbSolo[i].setToggleState(false, NotificationType::sendNotification);
            tgbSolo[i].setVisible(false);
            tgbMute[i].setEnabled(false);
            tgbMute[i].setToggleState(false, NotificationType::sendNotification);
            tgbMute[i].setVisible(false);
            polarPatternVisualizers[i].setActive(false);
            polarPatternVisualizers[i].setVisible(false);
        }
    }
    
    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
}

void PolarDesignerAudioProcessorEditor::showPresetList(bool shouldShow)
{
    presetListVisible = shouldShow;
    logoAA.setVisible(!shouldShow);
    titlePD.setVisible(!shouldShow);
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

void PolarDesignerAudioProcessorEditor::mouseDown(const MouseEvent& event)
{
    //Event for hiding preset panel when clicking outside
    if (presetListVisible && !presetArea.contains(event.mouseDownPosition))
    {
        showPresetList(false);
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
        File presetDir(processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory));

        auto selectedPreset = lbUserPresets.getSelectedPresetName();
        auto presetFile = presetDir.findChildFiles(File::findFiles, false, String(selectedPreset + ".json"));

        if (presetFile.size() == 1)
        {
            processor.loadPreset(presetFile.getFirst());
            titlePreset.setTitle(String("Preset: " + selectedPreset));
            showPresetList(false);
        }
    }
    else if (source == &lbFactoryPresets)
    {
        File presetDir(processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation(File::userHomeDirectory));

        auto selectedPreset = lbFactoryPresets.getSelectedPresetName();
        auto presetFile = presetDir.findChildFiles(File::findFiles, false, String(selectedPreset + ".json"));

        if (presetFile.size() == 1)
        {
            processor.loadPreset(presetFile.getFirst());
            titlePreset.setTitle(String("Preset: " + selectedPreset));
            showPresetList(false);
        }
    }
}

void PolarDesignerAudioProcessorEditor::setMainAreaEnabled(bool enable)
{
    directivityEqualiser.setActive(enable);
    directivityEqualiser.setEnabled(enable);

    for (int i = 0; i < nActiveBands; i++)
    {
        directivityEqualiser.getDirPathComponent(i).setEnabled(enable);
        slDir[i].setEnabled(enable);
        slBandGain[i].setEnabled(enable);
        tgbSolo[i].setEnabled(enable);
        tgbMute[i].setEnabled(enable);
        polarPatternVisualizers[i].setEnabled(enable);
        polarPatternVisualizers[i].setActive(enable);
        if (polarPatternVisualizers[i].getToggleState())
            polarPatternVisualizers[i].setToggleState(false, NotificationType::dontSendNotification);
    }
    tbZeroDelay.setEnabled(enable);
    titlePreset.setEnabled(enable);
    repaint();
}

void PolarDesignerAudioProcessorEditor::setSideAreaEnabled(bool set)
{
    tmbNrBandsButton[0].setEnabled(set);
    tmbNrBandsButton[1].setEnabled(set);
    tmbNrBandsButton[2].setEnabled(set);
    tmbNrBandsButton[3].setEnabled(set);
    tmbNrBandsButton[4].setEnabled(set);
    tmbSyncChannelButton[0].setEnabled(set);
    tmbSyncChannelButton[1].setEnabled(set);
    tmbSyncChannelButton[2].setEnabled(set);
    tmbSyncChannelButton[3].setEnabled(set);
    
    tbLoad.setEnabled(set);
    tbSave.setEnabled(set);
    ibEqCtr[0].setEnabled(set);
    ibEqCtr[1].setEnabled(set);
    tbAllowBackwardsPattern.setEnabled(set);
    tbTerminateSpill.setEnabled(set);
    tbMaximizeTarget.setEnabled(set);
    tbMaxTargetToSpill.setEnabled(set);
    tgbProxCtr.setEnabled(set);
    if (tgbProxCtr.getToggleState())
    {
        slProximity.setEnabled(set);
    }
    grpEq.setEnabled(set);
    grpPreset.setEnabled(set);
    grpProxComp.setEnabled(set);
    grpBands.setEnabled(set);
    grpSync.setEnabled(set);
    grpPresetList.setEnabled(set);
    repaint();
}

void PolarDesignerAudioProcessorEditor::setEqMode()
{
    int activeIdx = processor.getEqState();
    ibEqCtr[activeIdx].setToggleState(true, NotificationType::sendNotification);
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

