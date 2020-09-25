/*
 ==============================================================================
 PluginEditor.h
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/MuteSoloButton.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/DirSlider.h"
#include "../resources/customComponents/FirstOrderDirectivityVisualizer.h"
#include "../resources/customComponents/DirectivityEQ.h"
#include "../resources/customComponents/AlertOverlay.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
//==============================================================================
/**
*/
class PolarDesignerAudioProcessorEditor  : public AudioProcessorEditor, private Button::Listener,
                                     private ComboBox::Listener, private Slider::Listener, private Timer
{
public:
    PolarDesignerAudioProcessorEditor (PolarDesignerAudioProcessor&, AudioProcessorValueTreeState&);
    ~PolarDesignerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* button) override;
    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;
    void onAlOverlayErrorOkay();
    void onAlOverlayApplyPattern();
    void onAlOverlayCancelRecord();
    void onAlOverlayMaxSigToDist();
    void setEqMode();
    
    int getControlParameterIndex (Component& control) override;
        
private:
    static const int EDITOR_WIDTH = 990;
    static const int EDITOR_HEIGHT = 630;
    String presetFilename;
    String errorMessage;
        
    const int nBands = 5;
    int nActiveBands;
    int syncChannelIdx;
    
    bool loadingFile;
    bool recordingDisturber;
    
    TitleBar<AALogo, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    
    PolarDesignerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    TooltipWindow tooltipWindow;

    // Groups
    GroupComponent grpEq, grpPreset, grpDstC, grpProxComp, grpBands, grpSync;
    // Sliders
    ReverseSlider slBandGain[5], slXover[4], slProximity;
    DirSlider slDir[5];
    
    // Solo Buttons
    MuteSoloButton msbSolo[5], msbMute[5];
    // Text Buttons
    TextButton tbLoadFile, tbSaveFile, tbRecordDisturber, tbRecordSignal, tbZeroDelay, tbAbButton[2];
    // ToggleButtons
    ToggleButton tbEq[3], tbAllowBackwardsPattern;
    // Combox Boxes
    ComboBox cbSetNrBands, cbSyncChannel;
            
    // Pointers for value tree state
    std::unique_ptr<ReverseSlider::SliderAttachment> slBandGainAtt[5], slXoverAtt[4], slProximityAtt;
    std::unique_ptr<SliderAttachment> slDirAtt[5];
    std::unique_ptr<ButtonAttachment> msbSoloAtt[5], msbMuteAtt[5], tbAllowBackwardsPatternAtt, tbZeroDelayAtt;
    std::unique_ptr<ComboBoxAttachment> cbSetNrBandsAtt, cbSyncChannelAtt;
    
    DirectivityEQ dEQ;
    FirstOrderDirectivityVisualizer dv[5];
    AlertOverlay alOverlayError;
    AlertOverlay alOverlayDisturber;
    AlertOverlay alOverlaySignal;

    Path sideBorderPath;
    //==========================================================================
    void nActiveBandsChanged();
    void loadFile();
    void saveFile();
    void timerCallback() override;
    bool getSoloActive();
    void disableMainArea();
    void setSideAreaEnabled(bool set);
    void disableOverlay();
    void zeroDelayModeChange();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessorEditor)
};
