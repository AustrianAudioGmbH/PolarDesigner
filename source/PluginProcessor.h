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

#include "Constants.hpp"
#include "resources/Delay.h"

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <math.h>

// these params can be synced between plugin instances
struct ParamsToSync
{
    int nrActiveBands, ffDfEq;
    float xOverFreqs[MAX_NUM_EQS - 1], dirFactors[MAX_NUM_EQS], gains[MAX_NUM_EQS], proximity;
    bool solo[MAX_NUM_EQS], mute[MAX_NUM_EQS], allowBackwardsPattern, proximityOnOff,
        zeroLatencyMode, abLayer;
    bool paramsValid = false;
};

// use several channels to be syncable
struct SharedParams
{
    SharedParams()
    {
        for (int i = 0; i < 4; ++i) // provide 4 channels to sync params between plugin instances
            syncParams.add (ParamsToSync());
        instanceCount++;
    }
    juce::Array<ParamsToSync> syncParams;
    unsigned int instanceCount;
};

// the A/B compare button layers
enum
{
    COMPARE_LAYER_A = 0,
    COMPARE_LAYER_B = 1
};

enum eqBandStates : unsigned int
{
    BAND_1EQ = 1,
    BAND_2EQ = 2,
    BAND_3EQ = 3,
    BAND_4EQ = 4,
    BAND_5EQ = 5,
    BAND_ZLEQ = 6
};

//==============================================================================
/**
*/
class PolarDesignerAudioProcessor final : public juce::AudioProcessor,
                                          public juce::AudioProcessorValueTreeState::Listener,
                                          private juce::Timer

{
public:
    //==============================================================================
    PolarDesignerAudioProcessor();
    ~PolarDesignerAudioProcessor() override;

    void registerParameterListeners();

    // This is the ProTools PageFile for PolarDesigner3
    juce::String getPageFileName() const override { return "PolarDesigner3.xml"; }

    //==============================================================================
    void loadEqImpulseResponses();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    using AudioProcessor::processBlockBypassed;
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================

    void initializeDefaultState();
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    int getSyncChannelIdx();

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    //==============================================================================
    juce::Result loadPreset (const juce::File& presetFile);
    juce::Result savePreset (juce::File destination);

    void startTracking (bool trackDisturber);
    void stopTracking (int applyOptimalPattern);

    void setNProcessorBands (unsigned int numBands)
    {
        if (numBands >= 1 && numBands <= MAX_NUM_EQS)
        {
            nProcessorBands.store (numBands, std::memory_order_relaxed);
            // Update any internal state as needed
            recomputeAllFilterCoefficients = true;
            repaintDEQ.store (true, std::memory_order_relaxed);
        }
    }

    unsigned int getNProcessorBands() { return nProcessorBands.load (std::memory_order_relaxed); }

    float getXoverSliderRangeStart (int sliderNum);
    float getXoverSliderRangeEnd (int sliderNum);

    std::atomic<bool> repaintDEQ = true;
    std::atomic<bool> zeroLatencyModeChanged = true;
    std::array<std::atomic<bool>, MAX_NUM_EQS> recomputeFilterCoefficients;
    std::atomic<bool> recomputeAllFilterCoefficients;

    bool getDisturberRecorded() { return disturberRecorded; }
    bool getSignalRecorded() { return signalRecorded; }

    void changeABLayerState (int state);
    bool disturberRecorded;
    bool signalRecorded;

    bool abLayerState = COMPARE_LAYER_A;

    juce::Identifier saveTree = "save";
    juce::Identifier nodeA = "layerA";
    juce::Identifier nodeB = "layerB";
    juce::Identifier nodeParams = "vtsParams";
    juce::ValueTree layerA;
    juce::ValueTree layerB;
    juce::ValueTree saveStates;

    // when the A/B buttons are pressed, the proximity values are remembered
    std::atomic<float> oldProxDistance;
    std::atomic<float> oldProxDistanceA = 0;
    std::atomic<float> oldProxDistanceB = 0;

    // when the A/B Buttons are pressed, the prior nrBands state is remembered
    std::atomic<float> oldNrBands = 0;
    std::atomic<float> oldNrBandsA = MAX_NUM_EQS;
    std::atomic<float> oldNrBandsB = MAX_NUM_EQS;

    std::atomic<bool> abLayerChanged = false;
    float zeroLatencyModeA; // Zero Latency setting for Layer A
    float zeroLatencyModeB; // Zero Latency setting for Layer B

    juce::AudioVisualiserComponent termControlWaveform;

    juce::AudioPlayHead* audioPlayHead;
    juce::AudioPlayHead::PositionInfo playHeadPosition;

    // deprecated
    juce::AudioPlayHead::CurrentPositionInfo playHeadState;

    juce::UndoManager undoManager;

#if PERFETTO
    // perfetto
    MelatoninPerfetto tracingSession;
#endif

    bool zeroLatencyModeActive()
    {
        bool _zd = (zeroLatencyModePtr->load() > 0.5f);
        return (_zd);
    }

    void timerCallback() override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return vtsParams; }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessor)

    std::atomic<unsigned int> nProcessorBands;

    juce::AudioProcessorValueTreeState vtsParams;
    juce::SharedResourcePointer<SharedParams> sharedParams;

    // use odd FIR_LEN for even filter order (FIR_LEN = N+1)
    // (lowpass and highpass need even filter order to put a zero at f=0 and f=pi)
    int firLen = FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE;

    // free field / diffuse field eq
    juce::dsp::Convolution dfEqOmniConv;
    juce::dsp::Convolution dfEqEightConv;
    juce::dsp::Convolution ffEqOmniConv;
    juce::dsp::Convolution ffEqEightConv;

    // proximity compensation filter
    juce::dsp::IIR::Filter<float> proxCompIIR;

    // delay (in case of 1 active band)
    Delay delay;
    juce::AudioBuffer<float> delayBuffer;

    std::atomic<float>* nProcessorBandsPtr;
    std::atomic<float>* xOverFreqsPtr[MAX_NUM_EQS - 1];
    std::atomic<float>* dirFactorsPtr[MAX_NUM_EQS];
    float oldDirFactors[MAX_NUM_EQS];
    std::atomic<float>* bandGainsPtr[MAX_NUM_EQS];
    float oldBandGains[MAX_NUM_EQS];
    std::atomic<float>* syncChannelPtr;
    std::atomic<float>* ffDfEqPtr;
    std::atomic<float>* allowBackwardsPatternPtr;
    // !J! Note: allowBackwardsPatternPtr is being maintained, even though the UI for changing its value has been removed
    // in PolarDesigner3.  The reason for maintenance is for compatability purposes, even though it should ALWAYS be
    // set to true, from this point on.

    std::atomic<float>* proxDistancePtr;
    std::atomic<float>* proxOnOffPtr;

    std::atomic<float>* zeroLatencyModePtr;
    std::atomic<float>* soloBandPtr[MAX_NUM_EQS];
    std::atomic<float>* muteBandPtr[MAX_NUM_EQS];

    std::atomic<float>* trimPositionPtr;

    bool isBypassed;
    bool soloActive;
    bool loadingFile;
    std::atomic<bool> readingSharedParams;
    bool trackingActive;
    bool trackingDisturber;
    int nrBlocksRecorded;
    int ffDfEq;

    int eqLatency;

    float omniSqSumDist[MAX_NUM_EQS], eightSqSumDist[MAX_NUM_EQS], omniEightSumDist[MAX_NUM_EQS],
        omniSqSumSig[MAX_NUM_EQS], eightSqSumSig[MAX_NUM_EQS], omniEightSumSig[MAX_NUM_EQS];

    juce::AudioBuffer<float> filterBankBuffer; // holds filtered data, size: N_CH_IN*5
    juce::AudioBuffer<float> firFilterBuffer; // holds filter coefficients, size: 5
    juce::AudioBuffer<float> omniEightBuffer; // holds omni and fig-of-eight signals, size: 2
    std::array<juce::dsp::Convolution, 2 * MAX_NUM_EQS> convolvers;

    double currentSampleRate = 0.0f;
    double previousSampleRate = 0.0f;

    // This is intentionally set to match Pro Tools expectations ...
    int currentBlockSize = PD_DEFAULT_BLOCK_SIZE;
    juce::File lastDir;

    //==============================================================================
    void updateFirLen();
    void resizeBuffersIfNeeded();
    void resetXoverFreqs();
    void computeAllFilterCoefficients();
    void computeFilterCoefficients (unsigned int crossoverNr);
    void setProxCompCoefficients (float distance);
    void updateAllConvolvers();
    void updateConvolver (size_t convNr);

    void createOmniAndEightSignals (juce::AudioBuffer<float>& buffer);
    void createPolarPatterns (juce::AudioBuffer<float>& buffer);
    void trackSignalEnergy (int numSamples);
    void setMinimumDisturbancePattern();
    void setMaximumSignalPattern();
    void maximizeSigToDistRatio();
    void updateLatency();
    void recomputeFilterCoefficientsIfNeeded();
    void resetTrackingState();

    // Method to update A/B button states
    void updateABButtonState();

    //    // ValueTree::Listener override
    //    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property);

    // file handling
    const juce::String presetProperties[27] = {
        "nrActiveBands", "xOverF1",    "xOverF2",    "xOverF3",    "xOverF4", "dirFactor1",
        "dirFactor2",    "dirFactor3", "dirFactor4", "dirFactor5", "gain1",   "gain2",
        "gain3",         "gain4",      "gain5",      "solo1",      "solo2",   "solo3",
        "solo4",         "solo5",      "mute1",      "mute2",      "mute3",   "mute4",
        "mute5",         "ffDfEq",     "proximity"
    };
};
