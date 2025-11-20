/*
 ==============================================================================
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

#include "PluginProcessor.h"
#include "resources/customComponents/AnimatedLabel.h"
#include "resources/customComponents/DirSlider.h"
#include "resources/customComponents/DirectivityEQ.h"
#include "resources/customComponents/EndlessSlider.h"
#include "resources/customComponents/GainSlider.h"
#include "resources/customComponents/MultiTextButton.h"
#include "resources/customComponents/PolarPatternVisualizer.h"
#include "resources/customComponents/PresetListBox.h"
#include "resources/customComponents/ReverseSlider.h"
#include "resources/customComponents/TitleBar.h"
#include "resources/lookAndFeel/MainLookAndFeel.h"

#include <juce_audio_processors/juce_audio_processors.h>

#ifdef USE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class PolarDesignerAudioProcessorEditor : public AudioProcessorEditor,
                                          private Button::Listener,
                                          private Slider::Listener,
                                          private Timer,
                                          public ChangeListener,
                                          private ValueTree::Listener
{
private:
    //    LaF globalLaF;
    MainLookAndFeel mainLaF;
    TextButton tbLogoAA;

public:
    PolarDesignerAudioProcessorEditor (PolarDesignerAudioProcessor&, AudioProcessorValueTreeState&);
    ~PolarDesignerAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void buttonStateChanged (Button* button) override;
    void buttonClicked (Button* button) override;
    void sliderValueChanged (Slider* slider) override;

    void setEqMode();
    void calculateLockedBands (int nBands, bool trimSliderIncr);
    float getABButtonAlphaFromLayerState (int layerState);
    // Helper method to calculate flex on the base of bandlimitPathComponents
    std::vector<float> getBandLimitWidthVector (float sizeDirectionalEQ,
                                                float offsetPolarVisualizer);

    //void incrementTrim(int nProcessorBands);
    void setTrimValue (int nBands);
    void resetTrim (int nBands);

    int getControlParameterIndex (Component& control) override;

    void loadSavedPresetsToList();

    void changeListenerCallback (ChangeBroadcaster* source) override;

    void initializeSavedStates();

    void saveLayerState (int layer);
    void restoreLayerState (int layer);

    TooltipWindow sharedTooltipWindow;

private:
    // UI repainting is 'debounced' to avoid overwhelm
    bool repaintPending = false;
    double lastRepaintTime = 0.0;
    static constexpr double repaintDebounceInterval = 0.03;

    static const int EDITOR_MIN_WIDTH = 992; // 1194;
    static const int EDITOR_MIN_HEIGHT = 640; // 834;
    static const int EDITOR_MAX_WIDTH = 2732;
    static const int EDITOR_MAX_HEIGHT = 2048;

    String presetFilename;
    String errorMessage;

    static const int MAX_EDITOR_BANDS = 5;

    unsigned int nActiveBands;

    int syncChannelIdx;

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

    TitleBarTextLabel titleCompare, titlePreset;
    TextButton titlePresetUndoButton;

    Footer footer;

    PolarDesignerAudioProcessor& polarDesignerProcessor;
    AudioProcessorValueTreeState& valueTreeState;

    DirectivityEQ directivityEqualiser;
    PolarPatternVisualizer polarPatternVisualizers[5];

    // Groups
    GroupComponent grpEq, grpPreset, grpTerminatorControl, grpProxComp, grpBands, grpSync,
        grpPresetList;
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
    TextButton tbLoad, tbSave, tbTerminateSpill, tbMaximizeTarget, tbMaxTargetToSpill,
        tbZeroLatency, tbOpenFromFile;
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
    std::unique_ptr<ButtonAttachment> tgbSoloAtt[5], tgbMuteAtt[5], tbAllowBackwardsPatternAtt,
        tbZeroLatencyAtt, tgbProxCtrAtt;

    Rectangle<float> presetArea;
    AnimatedLabel albPlaybackSpill, albAcquiringTarget;

    TextButton terminatorLabelNr1, terminatorLabelSpillMain, terminatorLabelSpillSub;
    TextButton tbBeginTerminate;

    TextButton terminatorLabelNr2, terminatorLabelMaxMain, terminatorLabelMaxSub;
    TextButton tbBeginMaximize;

    TextButton terminatorLabelNr3, terminatorLabelMaxToSpillMain, terminatorLabelMaxToSpillSub;
    TextButton tbApplyMaxTargetToSpill;

    TextButton terminatorStageLine[8];

    /* Flags governing the display of the Terminator/Spill/Maximize UI */
    bool uiTerminatorAnimationWindowIsVisible;
    bool uiTargetAquisitionWindowIsVisible;
    bool uiMaxToSpillWindowIsVisible;
    bool uiMaximizeTargetWindowIsVisible;
    bool uiMaxTargetToSpillFlowStarted;

    enum class terminatorStage
    {
        DISABLED = 0,
        TERMINATE = 1,
        MAXIMIZE = 2,
        MAXTOSPILL = 3
    } termStage;

    struct LayerState
    {
        unsigned int nrBandsValue; // Changed from nrBands
        std::array<float, MAX_EDITOR_BANDS> dirValues;
        std::array<float, MAX_EDITOR_BANDS> bandGainValues;
        std::array<bool, MAX_EDITOR_BANDS> soloStates;
        std::array<bool, MAX_EDITOR_BANDS> muteStates;
        std::array<float, MAX_EDITOR_BANDS - 1> crossoverValues;
        bool proxCtrState;
        float proxValue;
        int eqState;
    };

    bool isRestoringState = false;

    std::array<LayerState, 2> savedStates; // One for each A/B layer
    std::array<bool, 2> isStateSaved = { false, false }; // Track if state is saved for each layer

#ifdef AA_DO_DEBUG_PATH
    Path debugPath;
#endif

    //==========================================================================
    void nEditorBandsChanged();
    void loadFile();
    void saveFile();
    void timerCallback() override;
    bool getSoloActive();
    void setMainAreaEnabled (bool enable);
    void activateMainUI (bool shouldBeActive);
    void activateEditingForZeroLatency();
    void showPresetList (bool shouldShow);
    void setBandEnabled (int bandNr, bool enable);

    void showActiveTerminatorStage (terminatorStage stage);
    void notifyPresetLabelChange();

    void mouseDown (const MouseEvent& event) override;

    void updateABButtonState();

#ifdef USE_MELATONIN_INSPECTOR
    #if defined(_MSC_VER)
        #pragma message("MELATONIN INSPECTOR IS CONFIGURED TO BE INCLUDED IN BUILD")
    #elif defined(__GNUC__) || defined(__clang__)
        #warning "MELATONIN INSPECTOR IS CONFIGURED TO BE INCLUDED IN BUILD"
    #endif
    melatonin::Inspector inspector { *this, true };
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessorEditor)
};
