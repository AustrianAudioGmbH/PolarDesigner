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

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include "BinaryData.h"


#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/lookAndFeel/MainLookAndFeel.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/DirSlider.h"
#include "../resources/customComponents/PolarPatternVisualizer.h"
#include "../resources/customComponents/DirectivityEQ.h"
#include "../resources/customComponents/AlertOverlay.h"
#include "../resources/customComponents/EndlessSlider.h"
#include "../resources/customComponents/MultiTextButton.h"
#include "../resources/customComponents/PresetListBox.h"
#include "../resources/customComponents/AnimatedLabel.h"
#include "../resources/customComponents/GainSlider.h"

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
    ~PolarDesignerAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void buttonStateChanged(Button* button) override;
    void buttonClicked (Button* button) override;
    void sliderValueChanged (Slider* slider) override;

    void setEqMode();
    void calculateLockedBands(int nBands, bool trimSliderIncr);
    float getABButtonAlphaFromLayerState(int layerState);
    // Helper method to calculate flex on the base of bandlimitPathComponents
    std::vector<float> getBandLimitWidthVector(float sizeDirectionalEQ, float offsetPolarVisualizer);

    //void incrementTrim(int nBands);
    void setTrimValue(int nBands);
    void resetTrim(int nBands);

    int getControlParameterIndex (Component& control) override;

    void loadSavedPresetsToList();

    void changeListenerCallback(ChangeBroadcaster* source) override;

private:
    static const int EDITOR_MIN_WIDTH = 1194;
    static const int EDITOR_MIN_HEIGHT = 834;
    static const int EDITOR_MAX_WIDTH = 2732;
    static const int EDITOR_MAX_HEIGHT = 2048;

    String presetFilename;
    String errorMessage;

    const int maxNumberBands = 5;
    int nActiveBands;
    int syncChannelIdx;
//    int oldAbLayerState;

    float trimSliderPrevPos = 0.22f;
    float minBandValueDistances[5];
    bool bandLockedOnMinMax[5] = { false, false, false, false, false };
    bool minBandValueDistancesSet = false;
    int maxIt = 0;

    bool loadingFile;
//    bool recordingDisturber;
    bool presetListVisible;
    bool presetLoaded = false;

    Colour eqColours[5];
 
    TextButton tbLogoAA;
    TitleBarTextLabel titleCompare, titlePreset;
    TextButton titlePresetUndoButton;

    Footer footer;
    LaF globalLaF;
    MainLookAndFeel mainLaF;

    PolarDesignerAudioProcessor& currentProcessor;
    AudioProcessorValueTreeState& valueTreeState;
    TooltipWindow tooltipWindow;

    // Groups
    GroupComponent grpEq, grpPreset, grpTerminatorControl, grpProxComp, grpBands, grpSync, grpPresetList;
    // Sliders
    ReverseSlider slCrossoverPosition[4];
    DirSlider slDir[5];
    Slider slProximity;
    GainSlider slBandGain[5];

    // a slider to use to 'trim' the EQ's
    EndlessSlider trimSlider;

    // Solo Buttons
    ToggleButton tgbSolo[5], tgbMute[5];
    // Text Buttons
    TextButton tbLoad, tbSave, tbTerminateSpill, tbMaximizeTarget, tbMaxTargetToSpill, tbZeroDelay, tbOpenFromFile;
    // ToggleButtons
    ToggleButton tbAllowBackwardsPattern, tgbProxCtr;
    // ImageButtons
    TextButton ibEqCtr[2], tbClosePresetList, tbCloseTerminatorControl, tbTrimSliderCenterPointer;

    TextMultiButton tmbABButton, tmbNrBandsButton, tmbSyncChannelButton;

    PresetListBox lbUserPresets, lbFactoryPresets;

    // Pointers for value tree state
    std::unique_ptr<ReverseSlider::SliderAttachment> slCrossoverAtt[4];
    std::unique_ptr<SliderAttachment> slBandGainAtt[5];
    std::unique_ptr<SliderAttachment> slProximityAtt;
    std::unique_ptr<SliderAttachment> slDirAtt[5];
    std::unique_ptr<ButtonAttachment> tgbSoloAtt[5], tgbMuteAtt[5], tbAllowBackwardsPatternAtt, tbZeroDelayAtt, tgbProxCtrAtt;
    
    DirectivityEQ directivityEqualiser;
    PolarPatternVisualizer polarPatternVisualizers[5];

    Rectangle<float> presetArea;
    AnimatedLabel albPlaybackSpill, albAcquiringTarget;

    TextButton terminatorLabelNr1, terminatorLabelSpillMain, terminatorLabelSpillSub;
    TextButton tbBeginTerminate;

    TextButton terminatorLabelNr2, terminatorLabelMaxMain, terminatorLabelMaxSub;
    TextButton tbBeginMaximize;

    TextButton terminatorLabelNr3, terminatorLabelMaxToSpillMain, terminatorLabelMaxToSpillSub;
    TextButton tbApplyMaxTargetToSpill;

    TextButton terminatorStageLine[8];

    bool showTerminatorAnimationWindow;
    bool isTargetAquiring;
    bool maximizeTarget;
    bool showMaxToSpillWindow;
    bool maxTargetToSpillFlowStarted;

    enum class terminatorStage
    {
        DISABLED = 0,
        TERMINATE = 1,
        MAXIMIZE = 2,
        MAXTOSPILL = 3
    } termStage;

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
    void zeroDelayModeChange();
    void showPresetList(bool shouldShow);
    void setBandEnabled(int bandNr, bool enable);

    void showActiveTerminatorStage(terminatorStage stage);
    void notifyPresetLabelChange();

    void mouseDown(const MouseEvent& event) override;
    
    OpenGLContext openGLContext;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessorEditor)
};
