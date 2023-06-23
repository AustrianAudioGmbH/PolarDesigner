/*
 ==============================================================================
 PluginEditor.h
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

#pragma once

//#define AA_DO_DEBUG_PATH


#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/lookAndFeel/MainLookAndFeel.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/MuteSoloButton.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/DirSlider.h"
#include "../resources/customComponents/PolarPatternVisualizer.h"
#include "../resources/customComponents/DirectivityEQ.h"
#include "../resources/customComponents/AlertOverlay.h"
#include "../resources/customComponents/EndlessSlider.h"
#include "../resources/customComponents/MultiTextButton.h"
#include "../resources/customComponents/PresetListBox.h"
#include "../resources/customComponents/AnimatedLabel.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class PolarDesignerAudioProcessorEditor  : public AudioProcessorEditor, private Button::Listener,
                                           private Slider::Listener, private Timer, public ChangeListener
{
public:
    PolarDesignerAudioProcessorEditor (PolarDesignerAudioProcessor&, AudioProcessorValueTreeState&);
    ~PolarDesignerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonStateChanged(Button* button) override;
    void buttonClicked (Button* button) override;
    void sliderValueChanged (Slider* slider) override;
    
    void onAlOverlayErrorOkay();
    void onAlOverlayApplyPattern();
    void onAlOverlayCancelRecord();
    void onAlOverlayMaxSigToDist();
    void setEqMode();
    float getABButtonAlphaFromLayerState(int layerState);
    // Helper method to calculate flex on the base of bandlimitPathComponents
    std::vector<float> getBandLimitWidthVector(float sizeDirectionalEQ, float offsetPolarVisualizer);
    
    void incrementTrim(int nBands);
    void decrementTrim(int nBands);

    int getControlParameterIndex (Component& control) override;

    void loadSavedPresetsToList();

    void changeListenerCallback(ChangeBroadcaster* source) override;
        
private:
    static const int EDITOR_WIDTH = 1194;
    static const int EDITOR_HEIGHT = 834;
    String presetFilename;
    String errorMessage;

    const int maxNumberBands = 5;
    int nActiveBands;
    int syncChannelIdx;
    int oldAbLayerState;

    bool loadingFile;
    bool recordingDisturber;
    bool presetListVisible;

    Colour eqColours[5];
 
    AALogo logoAA;
    TitleBarPDText titlePD;
    TitleBarTextLabel titleCompare, titlePreset;

    Footer footer;
    LaF globalLaF;
    MainLookAndFeel mainLaF;

    PolarDesignerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    TooltipWindow tooltipWindow;

    // Groups
    GroupComponent grpEq, grpPreset, grpDstC, grpProxComp, grpBands, grpSync, grpPresetList;
    // Sliders
    ReverseSlider slBandGain[5], slCrossoverPosition[4];
    DirSlider slDir[5];
    Slider slProximity;

    // a slider to use to 'trim' the EQ's
    EndlessSlider trimSlider;

    // Solo Buttons
    MuteSoloButton msbSolo[5], msbMute[5];
    // Text Buttons
    TextButton tbLoad, tbSave, tbTerminateSpill, tbMaximizeTarget, tbMaxTargetToSpill, tbZeroDelay, tbSyncChannel[5], tbOpenFromFile;
    // ToggleButtons
    ToggleButton tbAllowBackwardsPattern, tgbProxCtr;
    // ImageButtons
    TextButton ibEqCtr[2], tbClosePresetList, tbCloseTerminatorControl;

    TextMultiButton tmbABButton, tmbNrBandsButton, tmbSyncChannelButton;

    PresetListBox lbUserPresets, lbFactoryPresets;

    // Pointers for value tree state
    std::unique_ptr<ReverseSlider::SliderAttachment> slBandGainAtt[5], slCrossoverAtt[4];
    std::unique_ptr<SliderAttachment> slProximityAtt;
    std::unique_ptr<SliderAttachment> slDirAtt[5];
    std::unique_ptr<ButtonAttachment> msbSoloAtt[5], msbMuteAtt[5], tbAllowBackwardsPatternAtt, tbZeroDelayAtt, tgbProxCtrAtt;
    
    DirectivityEQ directivityEqualiser;
    PolarPatternVisualizer polarPatternVisualizers[5];
    AlertOverlay alOverlayError;
    //AlertOverlay alOverlayDisturber;
    AlertOverlay alOverlaySignal;

    Rectangle<float> presetArea;
    AnimatedLabel albPlaybackSpill, albAcquiringTarget;

    bool showTerminatorAnimationWindow;
    bool isTargetAquiring;
    bool showPlaybackSpill;

#ifdef AA_DO_DEBUG_PATH
    Path debugPath;
#endif
    
    //==========================================================================
    void nActiveBandsChanged();
    void loadFile();
    void saveFile();
    void timerCallback() override;
    bool getSoloActive();
    void setMainAreaEnabled(bool enable);
    void setSideAreaEnabled(bool set);
    void disableOverlay();
    void zeroDelayModeChange();
    void showPresetList(bool shouldShow);

    void mouseDown(const MouseEvent& event) override;
    
    OpenGLContext openGLContext;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessorEditor)
};
