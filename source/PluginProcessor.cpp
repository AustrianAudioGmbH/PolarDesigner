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

#include "PluginProcessor.h"
#include "Conversions.hpp"
#include "FilterCoefficients.hpp"
#include "PluginEditor.h"

/* We use versionHint of ParameterID from now on - rigorously! */
#define PD_PARAMETER_V1 1

/* Trim Slider */
#define PD_PARAMETER_V2 2

/* Alternative methods for performance optimization */

#define USE_NEW_UPDATELATENCY

/* Debug State Information dumps */
// #define USE_EXTRA_DEBUG_DUMPS

static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout (PolarDesignerAudioProcessor& processor)
{
    using namespace juce;

    using APF = AudioParameterFloat;
    using APB = AudioParameterBool;
    using API = AudioParameterInt;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<APF> (
        ParameterID { "trimPosition", PD_PARAMETER_V2 },
        "trimPosition",
        NormalisableRange<float> (0.0f, 1.0f, 0.0001f),
        0.0f,
        AudioParameterFloatAttributes()
            .withLabel ("Trim")
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction (
                [&] (float value, [[maybe_unused]] int maximumStringLength)
                {
                    return String (std::roundf (hzFromZeroToOne (MAX_NUM_EQS, 0, value)))
                           + " Hz trimPot";
                })
            .withAutomatable (false)));

    for (size_t i = 0; i < MAX_NUM_EQS - 1; ++i)
        layout.add (std::make_unique<APF> (
            ParameterID { "xOverF" + String (i + 1), PD_PARAMETER_V1 },
            "Xover" + String (i + 1),
            NormalisableRange<float> (0.0f, 1.0f, 0.0001f),
            hzToZeroToOne (MAX_NUM_EQS, 0, INIT_XOVER_FREQS_5B[i]),
            AudioParameterFloatAttributes()
                .withLabel ("Hz")
                .withCategory (AudioProcessorParameter::genericParameter)
                .withStringFromValueFunction (
                    [&, i] (float value, [[maybe_unused]] int maximumStringLength)
                    {
                        return String (std::roundf (
                            hzFromZeroToOne (processor.getNProcessorBands(), i, value)));
                    })));

    for (size_t i = 1; i < MAX_NUM_EQS + 1; ++i)
        layout.add (std::make_unique<APF> (
            ParameterID { "alpha" + String (i), PD_PARAMETER_V1 },
            "Polar" + String (i),
            NormalisableRange<float> (-0.5f, 1.0f, 0.01f),
            0.0f,
            AudioParameterFloatAttributes()
                .withCategory (AudioProcessorParameter::genericParameter)
                .withStringFromValueFunction (
                    [] (float value, [[maybe_unused]] int maximumStringLength)
                    { return String (value, 2); })));

    for (size_t i = 1; i < MAX_NUM_EQS + 1; ++i)
        layout.add (
            std::make_unique<APB> (ParameterID { "solo" + String (i), PD_PARAMETER_V1 },
                                   "Solo" + String (i),
                                   false,
                                   AudioParameterBoolAttributes()
                                       .withCategory (AudioProcessorParameter::genericParameter)
                                       .withStringFromValueFunction (
                                           [] (bool value, [[maybe_unused]] int maximumStringLength)
                                           { return value ? "on" : "off"; })));

    for (size_t i = 1; i < MAX_NUM_EQS + 1; ++i)
        layout.add (
            std::make_unique<APB> (ParameterID { "mute" + String (i), PD_PARAMETER_V1 },
                                   "Mute" + String (i),
                                   false,
                                   AudioParameterBoolAttributes()
                                       .withCategory (AudioProcessorParameter::genericParameter)
                                       .withStringFromValueFunction (
                                           [] (bool value, [[maybe_unused]] int maximumStringLength)
                                           { return value ? "on" : "off"; })));

    for (size_t i = 1; i < MAX_NUM_EQS + 1; ++i)
        layout.add (std::make_unique<APF> (
            ParameterID { "gain" + String (i), PD_PARAMETER_V1 },
            "Gain" + String (i),
            NormalisableRange<float> (-24.0f, 18.0f, 0.1f),
            0.0f,
            AudioParameterFloatAttributes()
                .withLabel ("dB")
                .withCategory (AudioProcessorParameter::genericParameter)
                .withStringFromValueFunction (
                    [] (float value, [[maybe_unused]] int maximumStringLength)
                    { return String (value, 1); })));

    layout.add (std::make_unique<API> (
        ParameterID { "nrBands", PD_PARAMETER_V1 },
        "Nr. of Bands",
        0,
        4,
        4,
        AudioParameterIntAttributes()
            .withLabel ("")
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction ([] (int value, [[maybe_unused]] int maximumStringLength)
                                          { return String (value + 1); })
            .withAutomatable (false)));

    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { "allowBackwardsPattern", PD_PARAMETER_V1 },
        "Allow Reverse Patterns",
        true,
        AudioParameterBoolAttributes()
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction ([] (bool value, [[maybe_unused]] int maximumStringLength)
                                          { return value ? "on" : "off"; })
            .withAutomatable (false)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "proximity", PD_PARAMETER_V1 },
        "Proximity",
        NormalisableRange<float> (-1.0f, 1.0f, 0.001f),
        0.0f,
        AudioParameterFloatAttributes()
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction (
                [] (float value, [[maybe_unused]] int maximumStringLength)
                { return std::abs (value) < 0.05f ? "0.00" : String (value, 2); })));

    layout.add (std::make_unique<APB> (
        ParameterID { "proximityOnOff", PD_PARAMETER_V1 },
        "ProximityOnOff",
        false,
        AudioParameterBoolAttributes()
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction ([] (bool value, [[maybe_unused]] int maximumStringLength)
                                          { return value ? "on" : "off"; })));

    layout.add (std::make_unique<APB> (
        ParameterID { "zeroLatencyMode", PD_PARAMETER_V1 },
        "Zero Latency",
        false,
        AudioParameterBoolAttributes()
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction ([] (bool value, [[maybe_unused]] int maximumStringLength)
                                          { return value ? "on" : "off"; })));

    layout.add (std::make_unique<API> (
        ParameterID { "syncChannel", PD_PARAMETER_V1 },
        "Sync to Channel",
        0,
        4,
        0,
        AudioParameterIntAttributes()
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction ([] (int value, [[maybe_unused]] int maximumStringLength)
                                          { return value == 0 ? "none" : String (value); })
            .withAutomatable (false)));

    layout.add (std::make_unique<API> (
        ParameterID { "ffDfEq", PD_PARAMETER_V1 },
        "Free/diffuse field EQ",
        0,
        2,
        0,
        AudioParameterIntAttributes()
            .withCategory (AudioProcessorParameter::genericParameter)
            .withStringFromValueFunction ([] (int value, [[maybe_unused]] int maximumStringLength)
                                          { return value == 0 ? "none" : String (value); })
            .withAutomatable (false)));

    return layout;
}

//==============================================================================
PolarDesignerAudioProcessor::PolarDesignerAudioProcessor() :
    AudioProcessor (BusesProperties()
                        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    repaintDEQ (true),
    zeroLatencyModeChanged (true),
    recomputeAllFilterCoefficients (true),
    disturberRecorded (false),
    signalRecorded (false),
    abLayerState (COMPARE_LAYER_A),
    saveTree ("save"),
    nodeA ("layerA"),
    nodeB ("layerB"),
    nodeParams ("vtsParams"),
    layerA (nodeA),
    layerB (nodeB),
    saveStates (saveTree),
    oldProxDistance (0.0f),
    oldProxDistanceA (0.0f),
    oldProxDistanceB (0.0f),
    oldNrBands (0.0f),
    oldNrBandsA (MAX_NUM_EQS),
    oldNrBandsB (MAX_NUM_EQS),
    abLayerChanged (false),
    zeroLatencyModeA (0.0f),
    zeroLatencyModeB (0.0f),
    termControlWaveform (1),
    audioPlayHead (getPlayHead()),
    playHeadPosition(),
    playHeadState(), // Deprecated, but included as per declaration
    undoManager(),
    nProcessorBands (MAX_NUM_EQS),
    vtsParams (*this, &undoManager, "AAPolarDesigner", createParameterLayout (*this)),
    firLen (FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE),
    delay(),
    delayBuffer(),
    oldDirFactors { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    oldBandGains { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    isBypassed (false),
    soloActive (false),
    loadingFile (false),
    readingSharedParams (false),
    trackingActive (false),
    trackingDisturber (false),
    nrBlocksRecorded (0),
    omniSqSumDist { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    eightSqSumDist { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    omniEightSumDist { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    omniSqSumSig { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    eightSqSumSig { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    omniEightSumSig { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    filterBankBuffer(),
    firFilterBuffer(),
    omniEightBuffer(),
    convolvers {},
    lastDir()
{
    using namespace juce;

    if (firLen % 2 == 0)
        firLen++;
    jassert (firLen % 2 == 1); // Ensure firLen is odd

    trimPositionPtr = vtsParams.getRawParameterValue ("trimPosition");

    for (size_t i = 0; i < MAX_NUM_EQS - 1; ++i)
        xOverFreqsPtr[i] = vtsParams.getRawParameterValue ("xOverF" + String (i + 1));

    for (size_t i = 0; i < MAX_NUM_EQS; ++i)
    {
        dirFactorsPtr[i] = vtsParams.getRawParameterValue ("alpha" + String (i + 1));
        soloBandPtr[i] = vtsParams.getRawParameterValue ("solo" + String (i + 1));
        muteBandPtr[i] = vtsParams.getRawParameterValue ("mute" + String (i + 1));
        bandGainsPtr[i] = vtsParams.getRawParameterValue ("gain" + String (i + 1));
    }
    nProcessorBandsPtr = vtsParams.getRawParameterValue ("nrBands");
    proxDistancePtr = vtsParams.getRawParameterValue ("proximity");
    proxOnOffPtr = vtsParams.getRawParameterValue ("proximityOnOff");
    zeroLatencyModePtr = vtsParams.getRawParameterValue ("zeroLatencyMode");
    ffDfEqPtr = vtsParams.getRawParameterValue ("ffDfEq");
    syncChannelPtr = vtsParams.getRawParameterValue ("syncChannel");

    // properties file: saves user preset folder location
    PropertiesFile::Options options;
    options.applicationName = "PolarDesigner";
    options.filenameSuffix = "settings";
    options.folderName = "AustrianAudio";
    options.osxLibrarySubFolder = "Preferences";

    registerParameterListeners();

    //    delay.setDelayTime (std::ceilf (static_cast<float> (FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE) / 2 - 1) / FILTER_BANK_NATIVE_SAMPLE_RATE);
    delay.setDelayTime (static_cast<float> (
        static_cast<float> ((static_cast<float> (firLen) - 1.0f) / 2.0f) / currentSampleRate));

    oldProxDistance = proxDistancePtr->load();

    termControlWaveform.setRepaintRate (30);
    termControlWaveform.setBufferSize (256);

    updateLatency();

    resetTrackingState();

    startTimer (50);
}

void PolarDesignerAudioProcessor::registerParameterListeners()
{
    static const juce::StringArray params = {
        "trimPosition", "xOverF1",        "xOverF2",
        "xOverF3",      "xOverF4",        "alpha1",
        "alpha2",       "alpha3",         "alpha4",
        "alpha5",       "solo1",          "solo2",
        "solo3",        "solo4",          "solo5",
        "mute1",        "mute2",          "mute3",
        "mute4",        "mute5",          "gain1",
        "gain2",        "gain3",          "gain4",
        "gain5",        "nrBands",        "allowBackwardsPattern",
        "proximity",    "proximityOnOff", "zeroLatencyMode",
        "ffDfEq",       "syncChannel",
    };
    for (const auto& id : params)
    {
        vtsParams.addParameterListener (id, this);
    }
}

PolarDesignerAudioProcessor::~PolarDesignerAudioProcessor()
{
    // Remove listeners for parameters
    for (auto* param : getParameters())
    {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*> (param))
        {
            vtsParams.removeParameterListener (rangedParam->paramID, this);
        }
    }
}

//==============================================================================
const juce::String PolarDesignerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PolarDesignerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PolarDesignerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PolarDesignerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PolarDesignerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PolarDesignerAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PolarDesignerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PolarDesignerAudioProcessor::setCurrentProgram ([[maybe_unused]] int index)
{
}

const juce::String PolarDesignerAudioProcessor::getProgramName ([[maybe_unused]] int index)
{
    return {};
}

void PolarDesignerAudioProcessor::changeProgramName ([[maybe_unused]] int index,
                                                     [[maybe_unused]] const juce::String& newName)
{
}

void PolarDesignerAudioProcessor::loadEqImpulseResponses()
{
    using namespace juce;

    jassert (DF_EQ_LEN == FF_EQ_LEN);

    // nothing changed, return early without allocating memory
    if (approximatelyEqual (currentSampleRate, previousSampleRate))
        return;

    AudioBuffer<float> DFEqOmni;
    AudioBuffer<float> DFEqEight;
    AudioBuffer<float> FFEqOmni;
    AudioBuffer<float> FFEqEight;

    // we don't need to resample
    if (approximatelyEqual (currentSampleRate,
                            static_cast<double> (FILTER_BANK_NATIVE_SAMPLE_RATE)))
    {
        DFEqOmni.setSize (1, DF_EQ_LEN);
        DFEqEight.setSize (1, DF_EQ_LEN);
        FFEqOmni.setSize (1, FF_EQ_LEN);
        FFEqEight.setSize (1, FF_EQ_LEN);

        DFEqOmni.copyFrom (0, 0, DFEQ_COEFFS_OMNI, DF_EQ_LEN);
        DFEqEight.copyFrom (0, 0, DFEQ_COEFFS_EIGHT, DF_EQ_LEN);
        FFEqOmni.copyFrom (0, 0, FFEQ_COEFFS_OMNI, FF_EQ_LEN);
        FFEqEight.copyFrom (0, 0, FFEQ_COEFFS_EIGHT, FF_EQ_LEN);

        eqLatency = 0;
    }

    // we do need to resample
    else
    {
        WindowedSincInterpolator resampler;

        const auto ratio = FILTER_BANK_NATIVE_SAMPLE_RATE / currentSampleRate;
        const auto nSamplesRatio = currentSampleRate / FILTER_BANK_NATIVE_SAMPLE_RATE;
        const auto nSamplesAvailable = DF_EQ_LEN;
        const auto nSamplesToProduce = static_cast<int> (
            std::ceil ((nSamplesAvailable + 2 * resampler.getBaseLatency()) * nSamplesRatio));

        DFEqOmni.setSize (1, nSamplesToProduce);
        DFEqEight.setSize (1, nSamplesToProduce);
        FFEqOmni.setSize (1, nSamplesToProduce);
        FFEqEight.setSize (1, nSamplesToProduce);

        resampler.process (ratio,
                           DFEQ_COEFFS_OMNI,
                           DFEqOmni.getWritePointer (0),
                           nSamplesToProduce,
                           nSamplesAvailable,
                           0);
        resampler.reset(); // resampler is stateful, we need to reset it

        resampler.process (ratio,
                           DFEQ_COEFFS_EIGHT,
                           DFEqEight.getWritePointer (0),
                           nSamplesToProduce,
                           nSamplesAvailable,
                           0);
        resampler.reset();

        resampler.process (ratio,
                           FFEQ_COEFFS_OMNI,
                           FFEqOmni.getWritePointer (0),
                           nSamplesToProduce,
                           nSamplesAvailable,
                           0);
        resampler.reset();

        resampler.process (ratio,
                           FFEQ_COEFFS_EIGHT,
                           FFEqEight.getWritePointer (0),
                           nSamplesToProduce,
                           nSamplesAvailable,
                           0);

        // resampling requires a power correction
        {
            const auto r = static_cast<float> (ratio);
            DFEqOmni.applyGain (r);
            DFEqEight.applyGain (r);
            FFEqOmni.applyGain (r);
            FFEqEight.applyGain (r);
        }

        eqLatency = static_cast<int> (std::ceil (resampler.getBaseLatency() * nSamplesRatio));
    }

    dfEqOmniConv.loadImpulseResponse (std::move (DFEqOmni),
                                      currentSampleRate,
                                      dsp::Convolution::Stereo::no,
                                      dsp::Convolution::Trim::no,
                                      dsp::Convolution::Normalise::no);
    dfEqEightConv.loadImpulseResponse (std::move (DFEqEight),
                                       currentSampleRate,
                                       dsp::Convolution::Stereo::no,
                                       dsp::Convolution::Trim::no,
                                       dsp::Convolution::Normalise::no);
    ffEqOmniConv.loadImpulseResponse (std::move (FFEqOmni),
                                      currentSampleRate,
                                      dsp::Convolution::Stereo::no,
                                      dsp::Convolution::Trim::no,
                                      dsp::Convolution::Normalise::no);
    ffEqEightConv.loadImpulseResponse (std::move (FFEqEight),
                                       currentSampleRate,
                                       dsp::Convolution::Stereo::no,
                                       dsp::Convolution::Trim::no,
                                       dsp::Convolution::Normalise::no);
}

void PolarDesignerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    using namespace juce;
    using namespace dsp;

    jassert (FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE > 0);

    // Validate inputs
    currentSampleRate = sampleRate > 0 ? sampleRate : FILTER_BANK_NATIVE_SAMPLE_RATE;
    currentBlockSize = samplesPerBlock > 0 ? samplesPerBlock : PD_DEFAULT_BLOCK_SIZE;

    // calculate the FIR filter length
    updateFirLen();

    // Resize buffers
    resizeBuffersIfNeeded();
    jassert (firFilterBuffer.getNumSamples() > 0);

    // Load EQ and compute filter coefficients
    loadEqImpulseResponses();
    computeAllFilterCoefficients();

    // Configure ProcessSpec
    ProcessSpec spec { currentSampleRate, static_cast<uint32> (currentBlockSize), 1 };
    dfEqOmniConv.prepare (spec);
    dfEqEightConv.prepare (spec);
    ffEqOmniConv.prepare (spec);
    ffEqEightConv.prepare (spec);

    for (auto& conv : convolvers)
        conv.prepare (spec);

    // Configure delay line
    delay.prepare (spec);
    delay.setDelayTime (static_cast<float> (((firLen - 1) / 2.0) / currentSampleRate));

    // Update latency
    updateLatency();

    previousSampleRate = currentSampleRate;
}

bool PolarDesignerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    using namespace juce;

    if ((layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        || layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet().isDisabled())
        return false;

    if (layouts.getMainOutputChannelSet().isDisabled())
        return false;

    return true;
}

void PolarDesignerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    using namespace juce;
    using namespace dsp;

    ScopedNoDenormals noDenormals;
    if (isBypassed)
    {
        isBypassed = false;
        updateLatency();
    }

    auto numSamples = static_cast<size_t> (buffer.getNumSamples());
    createOmniAndEightSignals (buffer);

    // Proximity compensation
    auto proximity =
        approximatelyEqual (proxOnOffPtr->load(), 1.0f) ? proxDistancePtr->load() : 0.f;
    if (! approximatelyEqual (zeroLatencyModePtr->load(), 1.0f) && (proximity < -0.05))
    {
        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> eightBlock (&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> contextProxEight (eightBlock);
        proxCompIIR.process (contextProxEight);
    }
    else if (! approximatelyEqual (zeroLatencyModePtr->load(), 1.0f) && (proximity > 0.05))
    {
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> omniBlock (&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> contextProxOmni (omniBlock);
        proxCompIIR.process (contextProxOmni);
    }

    // EQ processing
    if (! approximatelyEqual (zeroLatencyModePtr->load (std::memory_order_relaxed), 1.0f))
    {
        const auto ffDfEq = static_cast<int> (ffDfEqPtr->load (std::memory_order_relaxed));

        if (ffDfEq == 1)
        {
            float* writePointerOmni = omniEightBuffer.getWritePointer (0);
            dsp::AudioBlock<float> ffEqOmniBlk (&writePointerOmni, 1, numSamples);
            dsp::ProcessContextReplacing<float> ffEqOmniCtx (ffEqOmniBlk);
            ffEqOmniConv.process (ffEqOmniCtx);

            float* writePointerEight = omniEightBuffer.getWritePointer (1);
            dsp::AudioBlock<float> ffEqEightBlk (&writePointerEight, 1, numSamples);
            dsp::ProcessContextReplacing<float> ffEqEightCtx (ffEqEightBlk);
            ffEqEightConv.process (ffEqEightCtx);
        }
        else if (ffDfEq == 2)
        {
            float* writePointerOmni = omniEightBuffer.getWritePointer (0);
            dsp::AudioBlock<float> dfEqOmniBlk (&writePointerOmni, 1, numSamples);
            dsp::ProcessContextReplacing<float> dfEqOmniCtx (dfEqOmniBlk);
            dfEqOmniConv.process (dfEqOmniCtx);

            float* writePointerEight = omniEightBuffer.getWritePointer (1);
            dsp::AudioBlock<float> dfEqEightBlk (&writePointerEight, 1, numSamples);
            dsp::ProcessContextReplacing<float> dfEqEightCtx (dfEqEightBlk);
            dfEqEightConv.process (dfEqEightCtx);
        }
    }

    auto nActiveBands = static_cast<int> (nProcessorBands);
    if (zeroLatencyModePtr->load() > 0.5f)
        nActiveBands = 1;

    // Copy input to filter bank
    for (auto i = 0; i < nActiveBands && 2 * i + 1 < static_cast<int> (convolvers.size()); ++i)
    {
        filterBankBuffer.copyFrom (2 * i, 0, omniEightBuffer, 0, 0, buffer.getNumSamples());
        filterBankBuffer.copyFrom (2 * i + 1, 0, omniEightBuffer, 1, 0, buffer.getNumSamples());
    }

    // Process filter bank convolvers
    if (zeroLatencyModePtr->load() < 0.5f && nActiveBands > 1)
    {
        recomputeFilterCoefficientsIfNeeded();

        for (unsigned int i = 0;
             i < static_cast<size_t> (nActiveBands) && 2 * i + 1 < convolvers.size();
             ++i)
        {
            float* writePointerOmni = filterBankBuffer.getWritePointer (2 * static_cast<int> (i));
            dsp::AudioBlock<float> subBlk (&writePointerOmni, 1, numSamples);
            dsp::ProcessContextReplacing<float> filterCtx (subBlk);
            convolvers[2 * i].process (filterCtx);

            float* writePointerEight =
                filterBankBuffer.getWritePointer (2 * static_cast<int> (i) + 1);
            dsp::AudioBlock<float> subBlk2 (&writePointerEight, 1, numSamples);
            dsp::ProcessContextReplacing<float> filterCtx2 (subBlk2);
            convolvers[2 * i + 1].process (filterCtx2);
        }
    }

    if (auto* playhead = getPlayHead())
    {
        if (auto position = playhead->getPosition())
        {
            playHeadPosition = *position;
        }
    }

    termControlWaveform.pushBuffer (buffer);
    if (trackingActive)
        trackSignalEnergy (buffer.getNumSamples());

    createPolarPatterns (buffer);
}

void PolarDesignerAudioProcessor::processBlockBypassed (
    juce::AudioBuffer<float>& buffer,
    [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    if (! isBypassed)
    {
        isBypassed = true;
        updateLatency();
    }

    jassert (getLatencySamples() == 0);

    for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

//==============================================================================
bool PolarDesignerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PolarDesignerAudioProcessor::createEditor()
{
    return new PolarDesignerAudioProcessorEditor (*this, vtsParams);
}

//==============================================================================

int PolarDesignerAudioProcessor::getSyncChannelIdx()
{
    return static_cast<int> (syncChannelPtr->load (std::memory_order_relaxed)) - 1;
}

// getStateInformation: Ensure consistent updates for layerA and layerB
void PolarDesignerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    using namespace juce;

    // Update vtsParams.state properties
    vtsParams.state.setProperty ("oldProxDistance", var (oldProxDistance), nullptr);
    vtsParams.state.setProperty ("ABLayer", abLayerState, nullptr);
    vtsParams.state.setProperty ("oldNrBands", var (oldNrBands), nullptr);

    // Update the appropriate layer based on abLayerState
    if (abLayerState == COMPARE_LAYER_A)
    {
        layerA = vtsParams.copyState();
        if (proxDistancePtr && ! exactlyEqual (std::round (proxDistancePtr->load()), 0.0f))
        {
            oldProxDistanceA = proxDistancePtr->load();
        }
        if (nProcessorBandsPtr)
        {
            oldNrBandsA = nProcessorBandsPtr->load();
        }
    }
    else if (abLayerState == COMPARE_LAYER_B)
    {
        layerB = vtsParams.copyState();
        if (proxDistancePtr && ! exactlyEqual (std::round (proxDistancePtr->load()), 0.0f))
        {
            oldProxDistanceB = proxDistancePtr->load();
        }
        if (nProcessorBandsPtr)
        {
            oldNrBandsB = nProcessorBandsPtr->load();
        }
    }

    // Update layer properties
    layerA.setProperty ("oldProxDistance", var (oldProxDistanceA), nullptr);
    layerA.setProperty ("ABLayer", COMPARE_LAYER_A, nullptr);
    layerA.setProperty ("oldNrBands", var (oldNrBandsA), nullptr);

    layerB.setProperty ("oldProxDistance", var (oldProxDistanceB), nullptr);
    layerB.setProperty ("ABLayer", COMPARE_LAYER_B, nullptr);
    layerB.setProperty ("oldNrBands", var (oldNrBandsB), nullptr);

    // Store all states in saveStates
    saveStates.removeAllChildren (nullptr);
    saveStates.addChild (vtsParams.copyState(), 0, nullptr);
    saveStates.addChild (layerA.createCopy(), 1, nullptr);
    saveStates.addChild (layerB.createCopy(), 2, nullptr);

    std::unique_ptr<XmlElement> xml (saveStates.createXml());
    copyXmlToBinary (*xml, destData);

#ifdef USE_EXTRA_DEBUG_DUMPS
    juce::String treeAsXmlString = saveStates.toXmlString();
    LOG_DEBUG (treeAsXmlString);
#endif
}

// !J! Make setStateInformation more robust for ProTools
void PolarDesignerAudioProcessor::initializeDefaultState()
{
    using namespace juce;

    // Reinitialize ValueTree state
    vtsParams.state = ValueTree ("AAPolarDesigner");
    layerA = vtsParams.copyState();
    layerB = vtsParams.copyState();
    abLayerState = COMPARE_LAYER_A;

    // Reset non-parameter state variables
    oldProxDistance = oldProxDistanceA = oldProxDistanceB = 0.0f;
    oldNrBands = oldNrBandsA = oldNrBandsB = 4.0f;
    nProcessorBands = static_cast<unsigned int> (nProcessorBandsPtr ? nProcessorBandsPtr->load() + 1
                                                                    : MAX_NUM_EQS);
    soloActive = false;
    loadingFile = false;
    readingSharedParams = false;

    recomputeAllFilterCoefficients = true;

    for (auto& flag : recomputeFilterCoefficients)
        flag = false;
    zeroLatencyModeChanged = true;
    repaintDEQ.store (true, std::memory_order_relaxed);

    // Reset tracking state
    resetTrackingState();

    // Reset parameters to their default values using stored defaults
    // Define default values based on vtsParams initialization
    // Reset parameters to their default values using stored defaults
    // Define default values based on vtsParams initialization
    std::map<String, float> defaultValues = {
        { "trimPosition", 0.0f }, // Default from constructor: 0.0f
        { "xOverF1",
          hzToZeroToOne (
              nProcessorBands,
              0,
              INIT_XOVER_FREQS_5B
                  [0]) }, // Band 1: Matches constructor, assuming INIT_XOVER_FREQS_5B[0] = 100 Hz
        { "xOverF2",
          hzToZeroToOne (
              nProcessorBands,
              1,
              INIT_XOVER_FREQS_5B
                  [1]) }, // Band 2: Matches constructor, assuming INIT_XOVER_FREQS_5B[1] = 315 Hz
        { "xOverF3",
          hzToZeroToOne (
              nProcessorBands,
              2,
              INIT_XOVER_FREQS_5B
                  [2]) }, // Band 3: Matches constructor, assuming INIT_XOVER_FREQS_5B[2] = 1000 Hz
        { "xOverF4",
          hzToZeroToOne (
              nProcessorBands,
              3,
              INIT_XOVER_FREQS_5B
                  [3]) }, // Band 4: Matches constructor, assuming INIT_XOVER_FREQS_5B[3] = 3150 Hz
        { "alpha1", 0.0f }, // Default from constructor: 0.0f (Cardioid)
        { "alpha2", 0.0f }, // Default from constructor: 0.0f
        { "alpha3", 0.0f }, // Default from constructor: 0.0f
        { "alpha4", 0.0f }, // Default from constructor: 0.0f
        { "alpha5", 0.0f }, // Default from constructor: 0.0f
        { "solo1", 0.0f }, // Default from constructor: false (0.0f)
        { "solo2", 0.0f }, // Default from constructor: false (0.0f)
        { "solo3", 0.0f }, // Default from constructor: false (0.0f)
        { "solo4", 0.0f }, // Default from constructor: false (0.0f)
        { "solo5", 0.0f }, // Default from constructor: false (0.0f)
        { "mute1", 0.0f }, // Default from constructor: false (0.0f)
        { "mute2", 0.0f }, // Default from constructor: false (0.0f)
        { "mute3", 0.0f }, // Default from constructor: false (0.0f)
        { "mute4", 0.0f }, // Default from constructor: false (0.0f)
        { "mute5", 0.0f }, // Default from constructor: false (0.0f)
        { "gain1", 0.0f }, // Default from constructor: 0.0f (0 dB)
        { "gain2", 0.0f }, // Default from constructor: 0.0f
        { "gain3", 0.0f }, // Default from constructor: 0.0f
        { "gain4", 0.0f }, // Default from constructor: 0.0f
        { "gain5", 0.0f }, // Default from constructor: 0.0f
        { "nrBands", 4.0f }, // Default from constructor: 4 (5 bands, 0-based index)
        { "allowBackwardsPattern", 1.0f }, // Default from constructor: false (0.0f)
        { "proximity", 0.0f }, // Default from constructor: 0.0f
        { "proximityOnOff", 0.0f }, // Default from constructor: false (0.0f)
        { "zeroLatencyMode", 0.0f }, // Default from constructor: false (0.0f)
        { "ffDfEq", 0.0f }, // Default from constructor: 0 (none)
        { "syncChannel", 0.0f } // Default from constructor: 0
    };

    // Apply default values to parameters
    for (const auto& [paramID, defaultValue] : defaultValues)
    {
        if (auto* param = vtsParams.getParameter (paramID))
        {
            param->setValueNotifyingHost (defaultValue);
#ifdef USE_EXTRA_DEBUG_DUMPS
            LOG_DEBUG ("Reset parameter " + paramID + " to " + String (defaultValue));
#endif
        }
#ifdef USE_EXTRA_DEBUG_DUMPS
        else
        {
            LOG_DEBUG ("Parameter not found: " + paramID);
        }
#endif
    }

    // Initialize buffers (ensure compatibility with ProTools)
    if (currentBlockSize == 0)
        currentBlockSize = PD_DEFAULT_BLOCK_SIZE; // Default block size

    if (juce::approximatelyEqual (currentSampleRate, 0.0))
        currentSampleRate = FILTER_BANK_NATIVE_SAMPLE_RATE; // Default sample rate

    filterBankBuffer.setSize (N_CH_IN * MAX_NUM_EQS, currentBlockSize, false, false, true);
    firFilterBuffer.setSize (MAX_NUM_EQS, firLen, false, false, true);
    omniEightBuffer.setSize (MAX_NUM_INPUTS, currentBlockSize, false, false, true);

    for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
    {
        oldDirFactors[i] = 0.0f; // Default to cardioid
        oldBandGains[i] = 0.0f; // Default to 0 dB
    }

    // Always true:
    vtsParams.getParameter ("allowBackwardsPattern")->setValueNotifyingHost (1.0f);
}
void PolarDesignerAudioProcessor::resizeBuffersIfNeeded()
{
    using namespace juce;

    // Resize firFilterBuffer if length differs or is uninitialized
    if (firFilterBuffer.getNumSamples() != firLen)
    {
        firFilterBuffer.setSize (MAX_NUM_EQS, firLen, false, false, true);
        firFilterBuffer.clear();
    }

    // Resize filterBankBuffer if channels/samples differ or is uninitialized
    if (filterBankBuffer.getNumSamples() != currentBlockSize)
    {
        filterBankBuffer.setSize (N_CH_IN * MAX_NUM_EQS, currentBlockSize, false, false, true);
        filterBankBuffer.clear();
    }

    // Resize omniEightBuffer if channels/samples differ or is uninitialized
    if (omniEightBuffer.getNumSamples() != currentBlockSize)
    {
        omniEightBuffer.setSize (MAX_NUM_INPUTS, currentBlockSize, false, false, true);
        omniEightBuffer.clear();
    }

    // Resize delayBuffer if channels/samples differ or is uninitialized
    if (delayBuffer.getNumSamples() != currentBlockSize)
    {
        delayBuffer.setSize (1, currentBlockSize, false, false, true);
        delayBuffer.clear();
    }
}

void PolarDesignerAudioProcessor::releaseResources()
{
    resetTrackingState();
    for (auto& conv : convolvers)
    {
        conv.reset();
    }
    dfEqOmniConv.reset();
    dfEqEightConv.reset();
    ffEqOmniConv.reset();
    ffEqEightConv.reset();
    proxCompIIR.reset();

    filterBankBuffer.clear();
    firFilterBuffer.clear();
    omniEightBuffer.clear();
}

void PolarDesignerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    using namespace juce;

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (! xmlState)
    {
        LOG_ERROR ("Invalid state data");
        initializeDefaultState();
        return;
    }

    resetTrackingState();

    if (xmlState->hasTagName (saveStates.getType()))
    {
        saveStates = ValueTree::fromXml (*xmlState);
        if (saveStates.getNumChildren() >= 3)
        {
            layerA = saveStates.getChild (1).createCopy();
            layerB = saveStates.getChild (2).createCopy();
            ValueTree tempState = saveStates.getChild (0);
            if (tempState.hasProperty ("ABLayer"))
            {
                Value val = tempState.getPropertyAsValue ("ABLayer", nullptr);
                if (val.getValue().toString() != "")
                {
                    abLayerState = static_cast<int> (val.getValue());
                    if (abLayerState != COMPARE_LAYER_A && abLayerState != COMPARE_LAYER_B)
                        abLayerState = COMPARE_LAYER_A;
                }
            }
            vtsParams.replaceState (abLayerState == COMPARE_LAYER_B ? layerB : layerA);
        }
        else
        {
            LOG_ERROR ("Incomplete saveStates, falling back");
            vtsParams.replaceState (saveStates.getChild (0));
            layerA = vtsParams.copyState();
            layerB = vtsParams.copyState();
            abLayerState = COMPARE_LAYER_A;
        }
    }
    else if (xmlState->hasTagName (vtsParams.state.getType()))
    {
        vtsParams.state = ValueTree::fromXml (*xmlState);
        layerA = vtsParams.copyState();
        layerB = vtsParams.copyState();
        if (vtsParams.state.hasProperty ("ABLayer"))
        {
            Value val = vtsParams.state.getPropertyAsValue ("ABLayer", nullptr);
            if (val.getValue().toString() != "")
            {
                abLayerState = static_cast<int> (val.getValue());
                if (abLayerState != COMPARE_LAYER_A && abLayerState != COMPARE_LAYER_B)
                    abLayerState = COMPARE_LAYER_A;
            }
        }
    }
    else
    {
        LOG_ERROR ("Unknown state format");
        initializeDefaultState();
        return;
    }

    // Restore non-parameter state
    if (vtsParams.state.hasProperty ("oldProxDistance"))
    {
        Value val = vtsParams.state.getPropertyAsValue ("oldProxDistance", nullptr);
        if (val.getValue().toString() != "")
        {
            oldProxDistance = static_cast<float> (val.getValue());
        }
    }
    if (vtsParams.state.hasProperty ("allowBackwardsPattern"))
    {
        Value val = vtsParams.state.getPropertyAsValue ("allowBackwardsPattern", nullptr);
        if (val.getValue().toString() != "")
        {
            vtsParams.getParameter ("allowBackwardsPattern")
                ->setValueNotifyingHost (val.getValue() ? 1.0f : 0.0f);
        }
        else
        {
            vtsParams.getParameter ("allowBackwardsPattern")->setValueNotifyingHost (1.0f);
        }
    }

    auto* layerAPtr = &layerA;
    auto* layerBPtr = &layerB;

    // Restore layer properties
    for (auto& layer : { layerAPtr, layerBPtr })
    {
        if (layer->hasProperty ("oldProxDistance"))
        {
            Value val = layer->getPropertyAsValue ("oldProxDistance", nullptr);
            if (val.getValue().toString() != "")
            {
                if (layer == &layerA)
                    oldProxDistanceA = static_cast<float> (val.getValue());
                else
                    oldProxDistanceB = static_cast<float> (val.getValue());
            }
        }
        if (layer->hasProperty ("oldNrBands"))
        {
            Value val = layer->getPropertyAsValue ("oldNrBands", nullptr);
            if (val.getValue().toString() != "")
            {
                if (layer == &layerA)
                    oldNrBandsA = static_cast<float> (val.getValue());
                else
                    oldNrBandsB = static_cast<float> (val.getValue());
            }
        }
    }

    if (nProcessorBandsPtr)
    {
        nProcessorBands.store (
            std::min (
                static_cast<unsigned int> (
                    static_cast<int> (nProcessorBandsPtr->load (std::memory_order_relaxed)) + 1),
                static_cast<unsigned int> (MAX_NUM_EQS)),
            std::memory_order_relaxed);
        if (abLayerState == COMPARE_LAYER_A)
            oldNrBandsA.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                               std::memory_order_relaxed);
        else
            oldNrBandsB.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                               std::memory_order_relaxed);
        oldNrBands.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                          std::memory_order_relaxed);
    }

    zeroLatencyModeChanged.store (true, std::memory_order_relaxed);
    repaintDEQ.store (true, std::memory_order_relaxed);
}

void PolarDesignerAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    using namespace juce;

    if (parameterID.startsWith ("trimPosition"))
    {
        // TODO: update trimSlider position according to automation
    }
    else if (parameterID == "nrBands")
    {
        if (nProcessorBandsPtr)
        {
            const auto newBands = static_cast<unsigned int> (newValue + 1);
            setNProcessorBands (newBands);
            recomputeAllFilterCoefficients.store (true, std::memory_order_relaxed);
        }
    }
    else if (parameterID.startsWith ("xOverF") && ! loadingFile)
    {
        unsigned int idx = static_cast<unsigned int> (parameterID.getTrailingIntValue() - 1);
        recomputeFilterCoefficients[idx].store (true, std::memory_order_relaxed);
    }
    else if (parameterID.startsWith ("solo"))
    {
        soloActive = false;
        for (unsigned int i = 0; i < nProcessorBands; ++i)
        {
            // CHANGED: Replaced std::round(soloBandPtr[i]->load()) >= 0.5 with juce::approximatelyEqual(soloBandPtr[i]->load(), 1.0f)
            if (juce::approximatelyEqual (soloBandPtr[i]->load(), 1.0f))
                soloActive = true;
        }
    }
    else if (parameterID.startsWith ("alpha"))
    {
        repaintDEQ.store (true, std::memory_order_relaxed);
    }
    else if (parameterID == "proximity")
    {
        setProxCompCoefficients (proxDistancePtr->load (std::memory_order_relaxed));
    }
    else if (parameterID == "zeroLatencyMode")
    {
        updateLatency();
        if (newValue < 0.5f)
        {
            // Zero Latency Mode turned off
            if (abLayerState == COMPARE_LAYER_B)
            {
                vtsParams.getParameter ("proximity")
                    ->setValueNotifyingHost (
                        vtsParams.getParameter ("proximity")
                            ->convertTo0to1 (oldProxDistanceB.load (std::memory_order_relaxed)));
                vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
                    vtsParams.getParameter ("nrBands")->convertTo0to1 (
                        oldNrBandsB.load (std::memory_order_relaxed)));
            }
            else
            {
                vtsParams.getParameter ("proximity")
                    ->setValueNotifyingHost (
                        vtsParams.getParameter ("proximity")
                            ->convertTo0to1 (oldProxDistanceA.load (std::memory_order_relaxed)));
                vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
                    vtsParams.getParameter ("nrBands")->convertTo0to1 (
                        oldNrBandsA.load (std::memory_order_relaxed)));
            }
            recomputeAllFilterCoefficients.store (true, std::memory_order_relaxed);
            zeroLatencyModeChanged.store (true, std::memory_order_relaxed);
        }
        else
        {
            // Zero Latency Mode turned on
            if (! abLayerChanged.load (std::memory_order_relaxed))
            {
                if (abLayerState == COMPARE_LAYER_B)
                {
                    oldProxDistanceB.store (proxDistancePtr->load (std::memory_order_relaxed),
                                            std::memory_order_relaxed);
                    oldNrBandsB.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                                       std::memory_order_relaxed);
                }
                else
                {
                    oldProxDistanceA.store (proxDistancePtr->load (std::memory_order_relaxed),
                                            std::memory_order_relaxed);
                    oldNrBandsA.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                                       std::memory_order_relaxed);
                }
            }
            vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
                vtsParams.getParameter ("nrBands")->convertTo0to1 (0));

            vtsParams.getParameter ("proximity")
                ->setValueNotifyingHost (vtsParams.getParameter ("proximity")->convertTo0to1 (0));

            zeroLatencyModeChanged.store (true, std::memory_order_relaxed);
        }
        if (zeroLatencyModeChanged.load (std::memory_order_relaxed)
            && zeroLatencyModePtr->load (std::memory_order_relaxed) > 0.5f)
        {
            vtsParams.state.setProperty ("oldZeroLatencyMode",
                                         var (zeroLatencyModePtr->load (std::memory_order_relaxed)),
                                         nullptr);
            zeroLatencyModeChanged.store (true, std::memory_order_relaxed);
        }
    }
    else if (parameterID == "syncChannel")
    {
        const int ch = static_cast<int> (syncChannelPtr->load (std::memory_order_relaxed)) - 1;

        if (ch >= 0)
        {
            ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference (ch);

            if (! paramsToSync.paramsValid) // Initialize all params
            {
                for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
                {
                    paramsToSync.solo[i] = soloBandPtr[i] && soloBandPtr[i]->load() > 0.5f;
                    paramsToSync.mute[i] = muteBandPtr[i] && muteBandPtr[i]->load() > 0.5f;

                    paramsToSync.dirFactors[i] = dirFactorsPtr[i] ? dirFactorsPtr[i]->load() : 0.0f;
                    paramsToSync.gains[i] = bandGainsPtr[i] ? bandGainsPtr[i]->load() : 0.0f;

                    if (i < MAX_NUM_EQS - 1)
                    {
                        paramsToSync.xOverFreqs[i] =
                            xOverFreqsPtr[i] ? xOverFreqsPtr[i]->load() : 0.0f;
                    }
                }

                paramsToSync.nrActiveBands =
                    nProcessorBandsPtr ? static_cast<int> (nProcessorBandsPtr->load()) : 0;
                paramsToSync.proximity = proxDistancePtr ? proxDistancePtr->load() : 0.0f;
                paramsToSync.proximityOnOff =
                    proxOnOffPtr && juce::approximatelyEqual (proxOnOffPtr->load(), 1.0f);

                paramsToSync.allowBackwardsPattern = true; // !J! ALWAYS TRUE

                paramsToSync.zeroLatencyMode =
                    zeroLatencyModePtr
                    && juce::approximatelyEqual (zeroLatencyModePtr->load(), 1.0f);

                paramsToSync.ffDfEq =
                    static_cast<int> (ffDfEqPtr->load (std::memory_order_relaxed));
            }

            paramsToSync.paramsValid = true;
        }
    }

    // Update shared parameters if synced
    if ((syncChannelPtr->load (std::memory_order_relaxed) > 0)
        && ! readingSharedParams.load (std::memory_order_relaxed))
    {
        const auto ch = static_cast<int> (syncChannelPtr->load (std::memory_order_relaxed)) - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference (ch);

        if (parameterID.startsWith ("xOverF") && ! loadingFile)
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.xOverFreqs[idx] = xOverFreqsPtr[idx]->load();
        }
        else if (parameterID.startsWith ("solo"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            // CHANGED: Replaced std::round(soloBandPtr[idx]->load()) > 0.5f with juce::approximatelyEqual(soloBandPtr[idx]->load(), 1.0f)
            paramsToSync.solo[idx] = juce::approximatelyEqual (soloBandPtr[idx]->load(), 1.0f);
        }
        else if (parameterID.startsWith ("mute"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            // CHANGED: Replaced std::round(muteBandPtr[idx]->load()) > 0.5f with juce::approximatelyEqual(muteBandPtr[idx]->load(), 1.0f)
            paramsToSync.mute[idx] = juce::approximatelyEqual (muteBandPtr[idx]->load(), 1.0f);
        }
        else if (parameterID.startsWith ("alpha"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.dirFactors[idx] = dirFactorsPtr[idx]->load();
        }
        else if (parameterID == "nrBands")
        {
            paramsToSync.nrActiveBands = static_cast<int> (nProcessorBandsPtr->load());
        }
        else if (parameterID == "proximity")
        {
            paramsToSync.proximity = proxDistancePtr->load();
        }
        else if (parameterID == "proximityOnOff")
        {
            // CHANGED: Replaced std::round(proxOnOffPtr->load()) > 0.5f with juce::approximatelyEqual(proxOnOffPtr->load(), 1.0f)
            paramsToSync.proximityOnOff = juce::approximatelyEqual (proxOnOffPtr->load(), 1.0f);
        }
        else if (parameterID == "zeroLatencyMode")
        {
            // CHANGED: Replaced std::round(zeroLatencyModePtr->load()) > 0.5f with juce::approximatelyEqual(zeroLatencyModePtr->load(), 1.0f)
            paramsToSync.zeroLatencyMode =
                juce::approximatelyEqual (zeroLatencyModePtr->load(), 1.0f);
        }
        else if (parameterID.startsWith ("gain"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.gains[idx] = bandGainsPtr[idx]->load();
        }
        else if (parameterID == "ffDfEq")
        {
            paramsToSync.ffDfEq = static_cast<int> (ffDfEqPtr->load (std::memory_order_relaxed));
        }
    }
}

void PolarDesignerAudioProcessor::resetXoverFreqs()
{
    using namespace juce;

    switch (nProcessorBands)
    {
        case 1:
            break;

        case 2:
            for (unsigned int i = 0; i < nProcessorBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))
                    ->setValueNotifyingHost (
                        hzToZeroToOne (nProcessorBands, i, INIT_XOVER_FREQS_2B[i]));
            }
            break;

        case 3:
            for (unsigned int i = 0; i < nProcessorBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))
                    ->setValueNotifyingHost (
                        hzToZeroToOne (nProcessorBands, i, INIT_XOVER_FREQS_3B[i]));
            }
            break;

        case 4:
            for (unsigned int i = 0; i < nProcessorBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))
                    ->setValueNotifyingHost (
                        hzToZeroToOne (nProcessorBands, i, INIT_XOVER_FREQS_4B[i]));
            }
            break;

        case 5:
            for (unsigned int i = 0; i < (nProcessorBands - 1); ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))
                    ->setValueNotifyingHost (
                        hzToZeroToOne (nProcessorBands, i, INIT_XOVER_FREQS_5B[i]));
            }
            break;

        default:
            jassert (false);
            break;
    }
}

void PolarDesignerAudioProcessor::recomputeFilterCoefficientsIfNeeded()
{
    if (recomputeAllFilterCoefficients.exchange (false, std::memory_order_relaxed))
    {
        resetXoverFreqs();
        computeAllFilterCoefficients();
        repaintDEQ.store (true, std::memory_order_relaxed);
        return;
    }

    for (unsigned int i = 0; i < MAX_NUM_EQS - 1; ++i)
    {
        if (recomputeFilterCoefficients[i].exchange (false, std::memory_order_relaxed))
        {
            computeFilterCoefficients (i);
            updateConvolver (i);
        }
    }
}

void PolarDesignerAudioProcessor::computeAllFilterCoefficients()
{
    for (unsigned int i = 0; i < MAX_NUM_EQS - 1; ++i)
    {
        computeFilterCoefficients (i);
    }
    updateAllConvolvers();
}

void PolarDesignerAudioProcessor::computeFilterCoefficients (unsigned int crossoverNr)
{
    using namespace juce;
    using namespace dsp;
    if (nProcessorBands == 1)
        return;

    // Lowest band: lowpass
    if (crossoverNr == 0)
    {
        FilterDesign<float>::FIRCoefficientsPtr lowpass =
            FilterDesign<float>::designFIRLowpassWindowMethod (
                hzFromZeroToOne (nProcessorBands, 0, xOverFreqsPtr[0]->load()),
                currentSampleRate,
                static_cast<size_t> (firLen - 1),
                WindowingFunction<float>::WindowingMethod::hamming);
        firFilterBuffer.copyFrom (0, 0, lowpass->getRawCoefficients(), firLen - 1);
    }

    // Bandpass filters
    for (unsigned int i = std::max (1u, crossoverNr);
         i < std::min (crossoverNr + 2, nProcessorBands - 1);
         ++i)
    {
        const float halfBandwidth =
            (hzFromZeroToOne (nProcessorBands, i, xOverFreqsPtr[i]->load())
             - hzFromZeroToOne (nProcessorBands, i - 1, xOverFreqsPtr[i - 1]->load()))
            / 2;
        FilterDesign<float>::FIRCoefficientsPtr lp2bp =
            FilterDesign<float>::designFIRLowpassWindowMethod (
                halfBandwidth,
                currentSampleRate,
                static_cast<size_t> (firLen - 1),
                WindowingFunction<float>::WindowingMethod::hamming);

        const auto* lp2bpCoeffs = lp2bp->getRawCoefficients();
        auto* filterBufferPointer = firFilterBuffer.getWritePointer (static_cast<int> (i));
        const auto fCenter =
            halfBandwidth + hzFromZeroToOne (nProcessorBands, i - 1, xOverFreqsPtr[i - 1]->load());

        for (int j = 0; j < firLen; j++)
        {
            filterBufferPointer[j] = 2 * lp2bpCoeffs[j]
                                     * std::cos (MathConstants<float>::twoPi * fCenter
                                                 / static_cast<float> (currentSampleRate)
                                                 * static_cast<float> (j - (firLen - 1.0) / 2));
        }
    }

    // Highest band: highpass
    if (crossoverNr == nProcessorBands - 2)
    {
        float hpBandwidth =
            static_cast<float> (currentSampleRate / 2
                                - hzFromZeroToOne (nProcessorBands,
                                                   nProcessorBands - 2,
                                                   xOverFreqsPtr[nProcessorBands - 2]->load()));
        auto* filterBufferPointer =
            firFilterBuffer.getWritePointer (static_cast<int> (nProcessorBands) - 1);
        dsp::FilterDesign<float>::FIRCoefficientsPtr lp2hp =
            dsp::FilterDesign<float>::designFIRLowpassWindowMethod (
                hpBandwidth,
                currentSampleRate,
                static_cast<size_t> (firLen - 1),
                dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lp2hpCoeffs = lp2hp->getRawCoefficients();
        for (int i = 0; i < firLen; ++i)
        {
            filterBufferPointer[i] =
                lp2hpCoeffs[i]
                * std::cos (MathConstants<float>::pi
                            * (static_cast<float> (i) - (static_cast<float> (firLen) - 1.0f) / 2));
        }
    }
}

void PolarDesignerAudioProcessor::updateAllConvolvers()
{
    using namespace juce;

    const auto nBands = nProcessorBands.load();
    for (unsigned int i = 0; i < nBands; ++i)
    {
        // Load impulse response only if coefficients changed
        AudioBuffer<float> convSingleBuff (1, firLen);
        convSingleBuff.copyFrom (0, 0, firFilterBuffer, static_cast<int> (i), 0, firLen);

        convolvers[2 * i].loadImpulseResponse (std::move (convSingleBuff),
                                               currentSampleRate,
                                               dsp::Convolution::Stereo::no,
                                               dsp::Convolution::Trim::no,
                                               dsp::Convolution::Normalise::no);

        // Re-create convSingleBuff for second copy since previous was moved
        convSingleBuff = AudioBuffer<float> (1, firLen);
        convSingleBuff.copyFrom (0,
                                 0,
                                 firFilterBuffer,
                                 static_cast<int> (i),
                                 0,
                                 firLen); // Re-copy for Eight convolver
        convolvers[2 * i + 1].loadImpulseResponse (std::move (convSingleBuff),
                                                   currentSampleRate,
                                                   dsp::Convolution::Stereo::no,
                                                   dsp::Convolution::Trim::no,
                                                   dsp::Convolution::Normalise::no);
    }
}

void PolarDesignerAudioProcessor::updateConvolver (size_t convNr)
{
    using namespace juce;
    using namespace dsp;

    if (currentBlockSize == 0 || currentSampleRate <= 0.0)
    {
        LOG_ERROR ("Cannot initialize convolver: invalid block size or sample rate");
        return;
    }

    // TODO: ideally we should not allocate the IR holders on the audio thread
    // prepare impulse response holders
    std::array<AudioBuffer<float>, 4> convBuffers;

    for (auto i = convNr; i < convNr + 2; ++i)
    {
        const auto j = i - convNr;
        const auto k = j + 2;

        convBuffers[j].setSize (1, firLen);
        convBuffers[k].setSize (1, firLen);

        convBuffers[j].copyFrom (0, 0, firFilterBuffer, static_cast<int> (i), 0, firLen);
        convBuffers[k].copyFrom (0, 0, firFilterBuffer, static_cast<int> (i), 0, firLen);
    }

    for (auto i = convNr; i < convNr + 2; ++i)
    {
        const auto j = i - convNr;
        const auto k = j + 2;

        // Omni convolver
        convolvers[2 * i].loadImpulseResponse (std::move (convBuffers[j]),
                                               currentSampleRate,
                                               Convolution::Stereo::no, // isStereo
                                               Convolution::Trim::no, // trim
                                               Convolution::Normalise::no); // normalise

        // Eight convolver
        convolvers[2 * i + 1].loadImpulseResponse (std::move (convBuffers[k]),
                                                   currentSampleRate,
                                                   Convolution::Stereo::no, // isStereo
                                                   Convolution::Trim::no, // trim
                                                   Convolution::Normalise::no); // normalise
    }
}

void PolarDesignerAudioProcessor::createOmniAndEightSignals (juce::AudioBuffer<float>& buffer)
{
    using namespace juce;

    int numSamples = buffer.getNumSamples();
    // calculate omni part
    const float* readPointerFront = buffer.getReadPointer (0);
    const float* readPointerBack = buffer.getReadPointer (1);
    float* writePointerOmni = omniEightBuffer.getWritePointer (0);
    FloatVectorOperations::copy (writePointerOmni, readPointerFront, numSamples);
    FloatVectorOperations::add (writePointerOmni, readPointerBack, numSamples);

    // calculate fig-of-eight part
    float* writePointerEight = omniEightBuffer.getWritePointer (1);
    FloatVectorOperations::copy (writePointerEight, readPointerFront, numSamples);
    FloatVectorOperations::subtract (writePointerEight, readPointerBack, numSamples);
}

// In createPolarPatterns (around line 1350)
void PolarDesignerAudioProcessor::createPolarPatterns (juce::AudioBuffer<float>& buffer)
{
    using namespace juce;

    int numSamples = buffer.getNumSamples();
    buffer.clear();

    unsigned int nActiveBands = nProcessorBands;
    // CHANGED: Replaced std::round(zeroLatencyModePtr->load()) > 0.5f with juce::approximatelyEqual(zeroLatencyModePtr->load(), 1.0f)
    if (juce::approximatelyEqual (zeroLatencyModePtr->load(), 1.0f))
        nActiveBands = 1;

    for (unsigned int i = 0; i < nActiveBands; ++i)
    {
        // CHANGED: Replaced std::round(muteBandPtr[i]->load()) > 0.5 && std::round(soloBandPtr[i]->load()) < 0.5
        // with juce::approximatelyEqual(muteBandPtr[i]->load(), 1.0f) && !juce::approximatelyEqual(soloBandPtr[i]->load(), 1.0f)
        if ((juce::approximatelyEqual (muteBandPtr[i]->load(), 1.0f)
             && ! juce::approximatelyEqual (soloBandPtr[i]->load(), 1.0f))
            || (soloActive && ! juce::approximatelyEqual (soloBandPtr[i]->load(), 1.0f)))
            continue;

        // calculate patterns and add to output buffer
        const float* readPointerOmni = filterBankBuffer.getReadPointer (static_cast<int> (2 * i));
        const float* readPointerEight =
            filterBankBuffer.getReadPointer (static_cast<int> (2 * i + 1));

        float oldGain = Decibels::decibelsToGain (oldBandGains[i], -59.91f);
        float gain = Decibels::decibelsToGain (bandGainsPtr[i]->load(), -59.91f);

        // add with ramp to prevent crackling noises
        buffer.addFromWithRamp (0,
                                0,
                                readPointerOmni,
                                numSamples,
                                (1 - std::abs (oldDirFactors[i])) * oldGain,
                                (1 - std::abs (dirFactorsPtr[i]->load())) * gain);
        buffer.addFromWithRamp (0,
                                0,
                                readPointerEight,
                                numSamples,
                                oldDirFactors[i] * oldGain,
                                dirFactorsPtr[i]->load() * gain);

        oldDirFactors[i] = dirFactorsPtr[i]->load();
        oldBandGains[i] = bandGainsPtr[i]->load();
    }

    // delay needs to be running constantly to prevent clicks
    delayBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
    dsp::AudioBlock<float> delayBlock (delayBuffer);
    dsp::ProcessContextReplacing<float> delayContext (delayBlock);
    delay.process (delayContext);

    // CHANGED: Replaced std::round(zeroLatencyModePtr->load()) < 0.5f with !juce::approximatelyEqual(zeroLatencyModePtr->load(), 1.0f)
    if ((nActiveBands == 1) && ! juce::approximatelyEqual (zeroLatencyModePtr->load(), 1.0f))
    {
        buffer.copyFrom (0, 0, delayBuffer, 0, 0, numSamples);
    }

    // copy to second output channel -> this generates loud glitches in pro tools if mono output configuration is used
    // -> check getMainBusNumOutputChannels() !J!
    int numOutputChannels = getMainBusNumOutputChannels();

    if (buffer.getNumChannels() >= 2 && numOutputChannels >= 2)
    {
        buffer.copyFrom (1, 0, buffer, 0, 0, numSamples);
    }
    else if (numOutputChannels == 1)
    {
        // Ensure mono output is handled cleanly
        buffer.clear (1, 0, numSamples); // Clear unused channel
    }
    else
    {
        LOG_ERROR ("Unexpected output channel configuration: " + String (numOutputChannels));
    }
}

juce::Result PolarDesignerAudioProcessor::loadPreset (const juce::File& presetFile)
{
    using namespace juce;

    var parsedJson;
    if (! presetFile.exists())
        return Result::fail ("File does not exist!");

    String jsonString = presetFile.loadFileAsString();
    Result result = JSON::parse (jsonString, parsedJson);
    if (! result.wasOk())
        return Result::fail ("File could not be parsed: Please provide valid JSON!");

    for (auto& it : presetProperties)
    {
        if (! parsedJson.hasProperty (it))
            return Result::fail ("Corrupt preset file: No '" + it + "' property found.");
    }

    loadingFile = true;

    float x = parsedJson.getProperty ("nrActiveBands", parsedJson);
    vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
        vtsParams.getParameter ("nrBands")->convertTo0to1 (x - 1));

    for (size_t i = 0; i < 4; ++i)
    {
        x = parsedJson.getProperty ("xOverF" + String (i + 1), parsedJson);
        vtsParams.getParameter ("xOverF" + String (i + 1))
            ->setValueNotifyingHost (hzToZeroToOne (nProcessorBands, i, x));
    }

    NormalisableRange<float> dfRange = vtsParams.getParameter ("alpha1")->getNormalisableRange();

    for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
    {
        x = parsedJson.getProperty ("dirFactor" + String (i + 1), parsedJson);
        if (x < dfRange.start || x > dfRange.end)
            return Result::fail ("DirFactor" + String (i + 1) + " needs to be between "
                                 + String (dfRange.start) + " and " + String (dfRange.end) + ".");
        vtsParams.getParameter ("alpha" + String (i + 1))
            ->setValueNotifyingHost (dfRange.convertTo0to1 (x));

        x = parsedJson.getProperty ("gain" + String (i + 1), parsedJson);
        vtsParams.getParameter ("gain" + String (i + 1))
            ->setValueNotifyingHost (
                vtsParams.getParameter ("gain" + String (i + 1))->convertTo0to1 (x));

        x = parsedJson.getProperty ("solo" + String (i + 1), parsedJson);
        vtsParams.getParameter ("solo" + String (i + 1))
            ->setValueNotifyingHost (
                vtsParams.getParameter ("solo" + String (i + 1))->convertTo0to1 (x));

        x = parsedJson.getProperty ("mute" + String (i + 1), parsedJson);
        vtsParams.getParameter ("mute" + String (i + 1))
            ->setValueNotifyingHost (
                vtsParams.getParameter ("mute" + String (i + 1))->convertTo0to1 (x));
    }

    x = parsedJson.getProperty ("ffDfEq", parsedJson);
    vtsParams.getParameter ("ffDfEq")->setValueNotifyingHost (
        vtsParams.getParameter ("ffDfEq")->convertTo0to1 (x));

    x = parsedJson.getProperty ("proximity", parsedJson);
    vtsParams.getParameter ("proximity")
        ->setValueNotifyingHost (vtsParams.getParameter ("proximity")->convertTo0to1 (x));

    x = parsedJson.getProperty ("proximityOnOff", parsedJson);
    vtsParams.getParameter ("proximityOnOff")
        ->setValueNotifyingHost (vtsParams.getParameter ("proximityOnOff")->convertTo0to1 (x));

    x = parsedJson.getProperty ("zeroLatencyMode", parsedJson);
    vtsParams.getParameter ("zeroLatencyMode")
        ->setValueNotifyingHost (vtsParams.getParameter ("zeroLatencyMode")->convertTo0to1 (x));

    loadingFile = false;

    // set parameters
    nProcessorBands = static_cast<unsigned int> (nProcessorBandsPtr->load() + 1);

    computeAllFilterCoefficients();
    repaintDEQ.store (true, std::memory_order_relaxed);

    return Result::ok();
}

juce::Result PolarDesignerAudioProcessor::savePreset (juce::File destination)
{
    using namespace juce;

    DynamicObject::Ptr jsonObj = new DynamicObject();

    jsonObj->setProperty (
        "Description",
        var ("This preset file was created with the Austrian Audio PolarDesigner plugin v"
             + String (JucePlugin_VersionString)
             + ", for more information see www.austrian.audio ."));
    jsonObj->setProperty ("nrActiveBands",
                          static_cast<int> (nProcessorBands.load (std::memory_order_relaxed)));
    jsonObj->setProperty ("zeroLatencyMode", zeroLatencyModePtr->load (std::memory_order_relaxed));
    jsonObj->setProperty ("trimPosition", trimPositionPtr->load (std::memory_order_relaxed));
    jsonObj->setProperty (
        "xOverF1",
        static_cast<int> (hzFromZeroToOne (nProcessorBands,
                                           0,
                                           xOverFreqsPtr[0]->load (std::memory_order_relaxed))));
    jsonObj->setProperty (
        "xOverF2",
        static_cast<int> (hzFromZeroToOne (nProcessorBands,
                                           1,
                                           xOverFreqsPtr[1]->load (std::memory_order_relaxed))));
    jsonObj->setProperty (
        "xOverF3",
        static_cast<int> (hzFromZeroToOne (nProcessorBands,
                                           2,
                                           xOverFreqsPtr[2]->load (std::memory_order_relaxed))));
    jsonObj->setProperty (
        "xOverF4",
        static_cast<int> (hzFromZeroToOne (nProcessorBands,
                                           3,
                                           xOverFreqsPtr[3]->load (std::memory_order_relaxed))));
    jsonObj->setProperty ("dirFactor1", dirFactorsPtr[0]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("dirFactor2", dirFactorsPtr[1]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("dirFactor3", dirFactorsPtr[2]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("dirFactor4", dirFactorsPtr[3]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("dirFactor5", dirFactorsPtr[4]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("gain1", bandGainsPtr[0]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("gain2", bandGainsPtr[1]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("gain3", bandGainsPtr[2]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("gain4", bandGainsPtr[3]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("gain5", bandGainsPtr[4]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("solo1", soloBandPtr[0]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("solo2", soloBandPtr[1]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("solo3", soloBandPtr[2]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("solo4", soloBandPtr[3]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("solo5", soloBandPtr[4]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("mute1", muteBandPtr[0]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("mute2", muteBandPtr[1]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("mute3", muteBandPtr[2]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("mute4", muteBandPtr[3]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("mute5", muteBandPtr[4]->load (std::memory_order_relaxed));
    jsonObj->setProperty ("ffDfEq", ffDfEqPtr->load (std::memory_order_relaxed));
    jsonObj->setProperty ("proximity", proxDistancePtr->load (std::memory_order_relaxed));
    jsonObj->setProperty ("proximityOnOff", proxOnOffPtr->load (std::memory_order_relaxed));

    String jsonString = JSON::toString (var (jsonObj), false, 2);
    if (destination.replaceWithText (jsonString))
        return Result::ok();
    else
        return Result::fail ("Could not write preset file. Check file access permissions.");
}

float PolarDesignerAudioProcessor::getXoverSliderRangeStart (int sliderNum)
{
    switch (nProcessorBands)
    {
        case 2:
            return XOVER_RANGE_START_2B[sliderNum];
            break;

        case 3:
            return XOVER_RANGE_START_3B[sliderNum];
            break;

        case 4:
            return XOVER_RANGE_START_4B[sliderNum];
            break;

        case 5:
            return XOVER_RANGE_START_5B[sliderNum];
            break;

        default:
            jassert (false);
            break;
    }
    return 0;
}

float PolarDesignerAudioProcessor::getXoverSliderRangeEnd (int sliderNum)
{
    switch (nProcessorBands)
    {
        case 2:
            return XOVER_RANGE_END_2B[sliderNum];
            break;

        case 3:
            return XOVER_RANGE_END_3B[sliderNum];
            break;

        case 4:
            return XOVER_RANGE_END_4B[sliderNum];
            break;

        case 5:
            return XOVER_RANGE_END_5B[sliderNum];
            break;

        default:
            jassert (false);
            break;
    }
    return 0;
}

void PolarDesignerAudioProcessor::startTracking (bool trackDisturber)
{
    if (trackDisturber)
    {
        trackingDisturber = true;
        for (size_t i = 0; i < MAX_NUM_EQS; ++i)
        {
            omniSqSumDist[i] = 0.0f;
            eightSqSumDist[i] = 0.0f;
            omniEightSumDist[i] = 0.0f;
        }
    }
    else
    {
        trackingDisturber = false;
        for (size_t i = 0; i < MAX_NUM_EQS; ++i)
        {
            omniSqSumSig[i] = 0.0f;
            eightSqSumSig[i] = 0.0f;
            omniEightSumSig[i] = 0.0f;
        }
    }
    nrBlocksRecorded = 0;
    trackingActive = true;
}

void PolarDesignerAudioProcessor::resetTrackingState()
{
    trackingActive = false;
    trackingDisturber = false;
    signalRecorded = false;
    disturberRecorded = false;
    nrBlocksRecorded = 0;
    for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
    {
        omniSqSumSig[i] = 0.0f;
        eightSqSumSig[i] = 0.0f;
        omniEightSumSig[i] = 0.0f;
        omniSqSumDist[i] = 0.0f;
        eightSqSumDist[i] = 0.0f;
        omniEightSumDist[i] = 0.0f;
    }
}

void PolarDesignerAudioProcessor::stopTracking (int applyOptimalPattern)
{
    auto normalizeSqSumDist = [this]()
    {
        const auto n = static_cast<float> (nrBlocksRecorded);

        for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
        {
            omniSqSumDist[i] = omniSqSumDist[i] / n;
            eightSqSumDist[i] = eightSqSumDist[i] / n;
            omniEightSumDist[i] = omniEightSumDist[i] / n;
        }
    };

    auto normalizeSqSumSig = [this]()
    {
        const auto n = static_cast<float> (nrBlocksRecorded);

        for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
        {
            omniSqSumSig[i] = omniSqSumSig[i] / n;
            eightSqSumSig[i] = eightSqSumSig[i] / n;
            omniEightSumSig[i] = omniEightSumSig[i] / n;
        }
    };

    trackingActive = false;
    if (nrBlocksRecorded == 0)
        return; // Skip if no blocks recorded

    if (applyOptimalPattern == 1)
    {
        if (trackingDisturber)
        {
            if (nrBlocksRecorded != 0)
                normalizeSqSumDist();

            setMinimumDisturbancePattern();
        }
        else
        {
            if (nrBlocksRecorded != 0)
                normalizeSqSumSig();

            setMaximumSignalPattern();
        }
    }
    else if (applyOptimalPattern == 2) // max sig-to-dist
    {
        if (trackingDisturber)
        {
            if (nrBlocksRecorded != 0)
                normalizeSqSumDist();

            disturberRecorded = true;
        }
        else
        {
            if (nrBlocksRecorded != 0)
                normalizeSqSumSig();

            signalRecorded = true;
        }
        maximizeSigToDistRatio();
    }
}

void PolarDesignerAudioProcessor::trackSignalEnergy (int numSamples)
{
    using namespace juce;

    if (numSamples == 0)
        return; // avoid division by zero

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        const float* readPointerOmni = filterBankBuffer.getReadPointer (static_cast<int> (2 * i));
        const float* readPointerEight =
            filterBankBuffer.getReadPointer (static_cast<int> (2 * i + 1));
        if (trackingDisturber)
        {
            for (int j = 0; j < numSamples; ++j)
            {
                float omniSample = readPointerOmni[j];
                omniSqSumDist[i] += omniSample * omniSample / static_cast<float> (numSamples);
                float eightSample = readPointerEight[j];
                eightSqSumDist[i] += eightSample * eightSample / static_cast<float> (numSamples);
                omniEightSumDist[i] += omniSample * eightSample / static_cast<float> (numSamples);
            }
        }
        else
        {
            for (int j = 0; j < numSamples; ++j)
            {
                float omniSample = readPointerOmni[j];
                omniSqSumSig[i] += omniSample * omniSample / static_cast<float> (numSamples);
                float eightSample = readPointerEight[j];
                eightSqSumSig[i] += eightSample * eightSample / static_cast<float> (numSamples);
                omniEightSumSig[i] += omniSample * eightSample / static_cast<float> (numSamples);
            }
        }
    }
    ++nrBlocksRecorded;
}

void PolarDesignerAudioProcessor::setMinimumDisturbancePattern()
{
    using namespace juce;

    float disturberPower = 0.0f;
    float minPowerAlpha = 0.0f;
    float alphaStart = 0.0f;

    // !J! NOTE: allowBackwardsPattern is ALWAYS true
    alphaStart = -0.5f;

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentPower = std::pow ((1 - std::abs (alpha)), 2.0f) * omniSqSumDist[i]
                                 + std::pow (alpha, 2.0f) * eightSqSumDist[i]
                                 + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumDist[i];
            if (juce::exactlyEqual (alpha, alphaStart) || (currentPower < disturberPower))
            {
                disturberPower = currentPower;
                minPowerAlpha = alpha;
            }
        }

        // do not apply changes, if playback is not active
        if (! juce::exactlyEqual (disturberPower, 0.0f))
        {
            vtsParams.getParameter ("alpha" + String (i + 1))
                ->setValueNotifyingHost (
                    vtsParams.getParameter ("alpha1")->convertTo0to1 (minPowerAlpha));
            disturberRecorded = true;
        }
    }
}

void PolarDesignerAudioProcessor::setMaximumSignalPattern()
{
    using namespace juce;

    float signalPower = 0.0f;
    float maxPowerAlpha = 0.0f;
    float alphaStart = -0.5f;

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentPower = std::pow ((1 - std::abs (alpha)), 2.0f) * omniSqSumSig[i]
                                 + std::pow (alpha, 2.0f) * eightSqSumSig[i]
                                 + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumSig[i];
            if (juce::exactlyEqual (alpha, alphaStart) || (currentPower > signalPower))
            {
                signalPower = currentPower;
                maxPowerAlpha = alpha;
            }
        }

        if (! juce::exactlyEqual (signalPower, 0.0f))
        {
            vtsParams.getParameter ("alpha" + String (i + 1))
                ->setValueNotifyingHost (
                    vtsParams.getParameter ("alpha1")->convertTo0to1 (maxPowerAlpha));
            signalRecorded = true;
        }
    }
}

void PolarDesignerAudioProcessor::maximizeSigToDistRatio()
{
    using namespace juce;

    if (! signalRecorded || ! disturberRecorded)
        return; // Skip if both signal and disturber haven't been recorded

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        if (juce::approximatelyEqual (omniSqSumSig[i], 0.0f)
            && juce::approximatelyEqual (eightSqSumSig[i], 0.0f)
            && juce::approximatelyEqual (omniEightSumSig[i], 0.0f)
            && juce::approximatelyEqual (omniSqSumDist[i], 0.0f)
            && juce::approximatelyEqual (eightSqSumDist[i], 0.0f)
            && juce::approximatelyEqual (omniEightSumDist[i], 0.0f))
            return;
    }

    float distToSigRatio = 0.0f;
    float maxDistToSigAlpha = 0.0f;
    float alphaStart = -0.5f;

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentSigPower =
                static_cast<float> (std::pow ((1 - std::abs (alpha)), 2) * omniSqSumSig[i]
                                    + std::pow (alpha, 2) * eightSqSumSig[i]
                                    + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumSig[i]);
            float currentDistPower =
                static_cast<float> (std::pow ((1 - std::abs (alpha)), 2) * omniSqSumDist[i]
                                    + std::pow (alpha, 2) * eightSqSumDist[i]
                                    + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumDist[i]);
            float currentRatio;
            if (juce::exactlyEqual (currentDistPower, 0.0f))
            {
                currentRatio = 0.0f;
            }
            else
                currentRatio = currentSigPower / currentDistPower;

            if (juce::exactlyEqual (alpha, alphaStart) || (currentRatio > distToSigRatio))
            {
                distToSigRatio = currentRatio;
                maxDistToSigAlpha = alpha;
            }
        }

        if (! juce::exactlyEqual (distToSigRatio, 0.0f))
            vtsParams.getParameter ("alpha" + String (i + 1))
                ->setValueNotifyingHost (
                    vtsParams.getParameter ("alpha1")->convertTo0to1 (maxDistToSigAlpha));
    }
}

void PolarDesignerAudioProcessor::setProxCompCoefficients (float distance)
{
    using namespace juce::dsp::IIR;

    if (std::abs (distance) < 0.0001f)
    {
        LOG_WARN ("Invalid proximity distance, using default coefficients");
        *proxCompIIR.coefficients =
            Coefficients<float> (1.0f, 0.0f, 1.0f, 0.0f); // Unity gain filter
        return;
    }

    auto c = 343.0f;
    double fs = getSampleRate();
    if (fs <= 0.0)
    {
        LOG_WARN ("Invalid sample rate, using default coefficients");
        *proxCompIIR.coefficients = Coefficients<float> (1.0f, 0.0f, 1.0f, 0.0f);
        return;
    }

    float a = (0.05f - 1.0f) / (-log (1.1f) + log (0.1f));
    float b = 1.0f + a * log (0.1f);
    float r = -a * log (std::max (std::abs (distance), 0.0001f)) + b;

    float b0, b1, a0, a1;
    if (distance <= 0)
    { // Bass cut
        r = std::max (r, 0.01f);
        b0 = static_cast<float> (c * (r - 1.0f) / (fs * 2.0f * r) + 1.0f);
        b1 = static_cast<float> (-exp (-c / (fs * r)) * (1.0f - c * (r - 1.0f) / (fs * 2.0f * r)));
        a0 = 1.0f;
        a1 = static_cast<float> (-exp (-c / (fs * r)));
    }
    else
    { // Bass boost
        r = std::max (r, 0.05f);
        b0 = static_cast<float> (c * (1.0f - r) / (fs * 2.0f * r) + 1.0f);
        b1 = static_cast<float> (-exp (-c / fs) * (1.0f - c * (1.0f - r) / (fs * 2.0f * r)));
        a0 = 1.0f;
        a1 = static_cast<float> (-exp (-c / fs));
    }

    *proxCompIIR.coefficients = Coefficients<float> (b0, b1, a0, a1);
}

void PolarDesignerAudioProcessor::timerCallback()
{
    using namespace juce;

    if (zeroLatencyModeChanged.exchange (false, std::memory_order_relaxed))
        updateLatency();

    const auto syncChannel = static_cast<int> (syncChannelPtr->load (std::memory_order_relaxed));

    if (syncChannel > 0)
    {
        readingSharedParams.store (true, std::memory_order_relaxed);
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference (syncChannel - 1);

        if (! exactlyEqual (nProcessorBandsPtr->load (std::memory_order_relaxed),
                            static_cast<float> (paramsToSync.nrActiveBands)))
        {
            vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
                vtsParams.getParameterRange ("nrBands").convertTo0to1 (
                    static_cast<float> (paramsToSync.nrActiveBands)));
            repaintDEQ.store (true, std::memory_order_relaxed);
        }

        for (unsigned int i = 0; i < MAX_NUM_EQS; ++i)
        {
            if (! exactlyEqual (dirFactorsPtr[i]->load (std::memory_order_relaxed),
                                paramsToSync.dirFactors[i]))
            {
                vtsParams.getParameter ("alpha" + String (i + 1))
                    ->setValueNotifyingHost (vtsParams.getParameterRange ("alpha" + String (i + 1))
                                                 .convertTo0to1 (paramsToSync.dirFactors[i]));
            }

            if (! exactlyEqual (soloBandPtr[i]->load (std::memory_order_relaxed),
                                paramsToSync.solo[i] ? 1.0f : 0.0f))
            {
                vtsParams.getParameter ("solo" + String (i + 1))
                    ->setValueNotifyingHost (vtsParams.getParameterRange ("solo" + String (i + 1))
                                                 .convertTo0to1 (paramsToSync.solo[i]));
            }

            if (! exactlyEqual (muteBandPtr[i]->load (std::memory_order_relaxed),
                                paramsToSync.mute[i] ? 1.0f : 0.0f))
            {
                vtsParams.getParameter ("mute" + String (i + 1))
                    ->setValueNotifyingHost (vtsParams.getParameterRange ("mute" + String (i + 1))
                                                 .convertTo0to1 (paramsToSync.mute[i]));
            }

            if (! exactlyEqual (bandGainsPtr[i]->load (std::memory_order_relaxed),
                                paramsToSync.gains[i]))
            {
                vtsParams.getParameter ("gain" + String (i + 1))
                    ->setValueNotifyingHost (vtsParams.getParameterRange ("gain" + String (i + 1))
                                                 .convertTo0to1 (paramsToSync.gains[i]));
            }

            if ((i < 4)
                && ! exactlyEqual (xOverFreqsPtr[i]->load (std::memory_order_relaxed),
                                   paramsToSync.xOverFreqs[i]))
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))
                    ->setValueNotifyingHost (vtsParams.getParameterRange ("xOverF" + String (i + 1))
                                                 .convertTo0to1 (paramsToSync.xOverFreqs[i]));
            }
        }

        if (! exactlyEqual (proxDistancePtr->load (std::memory_order_relaxed),
                            paramsToSync.proximity))
        {
            vtsParams.getParameter ("proximity")
                ->setValueNotifyingHost (vtsParams.getParameterRange ("proximity")
                                             .convertTo0to1 (paramsToSync.proximity));
        }

        if (! exactlyEqual (proxOnOffPtr->load (std::memory_order_relaxed),
                            paramsToSync.proximityOnOff ? 1.0f : 0.0f))
        {
            vtsParams.getParameter ("proximityOnOff")
                ->setValueNotifyingHost (vtsParams.getParameterRange ("proximityOnOff")
                                             .convertTo0to1 (paramsToSync.proximityOnOff));
        }

        if (static_cast<int> (ffDfEqPtr->load (std::memory_order_relaxed)) != paramsToSync.ffDfEq)
            vtsParams.getParameter ("ffDfEq")->setValueNotifyingHost (
                vtsParams.getParameterRange ("ffDfEq").convertTo0to1 (
                    static_cast<float> (paramsToSync.ffDfEq)));

        if ((std::round (zeroLatencyModePtr->load (std::memory_order_relaxed)) > 0.5f ? true
                                                                                      : false)
            != paramsToSync.zeroLatencyMode)
        {
            vtsParams.getParameter ("zeroLatencyMode")
                ->setValueNotifyingHost (vtsParams.getParameterRange ("zeroLatencyMode")
                                             .convertTo0to1 (paramsToSync.zeroLatencyMode));

            paramsToSync.zeroLatencyMode =
                (std::round (zeroLatencyModePtr->load (std::memory_order_relaxed)) > 0.5f);

#ifdef USE_EXTRA_DEBUG_DUMPS
            LOG_DEBUG (String::formatted ("PLUGINPROCESSOR %p: zeroLatencyModePtr update", this));
#endif
        }

        readingSharedParams.store (false, std::memory_order_relaxed);
    }
}

void PolarDesignerAudioProcessor::updateLatency()
{
    using namespace juce;
    if (isBypassed
        || approximatelyEqual (zeroLatencyModePtr->load (std::memory_order_relaxed), 1.0f))
    {
        setLatencySamples (0);
        return;
    }

    auto latency = (firLen - 1) / 2;

    // we are using free/diffuse field eq
    if (! approximatelyEqual (ffDfEqPtr->load (std::memory_order_relaxed), 0.0f))
        latency += eqLatency;

    setLatencySamples (latency);
}

void PolarDesignerAudioProcessor::changeABLayerState (int state)
{
    // Validate abLayerState
    jassert (state == COMPARE_LAYER_A || state == COMPARE_LAYER_B);

    abLayerState = state;
    abLayerChanged.store (true, std::memory_order_relaxed);

    resetTrackingState(); // Clear tracking data

#ifdef USE_EXTRA_DEBUG_DUMPS
    juce::String treeAsXmlString = vtsParams.state.toXmlString();
    LOG_DEBUG (treeAsXmlString.toStdString());
#endif

    if (abLayerState == COMPARE_LAYER_B)
    {
        layerA = vtsParams.copyState();

        if (! zeroLatencyModeActive() && proxDistancePtr && nProcessorBandsPtr)
        {
            if (proxDistancePtr && nProcessorBandsPtr)
            {
                oldProxDistanceA.store (proxDistancePtr->load (std::memory_order_relaxed),
                                        std::memory_order_relaxed);
                oldNrBandsA.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                                   std::memory_order_relaxed);
            }
        }
        readingSharedParams.store (true, std::memory_order_relaxed);

        vtsParams.state = layerB.createCopy();

        if (zeroLatencyModeActive())
        {
            oldProxDistance = 0;
        }
        else
        {
            oldProxDistance.store (oldProxDistanceB.load (std::memory_order_relaxed),
                                   std::memory_order_relaxed);
            oldNrBands.store (oldNrBandsB.load (std::memory_order_relaxed),
                              std::memory_order_relaxed);
        }
    }
    else
    {
        layerB = vtsParams.copyState();

        if (! zeroLatencyModeActive())
        {
            if (proxDistancePtr && nProcessorBandsPtr)
            {
                oldProxDistanceB.store (proxDistancePtr->load (std::memory_order_relaxed),
                                        std::memory_order_relaxed);
                oldNrBandsB.store (nProcessorBandsPtr->load (std::memory_order_relaxed),
                                   std::memory_order_relaxed);
            }
        }
        readingSharedParams.store (true, std::memory_order_relaxed);

        vtsParams.state = layerA.createCopy();

        if (zeroLatencyModeActive())
        {
            oldProxDistance = 0;
        }
        else
        {
            oldProxDistance.store (oldProxDistanceA.load (std::memory_order_relaxed));
            oldNrBands.store (oldNrBandsA.load (std::memory_order_relaxed));
        }
    }

    if (proxDistancePtr)
        vtsParams.getParameter ("proximity")
            ->setValueNotifyingHost (
                vtsParams.getParameter ("proximity")->convertTo0to1 (oldProxDistance));

    if (nProcessorBandsPtr)
        vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
            vtsParams.getParameter ("nrBands")->convertTo0to1 (oldNrBands));

    abLayerChanged.store (false, std::memory_order_relaxed);

    repaintDEQ.store (true, std::memory_order_relaxed);

#ifdef USE_EXTRA_DEBUG_DUMPS
    juce::String treeAsXmlString2 = vtsParams.state.toXmlString();
    LOG_DEBUG (treeAsXmlString2.toStdString());
#endif
}

void PolarDesignerAudioProcessor::updateFirLen()
{
    // Calculate firLen
    int newFirLen = static_cast<int> (
        std::ceil (static_cast<double> (FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE)
                   / FILTER_BANK_NATIVE_SAMPLE_RATE * currentSampleRate));
    if (newFirLen % 2 == 0)
        newFirLen++;
    jassert (newFirLen % 2 == 1);
    firLen = newFirLen;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolarDesignerAudioProcessor();
}
