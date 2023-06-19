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
    alOverlaySignal(AlertOverlay::Type::signalTracking),
    presetListVisible(false)
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
    titlePD.setPDTextColour(mainLaF.mainTextColor);
    addAndMakeVisible(&titleCompare);
    titleCompare.setTitle(String("Compare"));
    titleCompare.setFont(mainLaF.normalFont);
    titleCompare.setLabelTextColour(mainLaF.mainTextColor);

    addAndMakeVisible(&tmbABButton);
    tmbABButton.setButtonsNumber(2);

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
    titlePreset.setLabelTextColour(mainLaF.mainTextColor);

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
    /*
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
    */
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

    eqColours[0] = Colour(0xFDBA4949);
    eqColours[1] = Colour(0xFDBA6F49);
    eqColours[2] = Colour(0xFDBAAF49);
    eqColours[3] = Colour(0xFD8CBA49);
    eqColours[4] = Colour(0xFD49BA64);
    
    // directivity eq
    addAndMakeVisible (&directivityEqualiser);
    
    for (int i = 0; i < maxNumberBands; ++i)
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
        addAndMakeVisible (&polarPatternVisualizers[i]);
        polarPatternVisualizers[i].setActive(true);
        polarPatternVisualizers[i].setDirWeight (slDir[i].getValue());
        polarPatternVisualizers[i].setMuteSoloButtons (&msbSolo[i], &msbMute[i]);
        polarPatternVisualizers[i].setColour (eqColours[i]);

        // main directivity Equaliser section
        directivityEqualiser.addSliders (eqColours[i], &slDir[i], (i > 0) ? &slCrossoverPosition[i - 1] : nullptr, (i < maxNumberBands - 1) ? &slCrossoverPosition[i] : nullptr, &msbSolo[i], &msbMute[i], &slBandGain[i], &polarPatternVisualizers[i]);
        
        if (i == maxNumberBands - 1)
            break; // there is one slCrossoverPosition less than bands
        
        addAndMakeVisible (&slCrossoverPosition[i]);
        slCrossoverAtt[i] = std::unique_ptr<ReverseSlider::SliderAttachment>(new ReverseSlider::SliderAttachment (valueTreeState, "xOverF" + String(i+1), slCrossoverPosition[i]));
        slCrossoverPosition[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slCrossoverPosition[i].addListener(this);
        slCrossoverPosition[i].setVisible(false);
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
    
    addAndMakeVisible (&tbAllowBackwardsPattern);
    tbAllowBackwardsPatternAtt = std::unique_ptr<ButtonAttachment>(new ButtonAttachment (valueTreeState, "allowBackwardsPattern", tbAllowBackwardsPattern));
    tbAllowBackwardsPattern.setButtonText ("allow reverse patterns");
    tbAllowBackwardsPattern.addListener (this);

    directivityEqualiser.setSoloActive(getSoloActive());

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

    addAndMakeVisible(&lbFactoryPresets);
    lbFactoryPresets.setHeaderText("Factory Presets");

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
    
    trimSlider.sliderIncremented = [this] { incrementTrim(nActiveBands); };
    trimSlider.sliderDecremented = [this] { decrementTrim(nActiveBands); };
    
    nActiveBandsChanged();
    zeroDelayModeChange();
    
    addAndMakeVisible(&trimSlider);

    startTimer (30);
    
    setEqMode();
}

// Handle the trimSlider increment/decrement calls
void PolarDesignerAudioProcessorEditor::incrementTrim(int nBands) {
    for (int i = 0; i < nBands; i++)
    {
        slDir[i].setValue(slDir[i].getValue() + trimSlider.step);
    }
}

void PolarDesignerAudioProcessorEditor::decrementTrim(int nBands) {
    for (int i = 0; i < nBands; i++)
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
    topComponent.items.add(juce::FlexItem().withFlex(0.18f));
    topComponent.items.add(juce::FlexItem(titlePreset).withFlex(0.04f));
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

    for (int i = 0; i < 4; i++)
    {
        syncChannelComponent.items.add(juce::FlexItem(tbSyncChannel[i]).withFlex(radioButonsFlex));
        if (i < 4) syncChannelComponent.items.add(juce::FlexItem().withFlex(radioButonsSpaceFlex));
    }

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

    const float muteSoloComponentButtonsFlex = 0.14f;

    juce::FlexBox muteSoloComponent[5];
    for (int i = 0; i < 5; i++)
    {
        muteSoloComponent[i].flexDirection = FlexBox::Direction::row;
        muteSoloComponent[i].justifyContent = juce::FlexBox::JustifyContent::center;
        muteSoloComponent[i].alignContent = juce::FlexBox::AlignContent::center;
        muteSoloComponent[i].items.add(juce::FlexItem().withFlex(marginFlex));
        muteSoloComponent[i].items.add(juce::FlexItem(msbMute[i]).withFlex(muteSoloComponentButtonsFlex));
        muteSoloComponent[i].items.add(juce::FlexItem().withFlex(1.f - 2 * marginFlex - 2 * muteSoloComponentButtonsFlex));
        muteSoloComponent[i].items.add(juce::FlexItem(msbSolo[i]).withFlex(muteSoloComponentButtonsFlex));
        muteSoloComponent[i].items.add(juce::FlexItem().withFlex(marginFlex));
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

    const float middleComponentFlex = 0.05f;

    juce::FlexBox middleComponent;
    middleComponent.flexDirection = FlexBox::Direction::column;
    middleComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    middleComponent.alignContent = juce::FlexBox::AlignContent::center;
    middleComponent.items.add(juce::FlexItem(polarVisualizersComponent).withFlex(middleComponentFlex*4));
    middleComponent.items.add(juce::FlexItem().withFlex(marginFlex));
    middleComponent.items.add(juce::FlexItem(directivityEqualiser).withFlex(middleComponentFlex*10));
    middleComponent.items.add(juce::FlexItem().withFlex(marginFlex));
    middleComponent.items.add(juce::FlexItem(dirSlidersComponent).withFlex(middleComponentFlex));
    middleComponent.items.add(juce::FlexItem(muteSoloModule).withFlex(middleComponentFlex));
    middleComponent.items.add(juce::FlexItem(gainBandSlidersComponent).withFlex(middleComponentFlex));
    middleComponent.items.add(juce::FlexItem().withFlex(marginFlex));

    const float trimSliderComponentFlex = 0.5f;
    const float trimSliderComponentMarginOffset = 0.03f;

    juce::FlexBox trimSliderComponent;
    trimSliderComponent.flexDirection = FlexBox::Direction::column;
    trimSliderComponent.justifyContent = juce::FlexBox::JustifyContent::center;
    trimSliderComponent.alignContent = juce::FlexBox::AlignContent::center;
    trimSliderComponent.items.add(juce::FlexItem().withFlex(trimSliderComponentFlex/2 + trimSliderComponentMarginOffset));
    trimSliderComponent.items.add(juce::FlexItem(trimSlider).withFlex(trimSliderComponentFlex));
    trimSliderComponent.items.add(juce::FlexItem().withFlex(trimSliderComponentFlex/2 - trimSliderComponentMarginOffset));
    
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
    mainComponent.items.add(juce::FlexItem().withFlex(0.017f));
    mainComponent.items.add(juce::FlexItem(/*trimSliderComponent*/).withFlex(0.03f));
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

    //fb.performLayout(area);

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
    fbFields.items.add(juce::FlexItem{ ibEqCtr[0] }.withFlex(0.5f));
    fbFields.items.add(juce::FlexItem{ ibEqCtr[1] }.withFlex(0.5f));

    juce::FlexBox fbEqCtrInComp;
    fbEqCtrInComp.flexDirection = juce::FlexBox::Direction::column;
    fbEqCtrInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbEqCtrInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbEqCtrInComp.items.add(juce::FlexItem{ fbFields }.withFlex(1.f));

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
    fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.25f));
    fbTerminatorControlInComp.items.add(juce::FlexItem{ tbTerminateSpill }.withFlex(0.22f));
    fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
    fbTerminatorControlInComp.items.add(juce::FlexItem{ tbMaximizeTarget }.withFlex(0.22f));
    fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.01f));
    fbTerminatorControlInComp.items.add(juce::FlexItem{ tbMaxTargetToSpill }.withFlex(0.22f));
    fbTerminatorControlInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));

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

    juce::FlexBox fbPresetListSub2InComp;
    fbPresetListSub2InComp.flexDirection = juce::FlexBox::Direction::row;
    fbPresetListSub2InComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbPresetListSub2InComp.alignContent = juce::FlexBox::AlignContent::center;
    fbPresetListSub2InComp.items.add(juce::FlexItem{  }.withFlex(0.5f));
    fbPresetListSub2InComp.items.add(juce::FlexItem{ tbOpenFromFile }.withFlex(0.5f));

    juce::FlexBox fbPresetListInComp;
    fbPresetListInComp.flexDirection = juce::FlexBox::Direction::column;
    fbPresetListInComp.justifyContent = juce::FlexBox::JustifyContent::center;
    fbPresetListInComp.alignContent = juce::FlexBox::AlignContent::center;
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.04f));
    fbPresetListInComp.items.add(juce::FlexItem{ fbPresetListSub1InComp }.withFlex(0.03f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.013f));
    fbPresetListInComp.items.add(juce::FlexItem{ fbPresetListSub2InComp }.withFlex(0.04f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
    fbPresetListInComp.items.add(juce::FlexItem{ lbUserPresets }.withFlex(0.27f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.06f));
    fbPresetListInComp.items.add(juce::FlexItem{ lbFactoryPresets }.withFlex(0.27f));
    fbPresetListInComp.items.add(juce::FlexItem{  }.withFlex(0.217f));

    outerBounds = fbPresetListOutComp.items[0].currentBounds;
    inCompWidth = outerBounds.getWidth();
    fbPresetListInComp.performLayout(outerBounds.reduced(inCompWidth * 0.06f, 0));
    /*
    alOverlayError.setBounds (directivityEqualiser.getX() + 120, directivityEqualiser.getY() + 50, directivityEqualiser.getWidth() - 240, directivityEqualiser.getHeight() - 100);
    alOverlayDisturber.setBounds (directivityEqualiser.getX() + 120, directivityEqualiser.getY() + 50, directivityEqualiser.getWidth() - 240, directivityEqualiser.getHeight() - 100);
    alOverlaySignal.setBounds (directivityEqualiser.getX() + 120, directivityEqualiser.getY() + 50, directivityEqualiser.getWidth() - 240, directivityEqualiser.getHeight() - 100);
    */
}

void PolarDesignerAudioProcessorEditor::buttonStateChanged(Button* button)
{
}

void PolarDesignerAudioProcessorEditor::buttonClicked (Button* button)
{
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
            logoAA.setVisible(!logoAA.isVisible());
            titlePD.setVisible(!titlePD.isVisible());
        }
    }
    else if (button == &tbOpenFromFile)
    {
        loadFile();
    }
    else if (button == &tbClosePresetList)
    {
        showPresetList(false);
        logoAA.setVisible(!logoAA.isVisible());
        titlePD.setVisible(!titlePD.isVisible());
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
        processor.startTracking(true);
        alOverlayDisturber.enableRatioButton(processor.getSignalRecorded());
        alOverlayDisturber.setVisible(true);
        disableMainArea();
        setSideAreaEnabled(false);
    }
    else if (button == &tbMaximizeTarget)
    {
        processor.startTracking(false);
        alOverlaySignal.enableRatioButton(processor.getDisturberRecorded());
        alOverlaySignal.setVisible(true);
        disableMainArea();
        setSideAreaEnabled(false);
    }
    else if (button == &tbMaxTargetToSpill)
    {
         //TODO
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
        else
        {
            lbUserPresets.AddNewPresetToList(presetFile.getFileNameWithoutExtension());
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
            polarPatternVisualizers[i].setActive(true);
            polarPatternVisualizers[i].setVisible(true);

            slDir[i].setVisible(true);
            slBandGain[i].setVisible(true);
            msbSolo[i].setVisible(true);
            msbMute[i].setVisible(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            msbSolo[i].setEnabled(false);
            msbSolo[i].setToggleState(false, NotificationType::sendNotification);
            msbMute[i].setEnabled(false);
            msbMute[i].setToggleState(false, NotificationType::sendNotification);
            polarPatternVisualizers[i].setActive(false);
            polarPatternVisualizers[i].setVisible(false);
            
            slDir[i].setVisible(false);
            slBandGain[i].setVisible(false);
            msbSolo[i].setVisible(false);
            msbMute[i].setVisible(false);
        }
    }
    tbSyncChannel[0].setToggleState(true,  NotificationType::sendNotification);
    
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
}

void PolarDesignerAudioProcessorEditor::zeroDelayModeChange()
{
    tbZeroDelay.setToggleState(processor.zeroDelayModeActive(), NotificationType::dontSendNotification);
    
    nActiveBands = processor.getNBands(); //cbSetNrBands.getSelectedId();
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
            polarPatternVisualizers[i].setActive(true);
        }
        else
        {
            slDir[i].setEnabled(false);
            slBandGain[i].setEnabled(false);
            msbSolo[i].setEnabled(false);
            msbSolo[i].setToggleState(false, NotificationType::sendNotification);
            msbMute[i].setEnabled(false);
            msbMute[i].setToggleState(false, NotificationType::sendNotification);
            polarPatternVisualizers[i].setActive(false);
        }
    }
    
    directivityEqualiser.resetTooltipTexts();
    directivityEqualiser.repaint();
}

void PolarDesignerAudioProcessorEditor::showPresetList(bool shouldShow)
{
    presetListVisible = shouldShow;
    resized();
}

void PolarDesignerAudioProcessorEditor::disableMainArea()
{
    directivityEqualiser.setActive(false);
    for (int i = 0; i < nActiveBands; i++)
    {
        slDir[i].setEnabled(false);
        slBandGain[i].setEnabled(false);
        msbSolo[i].setEnabled(false);
        msbMute[i].setEnabled(false);
        polarPatternVisualizers[i].setActive(false);
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
    tmbNrBandsButton[0].setEnabled(set);
    tmbNrBandsButton[1].setEnabled(set);
    tmbNrBandsButton[2].setEnabled(set);
    tmbNrBandsButton[3].setEnabled(set);
    tmbNrBandsButton[4].setEnabled(set);
    tbSyncChannel[0].setEnabled(set);
    tbSyncChannel[1].setEnabled(set);
    tbSyncChannel[2].setEnabled(set);
    tbSyncChannel[3].setEnabled(set);
    tbSyncChannel[4].setEnabled(set);
    
    //    cbSetNrBands.setEnabled(set);
    //    cbSyncChannel.setEnabled(set);
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
}

void PolarDesignerAudioProcessorEditor::setEqMode()
{
    int activeIdx = processor.getEqState();
    ibEqCtr[activeIdx].setToggleState(true, NotificationType::sendNotification);
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

