/*
==============================================================================
PluginProcessor.cpp
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
==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

/* We use versionHint of ParameterID from now on - rigorously! */
#define PD_PARAMETER_V1 1

/* Trim Slider */
#define PD_PARAMETER_V2 2

/* Alternative methods for performance optimization */
#define USE_LINEAR_INTERPOLATION_RESAMPLING
#undef USE_NEW_PREPARETOPLAY
#undef USE_NEW_SETSTATEINFORMATION
#undef USE_NEW_INITCONVOLVERS
#undef USE_NEW_UPDATELATENCY

/* Debug State Information dumps */
#define USE_EXTRA_DEBUG_DUMPS

//==============================================================================
PolarDesignerAudioProcessor::PolarDesignerAudioProcessor()
    : AudioProcessor(BusesProperties()
              .withInput("Input", AudioChannelSet::stereo(), true)
              .withOutput("Output", AudioChannelSet::stereo(), true)),
      repaintDEQ(true),
      activeBandsChanged(true),
      zeroLatencyModeChanged(true),
      ffDfEqChanged(true),
      recomputeFilterCoefficients({false, false, false, false}),
      recomputeAllFilterCoefficients(true),
      disturberRecorded(false),
      signalRecorded(false),
      abLayerState(COMPARE_LAYER_A),
      saveTree("save"),
      nodeA("layerA"),
      nodeB("layerB"),
      nodeParams("vtsParams"),
      layerA(nodeA),
      layerB(nodeB),
      saveStates(saveTree),
      doEq(0),
      doEqA(0),
      doEqB(0),
      oldProxDistance(0.0f),
      oldProxDistanceA(0.0f),
      oldProxDistanceB(0.0f),
      oldNrBands(0.0f),
      oldNrBandsA(MAX_NUM_EQS),
      oldNrBandsB(MAX_NUM_EQS),
      abLayerChanged(false),
      zeroLatencyModeA(0.0f),
      zeroLatencyModeB(0.0f),
      termControlWaveform(1),
      audioPlayHead(getPlayHead()),
      playHeadPosition(),
      playHeadState(), // Deprecated, but included as per declaration
      undoManager(),
      properties(),
      lastEqSpec{0.0, 0, 0},
      lastConvSpec{0.0, 0, 0},
      nProcessorBands(MAX_NUM_EQS),
      vtsParams(*this, &undoManager, "AAPolarDesigner",
          {
              std::make_unique<AudioParameterFloat>(ParameterID{"trimPosition", PD_PARAMETER_V2}, "trimPosition", NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0f, AudioParameterFloatAttributes().withLabel("Trim").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([&](float value, [[maybe_unused]] int maximumStringLength) { return String(std::roundf(hzFromZeroToOne(0, value))) + " Hz trimPot"; }).withAutomatable(false)),
              std::make_unique<AudioParameterFloat>(ParameterID{"xOverF1", PD_PARAMETER_V1}, "Xover1", NormalisableRange<float>(0.0f, 1.0f, 0.0001f), hzToZeroToOne(0, INIT_XOVER_FREQS_5B[0]), AudioParameterFloatAttributes().withLabel("Hz").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([&](float value, [[maybe_unused]] int maximumStringLength) { return String(std::roundf(hzFromZeroToOne(0, value))); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"xOverF2", PD_PARAMETER_V1}, "Xover2", NormalisableRange<float>(0.0f, 1.0f, 0.0001f), hzToZeroToOne(1, INIT_XOVER_FREQS_5B[1]), AudioParameterFloatAttributes().withLabel("Hz").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([&](float value, [[maybe_unused]] int maximumStringLength) { return String(std::roundf(hzFromZeroToOne(1, value))); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"xOverF3", PD_PARAMETER_V1}, "Xover3", NormalisableRange<float>(0.0f, 1.0f, 0.0001f), hzToZeroToOne(2, INIT_XOVER_FREQS_5B[2]), AudioParameterFloatAttributes().withLabel("Hz").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([&](float value, [[maybe_unused]] int maximumStringLength) { return String(std::roundf(hzFromZeroToOne(2, value))); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"xOverF4", PD_PARAMETER_V1}, "Xover4", NormalisableRange<float>(0.0f, 1.0f, 0.0001f), hzToZeroToOne(3, INIT_XOVER_FREQS_5B[3]), AudioParameterFloatAttributes().withLabel("Hz").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([&](float value, [[maybe_unused]] int maximumStringLength) { return String(std::roundf(hzFromZeroToOne(3, value))); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"alpha1", PD_PARAMETER_V1}, "Polar1", NormalisableRange<float>(-0.5f, 1.0f, 0.01f), 0.0f, AudioParameterFloatAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 2); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"alpha2", PD_PARAMETER_V1}, "Polar2", NormalisableRange<float>(-0.5f, 1.0f, 0.01f), 0.0f, AudioParameterFloatAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 2); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"alpha3", PD_PARAMETER_V1}, "Polar3", NormalisableRange<float>(-0.5f, 1.0f, 0.01f), 0.0f, AudioParameterFloatAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 2); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"alpha4", PD_PARAMETER_V1}, "Polar4", NormalisableRange<float>(-0.5f, 1.0f, 0.01f), 0.0f, AudioParameterFloatAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 2); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"alpha5", PD_PARAMETER_V1}, "Polar5", NormalisableRange<float>(-0.5f, 1.0f, 0.01f), 0.0f, AudioParameterFloatAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 2); })),
              std::make_unique<AudioParameterBool>(ParameterID{"solo1", PD_PARAMETER_V1}, "Solo1", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"solo2", PD_PARAMETER_V1}, "Solo2", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"solo3", PD_PARAMETER_V1}, "Solo3", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"solo4", PD_PARAMETER_V1}, "Solo4", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"solo5", PD_PARAMETER_V1}, "Solo5", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"mute1", PD_PARAMETER_V1}, "Mute1", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"mute2", PD_PARAMETER_V1}, "Mute2", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"mute3", PD_PARAMETER_V1}, "Mute3", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"mute4", PD_PARAMETER_V1}, "Mute4", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"mute5", PD_PARAMETER_V1}, "Mute5", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterFloat>(ParameterID{"gain1", PD_PARAMETER_V1}, "Gain1", NormalisableRange<float>(-24.0f, 18.0f, 0.1f), 0.0f, AudioParameterFloatAttributes().withLabel("dB").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 1); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"gain2", PD_PARAMETER_V1}, "Gain2", NormalisableRange<float>(-24.0f, 18.0f, 0.1f), 0.0f, AudioParameterFloatAttributes().withLabel("dB").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 1); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"gain3", PD_PARAMETER_V1}, "Gain3", NormalisableRange<float>(-24.0f, 18.0f, 0.1f), 0.0f, AudioParameterFloatAttributes().withLabel("dB").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 1); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"gain4", PD_PARAMETER_V1}, "Gain4", NormalisableRange<float>(-24.0f, 18.0f, 0.1f), 0.0f, AudioParameterFloatAttributes().withLabel("dB").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 1); })),
              std::make_unique<AudioParameterFloat>(ParameterID{"gain5", PD_PARAMETER_V1}, "Gain5", NormalisableRange<float>(-24.0f, 18.0f, 0.1f), 0.0f, AudioParameterFloatAttributes().withLabel("dB").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return String(value, 1); })),
              std::make_unique<AudioParameterInt>(ParameterID{"nrBands", PD_PARAMETER_V1}, "Nr. of Bands", 0, 4, 4, AudioParameterIntAttributes().withLabel("").withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](int value, [[maybe_unused]] int maximumStringLength) { return String(value + 1); }).withAutomatable(false)),
              std::make_unique<AudioParameterBool>(ParameterID{"allowBackwardsPattern", PD_PARAMETER_V1}, "Allow Reverse Patterns", true, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; }).withAutomatable(false)),
              std::make_unique<AudioParameterFloat>(ParameterID{"proximity", PD_PARAMETER_V1}, "Proximity", NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f, AudioParameterFloatAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](float value, [[maybe_unused]] int maximumStringLength) { return std::abs(value) < 0.05f ? "0.00" : String(value, 2); })),
              std::make_unique<AudioParameterBool>(ParameterID{"proximityOnOff", PD_PARAMETER_V1}, "ProximityOnOff", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterBool>(ParameterID{"zeroLatencyMode", PD_PARAMETER_V1}, "Zero Latency", false, AudioParameterBoolAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](bool value, [[maybe_unused]] int maximumStringLength) { return value ? "on" : "off"; })),
              std::make_unique<AudioParameterInt>(ParameterID{"syncChannel", PD_PARAMETER_V1}, "Sync to Channel", 0, 4, 0, AudioParameterIntAttributes().withCategory(AudioProcessorParameter::genericParameter).withStringFromValueFunction([](int value, [[maybe_unused]] int maximumStringLength) { return value == 0 ? "none" : String(value); }).withAutomatable(false)),
          }),
      firLen(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE),
      dfEqOmniBuffer(1, DF_EQ_LEN),
      dfEqEightBuffer(1, DF_EQ_LEN),
      ffEqOmniBuffer(1, FF_EQ_LEN),
      ffEqEightBuffer(1, FF_EQ_LEN),
      convolversReady(false),
      delay(),
      delayBuffer(),
      nProcessorBandsPtr(vtsParams.getRawParameterValue("nrBands")),
      syncChannelPtr(vtsParams.getRawParameterValue("syncChannel")),
      xOverFreqsPtr{vtsParams.getRawParameterValue("xOverF1"),
          vtsParams.getRawParameterValue("xOverF2"),
          vtsParams.getRawParameterValue("xOverF3"),
          vtsParams.getRawParameterValue("xOverF4")},
      dirFactorsPtr{vtsParams.getRawParameterValue("alpha1"),
          vtsParams.getRawParameterValue("alpha2"),
          vtsParams.getRawParameterValue("alpha3"),
          vtsParams.getRawParameterValue("alpha4"),
          vtsParams.getRawParameterValue("alpha5")},
      oldDirFactors{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      bandGainsPtr{vtsParams.getRawParameterValue("gain1"),
          vtsParams.getRawParameterValue("gain2"),
          vtsParams.getRawParameterValue("gain3"),
          vtsParams.getRawParameterValue("gain4"),
          vtsParams.getRawParameterValue("gain5")},
      oldBandGains{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      allowBackwardsPatternPtr(vtsParams.getRawParameterValue("allowBackwardsPattern")),
      proxDistancePtr(vtsParams.getRawParameterValue("proximity")),
      proxOnOffPtr(vtsParams.getRawParameterValue("proximityOnOff")),
      zeroLatencyModePtr(vtsParams.getRawParameterValue("zeroLatencyMode")),
      soloBandPtr{vtsParams.getRawParameterValue("solo1"),
          vtsParams.getRawParameterValue("solo2"),
          vtsParams.getRawParameterValue("solo3"),
          vtsParams.getRawParameterValue("solo4"),
          vtsParams.getRawParameterValue("solo5")},
      muteBandPtr{vtsParams.getRawParameterValue("mute1"),
          vtsParams.getRawParameterValue("mute2"),
          vtsParams.getRawParameterValue("mute3"),
          vtsParams.getRawParameterValue("mute4"),
          vtsParams.getRawParameterValue("mute5")},
      trimPositionPtr(vtsParams.getRawParameterValue("trimPosition")),
      isBypassed(false),
      soloActive(false),
      loadingFile(false),
      readingSharedParams(false),
      trackingActive(false),
      trackingDisturber(false),
      nrBlocksRecorded(0),
      omniSqSumDist{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      eightSqSumDist{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      omniEightSumDist{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      omniSqSumSig{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      eightSqSumSig{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      omniEightSumSig{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      filterBankBuffer(),
      firFilterBuffer(),
      omniEightBuffer(),
      convolvers{},
      currentSampleRate(FILTER_BANK_NATIVE_SAMPLE_RATE),           // 48000
      previousSampleRate(FILTER_BANK_NATIVE_SAMPLE_RATE),          // 48000
      currentBlockSize(PD_DEFAULT_BLOCK_SIZE),
      lastDir()
{
    TRACE_DSP();

    vtsParams.addParameterListener ("trimPosition", this);

    vtsParams.addParameterListener ("xOverF1", this);
    vtsParams.addParameterListener ("xOverF2", this);
    vtsParams.addParameterListener ("xOverF3", this);
    vtsParams.addParameterListener ("xOverF4", this);

    if (auto* param = vtsParams.getParameter ("trimPosition"))
    {
        trimPositionPtr = vtsParams.getRawParameterValue ("trimPosition");
    }
    else
    {
        jassertfalse; // Log error or set default
        trimPositionPtr = nullptr;
    }

    xOverFreqsPtr[0] = vtsParams.getRawParameterValue ("xOverF1");
    xOverFreqsPtr[1] = vtsParams.getRawParameterValue ("xOverF2");
    xOverFreqsPtr[2] = vtsParams.getRawParameterValue ("xOverF3");
    xOverFreqsPtr[3] = vtsParams.getRawParameterValue ("xOverF4");
    for (int i = 0; i < MAX_NUM_EQS; ++i)
    {
        vtsParams.addParameterListener ("alpha" + String (i + 1), this);
        dirFactorsPtr[i] = vtsParams.getRawParameterValue ("alpha" + String (i + 1));

        vtsParams.addParameterListener ("solo" + String (i + 1), this);
        soloBandPtr[i] = vtsParams.getRawParameterValue ("solo" + String (i + 1));

        vtsParams.addParameterListener ("mute" + String (i + 1), this);
        muteBandPtr[i] = vtsParams.getRawParameterValue ("mute" + String (i + 1));

        vtsParams.addParameterListener ("gain" + String (i + 1), this);
        bandGainsPtr[i] = vtsParams.getRawParameterValue ("gain" + String (i + 1));
    }
    vtsParams.addParameterListener ("nrBands", this);
    nProcessorBandsPtr = vtsParams.getRawParameterValue ("nrBands");

    vtsParams.addParameterListener ("allowBackwardsPattern", this);
    vtsParams.getRawParameterValue ("allowBackwardsPattern");

    vtsParams.addParameterListener ("proximity", this);
    proxDistancePtr = vtsParams.getRawParameterValue ("proximity");
    vtsParams.addParameterListener ("proximityOnOff", this);
    proxOnOffPtr = vtsParams.getRawParameterValue ("proximityOnOff");
    vtsParams.addParameterListener ("zeroLatencyMode", this);
    zeroLatencyModePtr = vtsParams.getRawParameterValue ("zeroLatencyMode");
    vtsParams.addParameterListener ("syncChannel", this);
    syncChannelPtr = vtsParams.getRawParameterValue ("syncChannel");

    // properties file: saves user preset folder location
    PropertiesFile::Options options;
    options.applicationName = "PolarDesigner";
    options.filenameSuffix = "settings";
    options.folderName = "AustrianAudio";
    options.osxLibrarySubFolder = "Preferences";

    properties = std::unique_ptr<PropertiesFile> (new PropertiesFile (options));
    lastDir = File (properties->getValue ("presetFolder"));

    dfEqOmniBuffer.copyFrom (0, 0, DFEQ_COEFFS_OMNI, DF_EQ_LEN);
    dfEqEightBuffer.copyFrom (0, 0, DFEQ_COEFFS_EIGHT, DF_EQ_LEN);
    ffEqOmniBuffer.copyFrom (0, 0, FFEQ_COEFFS_OMNI, FF_EQ_LEN);
    ffEqEightBuffer.copyFrom (0, 0, FFEQ_COEFFS_EIGHT, FF_EQ_LEN);

    updateLatency();

    //    delay.setDelayTime (std::ceilf (static_cast<float> (FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE) / 2 - 1) / FILTER_BANK_NATIVE_SAMPLE_RATE);
    delay.setDelayTime(static_cast<float> (static_cast<float> ((static_cast<float> (firLen) - 1.0f) / 2.0f) / currentSampleRate));

    oldProxDistance = proxDistancePtr->load();

    termControlWaveform.setRepaintRate (30);
    termControlWaveform.setBufferSize (256);

    // !J! Some DAW's (Cubase) do not prepareToPlay before setStateInformation, so these buffers need to be ready at least, even if empty
    filterBankBuffer.setSize (N_CH_IN * MAX_NUM_EQS, currentBlockSize);
    filterBankBuffer.clear();
    firFilterBuffer.setSize (MAX_NUM_EQS, firLen);
    firFilterBuffer.clear();
    omniEightBuffer.setSize (MAX_NUM_INPUTS, currentBlockSize);
    omniEightBuffer.clear();

    resetTrackingState();

    startTimer (50);
}

#if 0
void PolarDesignerAudioProcessor::registerParameterListeners() {
    static const StringArray paramIDs = {
        "trimPosition", "xOverF1", "xOverF2", "xOverF3", "xOverF4",
        "alpha1", "alpha2", "alpha3", "alpha4", "alpha5",
        "solo1", "solo2", "solo3", "solo4", "solo5",
        "mute1", "mute2", "mute3", "mute4", "mute5",
        "gain1", "gain2", "gain3", "gain4", "gain5",
        "nrBands", "allowBackwardsPattern", "proximity", "proximityOnOff", "zeroLatencyMode", "syncChannel"
    };
    for (const auto& id : paramIDs) {
        vtsParams.addParameterListener(id, this);
    }
}
#endif

PolarDesignerAudioProcessor::~PolarDesignerAudioProcessor()
{

    // Remove listeners for parameters
    for (auto* param : getParameters())
    {
        if (auto* rangedParam = dynamic_cast<RangedAudioParameter*>(param))
        {
            vtsParams.removeParameterListener(rangedParam->paramID, this);
        }
    }

#if PERFETTO
    tracingSession.endSession();
#endif
}

//==============================================================================
const String PolarDesignerAudioProcessor::getName() const
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

const String PolarDesignerAudioProcessor::getProgramName ([[maybe_unused]] int index)
{
    return {};
}

void PolarDesignerAudioProcessor::changeProgramName ([[maybe_unused]] int index, [[maybe_unused]] const String& newName)
{
}



void PolarDesignerAudioProcessor::loadEqImpulseResponses() {
    float ratio = static_cast<float> (currentSampleRate / (FILTER_BANK_NATIVE_SAMPLE_RATE * 1.0f));
    int newFFEqLen = static_cast<int>(FF_EQ_LEN * ratio);
    int newDFEqLen = static_cast<int>(DF_EQ_LEN * ratio);
    if (newFFEqLen > firLen) newFFEqLen = firLen;
    if (newDFEqLen > firLen) newDFEqLen = firLen;

    // Reuse static buffers to avoid repeated allocations
    static AudioBuffer<float> resampledFFEqOmni(1, firLen);
    static AudioBuffer<float> resampledFFEqEight(1, firLen);
    static AudioBuffer<float> resampledDFEqOmni(1, firLen);
    static AudioBuffer<float> resampledDFEqEight(1, firLen);

    resampledFFEqOmni.clear();
    resampledFFEqEight.clear();
    resampledDFEqOmni.clear();
    resampledDFEqEight.clear();

#ifdef USE_LINEAR_INTERPOLATION_RESAMPLING
    // Linear interpolation for resampling
    for (int i = 0; i < newFFEqLen; ++i) {
        float srcIdx = i / ratio;
        int idxFloor = static_cast<int>(srcIdx);
        float frac = srcIdx - idxFloor;
        if (idxFloor < FF_EQ_LEN - 1) {
            float sample0 = ffEqOmniBuffer.getSample(0, idxFloor);
            float sample1 = ffEqOmniBuffer.getSample(0, idxFloor + 1);
            resampledFFEqOmni.setSample(0, i, sample0 + frac * (sample1 - sample0));

            sample0 = ffEqEightBuffer.getSample(0, idxFloor);
            sample1 = ffEqEightBuffer.getSample(0, idxFloor + 1);
            resampledFFEqEight.setSample(0, i, sample0 + frac * (sample1 - sample0));
        } else if (idxFloor < FF_EQ_LEN) {
            resampledFFEqOmni.setSample(0, i, ffEqOmniBuffer.getSample(0, idxFloor));
            resampledFFEqEight.setSample(0, i, ffEqEightBuffer.getSample(0, idxFloor));
        }
    }

    for (int i = 0; i < newDFEqLen; ++i) {
        float srcIdx = i / ratio;
        int idxFloor = static_cast<int>(srcIdx);
        float frac = srcIdx - idxFloor;
        if (idxFloor < DF_EQ_LEN - 1) {
            float sample0 = dfEqOmniBuffer.getSample(0, idxFloor);
            float sample1 = dfEqOmniBuffer.getSample(0, idxFloor + 1);
            resampledDFEqOmni.setSample(0, i, sample0 + frac * (sample1 - sample0));

            sample0 = dfEqEightBuffer.getSample(0, idxFloor);
            sample1 = dfEqEightBuffer.getSample(0, idxFloor + 1);
            resampledDFEqEight.setSample(0, i, sample0 + frac * (sample1 - sample0));
        } else if (idxFloor < DF_EQ_LEN) {
            resampledDFEqOmni.setSample(0, i, dfEqOmniBuffer.getSample(0, idxFloor));
            resampledDFEqEight.setSample(0, i, dfEqEightBuffer.getSample(0, idxFloor));
        }
    }
#else
    if (previousSampleRate != currentSampleRate)
    {
        resampleBuffer (ffEqOmniBuffer, resampledFFEqOmni, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
        resampleBuffer (ffEqEightBuffer, resampledFFEqEight, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
        resampleBuffer (dfEqOmniBuffer, resampledDFEqOmni, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
        resampleBuffer (dfEqEightBuffer, resampledDFEqEight, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);

        previousSampleRate = currentSampleRate;
    }
#endif

    // Center impulse responses
    int ffEqOffset = (firLen - newFFEqLen) / 2;
    int dfEqOffset = (firLen - newDFEqLen) / 2;

    AudioBuffer<float> paddedFFEqOmni(1, firLen);
    AudioBuffer<float> paddedFFEqEight(1, firLen);
    AudioBuffer<float> paddedDFEqOmni(1, firLen);
    AudioBuffer<float> paddedDFEqEight(1, firLen);

    paddedFFEqOmni.clear();
    paddedFFEqEight.clear();
    paddedDFEqOmni.clear();
    paddedDFEqEight.clear();

    if (newFFEqLen <= firLen) {
        paddedFFEqOmni.copyFrom(0, ffEqOffset, resampledFFEqOmni, 0, 0, newFFEqLen);
        paddedFFEqEight.copyFrom(0, ffEqOffset, resampledFFEqEight, 0, 0, newFFEqLen);
    } else {
        jassertfalse;
        paddedFFEqOmni.copyFrom(0, 0, resampledFFEqOmni, 0, 0, firLen);
        paddedFFEqEight.copyFrom(0, 0, resampledFFEqEight, 0, 0, firLen);
    }

    if (newDFEqLen <= firLen) {
        paddedDFEqOmni.copyFrom(0, dfEqOffset, resampledDFEqOmni, 0, 0, newDFEqLen);
        paddedDFEqEight.copyFrom(0, dfEqOffset, resampledDFEqEight, 0, 0, newDFEqLen);
    } else {
        jassertfalse;
        paddedDFEqOmni.copyFrom(0, 0, resampledDFEqOmni, 0, 0, firLen);
        paddedDFEqEight.copyFrom(0, 0, resampledDFEqEight, 0, 0, firLen);
    }

    // Load impulse responses without re-preparing
    dfEqOmniConv.loadImpulseResponse(std::move(paddedDFEqOmni), currentSampleRate, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    dfEqEightConv.loadImpulseResponse(std::move(paddedDFEqEight), currentSampleRate, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    ffEqOmniConv.loadImpulseResponse(std::move(paddedFFEqOmni), currentSampleRate, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    ffEqEightConv.loadImpulseResponse(std::move(paddedFFEqEight), currentSampleRate, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
}

#ifdef USE_NEW_PREPARETOPLAY
void PolarDesignerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    TRACE_EVENT("dsp", "prepareToPlay", "sampleRate:", sampleRate, "samplesPerBlock:", samplesPerBlock);

    jassert(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE > 0);

    firLen = static_cast<int>(std::ceil(
        static_cast<float>(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE) / FILTER_BANK_NATIVE_SAMPLE_RATE * sampleRate));
    if (firLen % 2 == 0) firLen++; // Ensure odd length for linear-phase FIR filters
    jassert(firLen % 2 == 1);

    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    if (samplesPerBlock > currentBlockSize)
    {
        currentBlockSize = samplesPerBlock;
        filterBankBuffer.setSize(N_CH_IN * MAX_NUM_EQS, currentBlockSize, true);
        omniEightBuffer.setSize(MAX_NUM_INPUTS, currentBlockSize, true);
        delayBuffer.setSize(1, currentBlockSize, true);
        LOG_ERROR("Resized buffers to accommodate larger block size:" + String(samplesPerBlock));
    }

    // Update latency
    updateLatency();

    // Configure ProcessSpec
    dsp::ProcessSpec eqSpec{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    dsp::ProcessSpec convSpec{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};

    // Prepare EQ convolvers only if spec has changed
    if (eqSpec != lastEqSpec) {
        dfEqOmniConv.prepare(eqSpec);
        dfEqEightConv.prepare(eqSpec);
        ffEqOmniConv.prepare(eqSpec);
        ffEqEightConv.prepare(eqSpec);
        lastEqSpec = eqSpec;
    }

    // Prepare filter bank convolvers only if spec has changed
    if (convSpec != lastConvSpec) {
        for (auto &conv : convolvers) {
            conv.prepare(convSpec);
        }
        lastConvSpec = convSpec;
    }

    // Configure delay line
    dsp::ProcessSpec delaySpec{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    delay.prepare(delaySpec);
    delay.setDelayTime(static_cast<float>((firLen - 1.0f) / 2.0f) / currentSampleRate);
    delayBuffer.setSize(1, currentBlockSize);
    delayBuffer.clear();

    // Resize buffers
    filterBankBuffer.setSize(N_CH_IN * MAX_NUM_EQS, currentBlockSize);
    filterBankBuffer.clear();
    firFilterBuffer.setSize(MAX_NUM_EQS, firLen);
    firFilterBuffer.clear();
    omniEightBuffer.setSize(MAX_NUM_INPUTS, currentBlockSize);
    omniEightBuffer.clear();

    // Resample and load EQ impulse responses
    loadEqImpulseResponses();

    // Compute and load filter bank coefficients
    computeAllFilterCoefficients();
    loadFilterBankImpulseResponses();

    // Configure proximity compensation IIR
    dsp::ProcessSpec specProx{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    proxCompIIR.prepare(specProx);
    proxCompIIR.reset();
    setProxCompCoefficients(proxDistancePtr->load());
}
#else

    #ifndef USE_LINEAR_INTERPOLATION_RESAMPLING
void PolarDesignerAudioProcessor::resampleBuffer(const AudioBuffer<float>& src, AudioBuffer<float>& dst, float srcSampleRate, float dstSampleRate)
{
    // Ensure source and destination buffers are valid
    jassert(src.getNumChannels() == dst.getNumChannels());
    jassert(src.getNumSamples() > 0 && dst.getNumSamples() > 0);

    // Calculate resampling ratio
    const double ratio = static_cast<double>(dstSampleRate) / srcSampleRate;
    const int numChannels = src.getNumChannels();
    //    const int srcNumSamples = src.getNumSamples();
    const int dstNumSamples = dst.getNumSamples();

    // Create Lagrange interpolator
    LagrangeInterpolator interpolator;

    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* srcData = src.getReadPointer(channel);
        float* dstData = dst.getWritePointer(channel);

        // Resample using Lagrange interpolator
        interpolator.process(ratio, srcData, dstData, dstNumSamples);
    }
}
    #endif


void PolarDesignerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {

    TRACE_EVENT("dsp", "prepareToPlay", "sampleRate:", sampleRate, "samplesPerBlock:", samplesPerBlock);

    // Validate FIR filter length initialization
    jassert(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE > 0);

    firLen = static_cast<int>(std::ceil(
        static_cast<float>(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE) / FILTER_BANK_NATIVE_SAMPLE_RATE * sampleRate));

    if (firLen % 2 == 0) // Ensure firLen is odd for linear-phase FIR filters
        firLen++;

    jassert(firLen % 2 == 1); // Double-check odd length

    // Update current sample rate and block size
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Update latency
    updateLatency();

    // Configure and prepare delay line
    dsp::ProcessSpec delaySpec{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    delay.prepare(delaySpec);
    delay.setDelayTime(static_cast<float> (static_cast<float> ((firLen - 1.0f) / 2.0f) / currentSampleRate)); // Set delay to exact FIR group delay
    delayBuffer.clear();
    delayBuffer.setSize(1, currentBlockSize);

    if (currentSampleRate != sampleRate || currentBlockSize != samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;
        // Resize and clear buffers
        filterBankBuffer.setSize(N_CH_IN * MAX_NUM_EQS, currentBlockSize, false, true, true);
        firFilterBuffer.setSize(MAX_NUM_EQS, firLen, false, true, true);
        omniEightBuffer.setSize(MAX_NUM_INPUTS, currentBlockSize, false, true, true);

        // Resize buffers
        filterBankBuffer.clear();
        firFilterBuffer.clear();
        omniEightBuffer.clear();

        // Reinitialize convolvers
//        loadEqImpulseResponses();
        computeAllFilterCoefficients();
        loadFilterBankImpulseResponses();
    }


    for (auto &conv : convolvers) {
        conv.reset();
    }

    // Resample EQ impulse responses to current sample rate
    float ratio = static_cast<float> (currentSampleRate / (FILTER_BANK_NATIVE_SAMPLE_RATE * 1.0f));
    int newFFEqLen = static_cast<int>(FF_EQ_LEN * ratio);
    int newDFEqLen = static_cast<int>(DF_EQ_LEN * ratio);
    if (newFFEqLen > firLen) newFFEqLen = firLen; // Truncate if too long
    if (newDFEqLen > firLen) newDFEqLen = firLen;

    AudioBuffer<float> resampledFFEqOmni(1, newFFEqLen);
    AudioBuffer<float> resampledFFEqEight(1, newFFEqLen);
    AudioBuffer<float> resampledDFEqOmni(1, newDFEqLen);
    AudioBuffer<float> resampledDFEqEight(1, newDFEqLen);

    resampledFFEqOmni.clear();
    resampledFFEqEight.clear();
    resampledDFEqOmni.clear();
    resampledDFEqEight.clear();

    #ifdef USE_LINEAR_INTERPOLATION_RESAMPLING
    // Linear interpolation for resampling
    for (int i = 0; i < newFFEqLen; ++i) {
        float srcIdx = i / ratio;
        int idxFloor = static_cast<int>(srcIdx);
        float frac = srcIdx - idxFloor;
        if (idxFloor < FF_EQ_LEN - 1) {
            float sample0 = ffEqOmniBuffer.getSample(0, idxFloor);
            float sample1 = ffEqOmniBuffer.getSample(0, idxFloor + 1);
            resampledFFEqOmni.setSample(0, i, sample0 + frac * (sample1 - sample0));

            sample0 = ffEqEightBuffer.getSample(0, idxFloor);
            sample1 = ffEqEightBuffer.getSample(0, idxFloor + 1);
            resampledFFEqEight.setSample(0, i, sample0 + frac * (sample1 - sample0));
        } else if (idxFloor < FF_EQ_LEN) {
            resampledFFEqOmni.setSample(0, i, ffEqOmniBuffer.getSample(0, idxFloor));
            resampledFFEqEight.setSample(0, i, ffEqEightBuffer.getSample(0, idxFloor));
        }
    }

    for (int i = 0; i < newDFEqLen; ++i) {
        float srcIdx = i / ratio;
        int idxFloor = static_cast<int>(srcIdx);
        float frac = srcIdx - idxFloor;
        if (idxFloor < DF_EQ_LEN - 1) {
            float sample0 = dfEqOmniBuffer.getSample(0, idxFloor);
            float sample1 = dfEqOmniBuffer.getSample(0, idxFloor + 1);
            resampledDFEqOmni.setSample(0, i, sample0 + frac * (sample1 - sample0));

            sample0 = dfEqEightBuffer.getSample(0, idxFloor);
            sample1 = dfEqEightBuffer.getSample(0, idxFloor + 1);
            resampledDFEqEight.setSample(0, i, sample0 + frac * (sample1 - sample0));
        } else if (idxFloor < DF_EQ_LEN) {
            resampledDFEqOmni.setSample(0, i, dfEqOmniBuffer.getSample(0, idxFloor));
            resampledDFEqEight.setSample(0, i, dfEqEightBuffer.getSample(0, idxFloor));
        }
    }
    #else
    // !J! - Note, prefer JUCE' instead.
    resampleBuffer(ffEqOmniBuffer, resampledFFEqOmni, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
    resampleBuffer(ffEqEightBuffer, resampledFFEqEight, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
    resampleBuffer(dfEqOmniBuffer, resampledDFEqOmni, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
    resampleBuffer(dfEqEightBuffer, resampledDFEqEight, FILTER_BANK_NATIVE_SAMPLE_RATE, currentSampleRate);
    #endif

    // Pad resampled EQ impulse responses to match firLen
    AudioBuffer<float> paddedFFEqOmni(1, firLen);
    AudioBuffer<float> paddedFFEqEight(1, firLen);
    AudioBuffer<float> paddedDFEqOmni(1, firLen);
    AudioBuffer<float> paddedDFEqEight(1, firLen);

    paddedFFEqOmni.clear();
    paddedFFEqEight.clear();
    paddedDFEqOmni.clear();
    paddedDFEqEight.clear();

    // Center the resampled EQ impulse responses to align group delay with (firLen - 1) / 2
    int ffEqOffset = (firLen - newFFEqLen) / 2;
    int dfEqOffset = (firLen - newDFEqLen) / 2;

    if (newFFEqLen <= firLen) {
        paddedFFEqOmni.copyFrom(0, ffEqOffset, resampledFFEqOmni, 0, 0, newFFEqLen);
        paddedFFEqEight.copyFrom(0, ffEqOffset, resampledFFEqEight, 0, 0, newFFEqLen);
    } else {
        jassertfalse; // Log warning: resampled length exceeds firLen
        paddedFFEqOmni.copyFrom(0, 0, resampledFFEqOmni, 0, 0, firLen);
        paddedFFEqEight.copyFrom(0, 0, resampledFFEqEight, 0, 0, firLen);
    }

    if (newDFEqLen <= firLen) {
        paddedDFEqOmni.copyFrom(0, dfEqOffset, resampledDFEqOmni, 0, 0, newDFEqLen);
        paddedDFEqEight.copyFrom(0, dfEqOffset, resampledDFEqEight, 0, 0, newDFEqLen);
    } else {
        jassertfalse; // Log warning: resampled length exceeds firLen
        paddedDFEqOmni.copyFrom(0, 0, resampledDFEqOmni, 0, 0, firLen);
        paddedDFEqEight.copyFrom(0, 0, resampledDFEqEight, 0, 0, firLen);
    }

    // Configure and load EQ convolutions
    dsp::ProcessSpec eqSpec{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};

    // !J! TODO: Evaluate if this is needed for ProTools/Nuendo?
    //dfEqOmniConv.prepare({currentSampleRate, static_cast<uint32>(currentBlockSize), 1, dsp::Convolution::LatencyType::nonUniform});

    dfEqOmniConv.prepare(eqSpec);
    dfEqOmniConv.loadImpulseResponse(std::move(paddedDFEqOmni), currentSampleRate, dsp::Convolution::Stereo::no,
        dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

    dfEqEightConv.prepare(eqSpec);
    dfEqEightConv.loadImpulseResponse(std::move(paddedDFEqEight), currentSampleRate, dsp::Convolution::Stereo::no,
        dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

    ffEqOmniConv.prepare(eqSpec);
    ffEqOmniConv.loadImpulseResponse(std::move(paddedFFEqOmni), currentSampleRate, dsp::Convolution::Stereo::no,
        dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

    ffEqEightConv.prepare(eqSpec);
    ffEqEightConv.loadImpulseResponse(std::move(paddedFFEqEight), currentSampleRate, dsp::Convolution::Stereo::no,
        dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

    // Store old parameter values
    for (int i = 0; i < MAX_NUM_EQS; ++i) {
        oldDirFactors[i] = dirFactorsPtr[i]->load();
        oldBandGains[i] = bandGainsPtr[i]->load();
    }

    // Configure proximity compensation IIR
    dsp::ProcessSpec specProx{currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    proxCompIIR.prepare(specProx);
    proxCompIIR.reset();
    setProxCompCoefficients(proxDistancePtr->load());
}
#endif


bool PolarDesignerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
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

void PolarDesignerAudioProcessor::processBlock (AudioBuffer<float>& buffer, [[maybe_unused]] MidiBuffer& midiMessages)
{
    TRACE_DSP();

    ScopedNoDenormals noDenormals;
    unsigned int numSamples = (unsigned int) buffer.getNumSamples();

    // glitch mitigation:
    if (currentSampleRate <= 0.0 || currentBlockSize <= 0) {
        LOG_ERROR("Invalid sample rate or block size");
        buffer.clear();
        return;
    }

    // some DAW's call setStateInformation/processBlock before prepareToPlay - mitigate side-effects in this case:
    if (currentBlockSize == 0)
        initializeBuffers();

    if (isBypassed)
    {
        isBypassed = false;
        // updateLatency();
    }

    recomputeFilterCoefficientsIfNeeded();

    // create omni and eight signals
    createOmniAndEightSignals (buffer);

    // proximity compensation filter
    auto proximity = (std::round(proxOnOffPtr->load()) > 0.5f) ? proxDistancePtr->load() : 0.f;
    if ((std::round(zeroLatencyModePtr->load()) < 0.5f) && (proximity < -0.05)) // reduce proximity effect only on figure-of-eight
    {
        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> eightBlock (&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> contextProxEight (eightBlock);
        proxCompIIR.process (contextProxEight);
    }
    else if ((std::round(zeroLatencyModePtr->load()) < 0.5f) && (proximity > 0.05)) // apply proximity to omni
    {
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> omniBlock (&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> contextProxOmni (omniBlock);
        proxCompIIR.process (contextProxOmni);
    }

    if ((doEq == 1) && (std::round(zeroLatencyModePtr->load()) < 0.5f))
    {
        // free field equalization
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> ffEqOmniBlk (&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> ffEqOmniCtx (ffEqOmniBlk);
        ffEqOmniConv.process (ffEqOmniCtx);

        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> ffEqEightBlk (&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> ffEqEightCtx (ffEqEightBlk);
        ffEqEightConv.process (ffEqEightCtx);
    }
    else if ((doEq == 2) && (std::round(zeroLatencyModePtr->load()) < 0.5f))
    {
        // diffuse field equalization
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> dfEqOmniBlk (&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> dfEqOmniCtx (dfEqOmniBlk);
        dfEqOmniConv.process (dfEqOmniCtx);

        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> dfEqEightBlk (&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> dfEqEightCtx (dfEqEightBlk);
        dfEqEightConv.process (dfEqEightCtx);
    }

    int nActiveBands = static_cast<int> (nProcessorBands);

    // 1-band EQ
    if (std::round(zeroLatencyModePtr->load()) > 0.5f)
        nActiveBands = 1;

    for (int i = 0; i < nActiveBands; ++i)
    {
        // copy input buffer for each band
        filterBankBuffer.copyFrom (2 * i, 0, omniEightBuffer, 0, 0, static_cast<int> (numSamples));
        filterBankBuffer.copyFrom (2 * i + 1, 0, omniEightBuffer, 1, 0, static_cast<int> (numSamples));
    }

    // 5-band EQ
    if ((std::round(zeroLatencyModePtr->load()) < 0.5f) && (nActiveBands > 1))
    {
        if (!convolversReady)
        {
            return;
        }

        for (int i = 0; i < nActiveBands; ++i)
        {
            // omni
            float* writePointerOmni = filterBankBuffer.getWritePointer (2 * i);
            dsp::AudioBlock<float> subBlk (&writePointerOmni, 1, numSamples);
            dsp::ProcessContextReplacing<float> filterCtx (subBlk);
            convolvers[2 * i].process (filterCtx); // mono processing

            // eight
            float* writePointerEight = filterBankBuffer.getWritePointer (2 * i + 1);
            dsp::AudioBlock<float> subBlk2 (&writePointerEight, 1, numSamples);
            dsp::ProcessContextReplacing<float> filterCtx2 (subBlk2);
            convolvers[2 * i + 1].process (filterCtx2); // mono processing
        }
        convolversReady = true;
    }

    audioPlayHead = getPlayHead();
    if (audioPlayHead != nullptr)
    {
        if (auto position = audioPlayHead->getPosition())
        {
            playHeadPosition = *position;
        }
    }

    termControlWaveform.pushBuffer (buffer);

    if (trackingActive)
        trackSignalEnergy();

    createPolarPatterns (buffer);
}

void PolarDesignerAudioProcessor::processBlockBypassed (AudioBuffer<float>& buffer,
    [[maybe_unused]] MidiBuffer& midiMessages)
{

    TRACE_DSP();

    if (!isBypassed)
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

AudioProcessorEditor* PolarDesignerAudioProcessor::createEditor()
{
    TRACE_DSP();

    return new PolarDesignerAudioProcessorEditor (*this, vtsParams);
}

//==============================================================================

int PolarDesignerAudioProcessor::getSyncChannelIdx()
{
    return static_cast<int> (*syncChannelPtr);
}

// getStateInformation: Ensure consistent updates for layerA and layerB
void PolarDesignerAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // Update vtsParams.state properties
    vtsParams.state.setProperty("ffDfEq", var(doEq), nullptr);
    vtsParams.state.setProperty("oldProxDistance", var(oldProxDistance), nullptr);
    vtsParams.state.setProperty("ABLayer", abLayerState, nullptr);
    vtsParams.state.setProperty("oldNrBands", var(oldNrBands), nullptr);

    // Update the appropriate layer based on abLayerState
    if (abLayerState == COMPARE_LAYER_A)
    {
        layerA = vtsParams.copyState();
        doEqA = doEq;
        if (proxDistancePtr && !exactlyEqual(std::round(proxDistancePtr->load()), 0.0f))
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
        doEqB = doEq;
        if (proxDistancePtr && !exactlyEqual(std::round(proxDistancePtr->load()), 0.0f))
        {
            oldProxDistanceB = proxDistancePtr->load();
        }
        if (nProcessorBandsPtr)
        {
            oldNrBandsB = nProcessorBandsPtr->load();
        }
    }

    // Update layer properties
    layerA.setProperty("ffDfEq", var(doEqA), nullptr);
    layerA.setProperty("oldProxDistance", var(oldProxDistanceA), nullptr);
    layerA.setProperty("ABLayer", COMPARE_LAYER_A, nullptr);
    layerA.setProperty("oldNrBands", var(oldNrBandsA), nullptr);

    layerB.setProperty("ffDfEq", var(doEqB), nullptr);
    layerB.setProperty("oldProxDistance", var(oldProxDistanceB), nullptr);
    layerB.setProperty("ABLayer", COMPARE_LAYER_B, nullptr);
    layerB.setProperty("oldNrBands", var(oldNrBandsB), nullptr);

    // Store all states in saveStates
    saveStates.removeAllChildren(nullptr);
    saveStates.addChild(vtsParams.copyState(), 0, nullptr);
    saveStates.addChild(layerA.createCopy(), 1, nullptr);
    saveStates.addChild(layerB.createCopy(), 2, nullptr);

    std::unique_ptr<XmlElement> xml(saveStates.createXml());
    copyXmlToBinary(*xml, destData);

#ifdef USE_EXTRA_DEBUG_DUMPS
    juce::String treeAsXmlString = saveStates.toXmlString();
    LOG_DEBUG(treeAsXmlString);
#endif
}

// !J! Make setStateInformation more robust for ProTools
void PolarDesignerAudioProcessor::initializeDefaultState() {

    TRACE_DSP();

    // Reinitialize ValueTree state
    vtsParams.state = ValueTree("AAPolarDesigner");
    layerA = vtsParams.copyState();
    layerB = vtsParams.copyState();
    abLayerState = COMPARE_LAYER_A;

    // Reset non-parameter state variables
    doEq = doEqA = doEqB = 0;
    oldProxDistance = oldProxDistanceA = oldProxDistanceB = 0.0f;
    oldNrBands = oldNrBandsA = oldNrBandsB = 4.0f;
    nProcessorBands = 5; // Ensure internal consistency
    soloActive = false;
    loadingFile = false;
    readingSharedParams = false;

    recomputeAllFilterCoefficients = true;

    for (auto& flag : recomputeFilterCoefficients) flag = false;
    activeBandsChanged = true;
    zeroLatencyModeChanged = true;
    ffDfEqChanged = true;
    repaintDEQ = true;

    // Reset tracking state
    resetTrackingState();

    // Reset parameters to their default values using stored defaults
    // Define default values based on vtsParams initialization
    // Reset parameters to their default values using stored defaults
    // Define default values based on vtsParams initialization
    std::map<String, float> defaultValues = {
        {"trimPosition", 0.0f}, // Default from constructor: 0.0f
        {"xOverF1", hzToZeroToOne(0, INIT_XOVER_FREQS_5B[0])}, // Band 1: Matches constructor, assuming INIT_XOVER_FREQS_5B[0] = 100 Hz
        {"xOverF2", hzToZeroToOne(1, INIT_XOVER_FREQS_5B[1])}, // Band 2: Matches constructor, assuming INIT_XOVER_FREQS_5B[1] = 315 Hz
        {"xOverF3", hzToZeroToOne(2, INIT_XOVER_FREQS_5B[2])}, // Band 3: Matches constructor, assuming INIT_XOVER_FREQS_5B[2] = 1000 Hz
        {"xOverF4", hzToZeroToOne(3, INIT_XOVER_FREQS_5B[3])}, // Band 4: Matches constructor, assuming INIT_XOVER_FREQS_5B[3] = 3150 Hz
        {"alpha1", 0.0f}, // Default from constructor: 0.0f (Cardioid)
        {"alpha2", 0.0f}, // Default from constructor: 0.0f
        {"alpha3", 0.0f}, // Default from constructor: 0.0f
        {"alpha4", 0.0f}, // Default from constructor: 0.0f
        {"alpha5", 0.0f}, // Default from constructor: 0.0f
        {"solo1", 0.0f}, // Default from constructor: false (0.0f)
        {"solo2", 0.0f}, // Default from constructor: false (0.0f)
        {"solo3", 0.0f}, // Default from constructor: false (0.0f)
        {"solo4", 0.0f}, // Default from constructor: false (0.0f)
        {"solo5", 0.0f}, // Default from constructor: false (0.0f)
        {"mute1", 0.0f}, // Default from constructor: false (0.0f)
        {"mute2", 0.0f}, // Default from constructor: false (0.0f)
        {"mute3", 0.0f}, // Default from constructor: false (0.0f)
        {"mute4", 0.0f}, // Default from constructor: false (0.0f)
        {"mute5", 0.0f}, // Default from constructor: false (0.0f)
        {"gain1", 0.0f}, // Default from constructor: 0.0f (0 dB)
        {"gain2", 0.0f}, // Default from constructor: 0.0f
        {"gain3", 0.0f}, // Default from constructor: 0.0f
        {"gain4", 0.0f}, // Default from constructor: 0.0f
        {"gain5", 0.0f}, // Default from constructor: 0.0f
        {"nrBands", 4.0f}, // Default from constructor: 4 (5 bands, 0-based index)
        {"allowBackwardsPattern", 1.0f}, // Default from constructor: false (0.0f)
        {"proximity", 0.0f}, // Default from constructor: 0.0f
        {"proximityOnOff", 0.0f}, // Default from constructor: false (0.0f)
        {"zeroLatencyMode", 0.0f}, // Default from constructor: false (0.0f)
        {"syncChannel", 0.0f} // Default from constructor: 0
    };

    // Apply default values to parameters
    for (const auto& [paramID, defaultValue] : defaultValues) {
        if (auto* param = vtsParams.getParameter(paramID)) {
            param->setValueNotifyingHost(defaultValue);
#ifdef USE_EXTRA_DEBUG_DUMPS
            LOG_DEBUG("Reset parameter " + paramID + " to " + String(defaultValue));
#endif
        }
#ifdef USE_EXTRA_DEBUG_DUMPS
        else {
            LOG_DEBUG("Parameter not found: " + paramID);
        }
#endif
    }

    // Initialize buffers (ensure compatibility with ProTools)
    if (currentBlockSize == 0) currentBlockSize = PD_DEFAULT_BLOCK_SIZE; // Default block size
    if (currentSampleRate == 0.0f) currentSampleRate = FILTER_BANK_NATIVE_SAMPLE_RATE; // Default sample rate

    filterBankBuffer.setSize(N_CH_IN * MAX_NUM_EQS, currentBlockSize, false, false, true);
    firFilterBuffer.setSize(MAX_NUM_EQS, firLen, false, false, true);
    omniEightBuffer.setSize(MAX_NUM_INPUTS, currentBlockSize, false, false, true);

    for (int i = 0; i < MAX_NUM_EQS; ++i) {
        oldDirFactors[i] = 0.0f; // Default to cardioid
        oldBandGains[i] = 0.0f; // Default to 0 dB
    }

    // Always true:
    vtsParams.getParameter("allowBackwardsPattern")->setValueNotifyingHost(1.0f);

}


void PolarDesignerAudioProcessor::initializeBuffers()
{
    if (currentBlockSize == 0)
        currentBlockSize = PD_DEFAULT_BLOCK_SIZE;
    if (currentSampleRate == 0.0f)
        currentSampleRate = FILTER_BANK_NATIVE_SAMPLE_RATE;

    prepareToPlay (currentSampleRate, currentBlockSize); // Ensure proper DSP setup

    filterBankBuffer.setSize (N_CH_IN * MAX_NUM_EQS, currentBlockSize, false, true, true);
    firFilterBuffer.setSize (MAX_NUM_EQS, firLen, false, true, true);
    omniEightBuffer.setSize (MAX_NUM_INPUTS, currentBlockSize, false, true, true);
    filterBankBuffer.clear();
    firFilterBuffer.clear();
    omniEightBuffer.clear();
}

#ifdef USE_NEW_SETSTATEINFORMATION
void PolarDesignerAudioProcessor::releaseResources()
{
    resetTrackingState();
    filterBankBuffer.clear();
    firFilterBuffer.clear();
    omniEightBuffer.clear();
}

void PolarDesignerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState == nullptr)
        return; // Invalid state data

    resetTrackingState(); // Clear tracking data

    // Load saveStates and restore abLayerState first
    if (xmlState->hasTagName(saveStates.getType()))
    {
        saveStates = ValueTree::fromXml(*xmlState);
        // Validate children
        if (saveStates.getNumChildren() >= 3)
        {
            layerA = saveStates.getChild(1).createCopy();
            layerB = saveStates.getChild(2).createCopy();
            // Read abLayerState from vtsParams.state (child 0)
            ValueTree tempState = saveStates.getChild(0);
            if (tempState.hasProperty("ABLayer"))
            {
                Value val = tempState.getPropertyAsValue("ABLayer", nullptr);
                if (val.getValue().toString() != "")
                {
                    abLayerState = static_cast<int>(val.getValue()); // Restore abLayerState
                    // Validate abLayerState
                    if (abLayerState != COMPARE_LAYER_A && abLayerState != COMPARE_LAYER_B)
                        abLayerState = COMPARE_LAYER_A; // Fallback to default
                }
            }
            // Restore the correct layer based on abLayerState
            if (abLayerState == COMPARE_LAYER_B)
                vtsParams.replaceState(layerB);
            else
                vtsParams.replaceState(layerA);
        }
        else
        {
            jassertfalse; // Log error: incomplete saveStates
            vtsParams.replaceState(saveStates.getChild(0));
            layerA = vtsParams.copyState();
            layerB = vtsParams.copyState();
            abLayerState = COMPARE_LAYER_A; // Ensure consistent default
        }
    }
    else if (xmlState->hasTagName(vtsParams.state.getType()))
    {
        vtsParams.state = ValueTree::fromXml(*xmlState);
        layerA = vtsParams.copyState();
        layerB = vtsParams.copyState();
        // Read abLayerState from vtsParams.state
        if (vtsParams.state.hasProperty("ABLayer"))
        {
            Value val = vtsParams.state.getPropertyAsValue("ABLayer", nullptr);
            if (val.getValue().toString() != "")
            {
                abLayerState = static_cast<int>(val.getValue());
                // Validate abLayerState
                if (abLayerState != COMPARE_LAYER_A && abLayerState != COMPARE_LAYER_B)
                    abLayerState = COMPARE_LAYER_A; // Fallback to default
            }
        }
    }

    // Update doEq and oldProxDistance from vtsParams.state
    if (vtsParams.state.hasProperty("ffDfEq"))
    {
        Value val = vtsParams.state.getPropertyAsValue("ffDfEq", nullptr);
        if (val.getValue().toString() != "")
        {
            doEq = static_cast<int>(val.getValue());
        }
    }
    if (vtsParams.state.hasProperty("oldProxDistance"))
    {
        Value val = vtsParams.state.getPropertyAsValue("oldProxDistance", nullptr);
        if (val.getValue().toString() != "")
        {
            oldProxDistance = static_cast<float>(val.getValue());
        }
    }

    // Update layerB properties
    if (layerB.hasProperty("ffDfEq"))
    {
        Value val = layerB.getPropertyAsValue("ffDfEq", nullptr);
        if (val.getValue().toString() != "")
        {
            doEqB = static_cast<int>(val.getValue());
        }
    }
    if (layerB.hasProperty("oldProxDistance"))
    {
        Value val = layerB.getPropertyAsValue("oldProxDistance", nullptr);
        if (val.getValue().toString() != "")
        {
            oldProxDistanceB = static_cast<float>(val.getValue());
        }
    }
    if (layerB.hasProperty("oldNrBands"))
    {
        Value val = layerB.getPropertyAsValue("oldNrBands", nullptr);
        if (val.getValue().toString() != "")
        {
            oldNrBandsB = static_cast<float>(val.getValue());
        }
    }

    // Update layerA properties
    if (layerA.hasProperty("ffDfEq"))
    {
        Value val = layerA.getPropertyAsValue("ffDfEq", nullptr);
        if (val.getValue().toString() != "")
        {
            doEqA = static_cast<int>(val.getValue());
        }
    }
    if (layerA.hasProperty("oldProxDistance"))
    {
        Value val = layerA.getPropertyAsValue("oldProxDistance", nullptr);
        if (val.getValue().toString() != "")
        {
            oldProxDistanceA = static_cast<float>(val.getValue());
        }
    }
    if (layerA.hasProperty("oldNrBands"))
    {
        Value val = layerA.getPropertyAsValue("oldNrBands", nullptr);
        if (val.getValue().toString() != "")
        {
            oldNrBandsA = static_cast<float>(val.getValue());
        }
    }

    // Update nProcessorBands and ensure layer-specific oldNrBands is set
    if (nProcessorBandsPtr)
    {
        nProcessorBands = static_cast<unsigned int>(static_cast<int>(nProcessorBandsPtr->load()) + 1);
        if (abLayerState == COMPARE_LAYER_A)
            oldNrBandsA = nProcessorBandsPtr->load();
        else
            oldNrBandsB = nProcessorBandsPtr->load();
        oldNrBands = nProcessorBandsPtr->load();
    }

    // Initialize buffers for DAWs calling setStateInformation before prepareToPlay
    filterBankBuffer.setSize(N_CH_IN * MAX_NUM_EQS, currentBlockSize);
    filterBankBuffer.clear();
    firFilterBuffer.setSize(MAX_NUM_EQS, firLen);
    firFilterBuffer.clear();
    omniEightBuffer.setSize(MAX_NUM_INPUTS, currentBlockSize);
    omniEightBuffer.clear();

    recomputeAllFilterCoefficients = true;
    recomputeFilterCoefficientsIfNeeded();
    activeBandsChanged = true;
    zeroLatencyModeChanged = true;
    ffDfEqChanged = true;
    repaintDEQ = true;
}
#else

void PolarDesignerAudioProcessor::releaseResources() {
    resetTrackingState();
    for (auto &conv : convolvers) {
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


void PolarDesignerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    TRACE_EVENT("dsp", "setStateInformation");

    initializeBuffers();  // !J!

    if (currentSampleRate == 0.0 || currentBlockSize == 0) {
        LOG_WARN("Plugin not prepared, calling prepareToPlay with defaults");
        prepareToPlay(FILTER_BANK_NATIVE_SAMPLE_RATE, PD_DEFAULT_BLOCK_SIZE);
    }

    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState == nullptr) {
        LOG_ERROR("Invalid state data");
        initializeDefaultState();  // !J!
        return; // Match non-PROTOOLS behavior: silently fail
    }

    // Ensure buffers are initialized
    if (currentBlockSize == 0 || currentSampleRate == 0.0f)
    {
        LOG_ERROR("Uninitialized block size or sample rate, using defaults");
        initializeBuffers();
    }

    resetTrackingState(); // Clear tracking data

    // Load saveStates and restore abLayerState
    if (xmlState->hasTagName(saveStates.getType())) {
        saveStates = ValueTree::fromXml(*xmlState);
        if (saveStates.getNumChildren() >= 3) {
            layerA = saveStates.getChild(1).createCopy();
            layerB = saveStates.getChild(2).createCopy();
            ValueTree tempState = saveStates.getChild(0);
            if (tempState.hasProperty("ABLayer")) {
                Value val = tempState.getPropertyAsValue("ABLayer", nullptr);
                if (val.getValue().toString() != "") {
                    abLayerState = static_cast<int>(val.getValue());
                    if (abLayerState != COMPARE_LAYER_A && abLayerState != COMPARE_LAYER_B)
                        abLayerState = COMPARE_LAYER_A; // Fallback
                }
            }
            // Restore the correct layer
            vtsParams.replaceState(abLayerState == COMPARE_LAYER_B ? layerB : layerA);
        } else {
            LOG_ERROR("Incomplete saveStates, falling back");
            vtsParams.replaceState(saveStates.getChild(0));
            layerA = vtsParams.copyState();
            layerB = vtsParams.copyState();
            abLayerState = COMPARE_LAYER_A; // Match non-PROTOOLS fallback
        }
    } else if (xmlState->hasTagName(vtsParams.state.getType())) {
        vtsParams.state = ValueTree::fromXml(*xmlState);
        layerA = vtsParams.copyState();
        layerB = vtsParams.copyState();
        if (vtsParams.state.hasProperty("ABLayer")) {
            Value val = vtsParams.state.getPropertyAsValue("ABLayer", nullptr);
            if (val.getValue().toString() != "") {
                abLayerState = static_cast<int>(val.getValue());
                if (abLayerState != COMPARE_LAYER_A && abLayerState != COMPARE_LAYER_B)
                    abLayerState = COMPARE_LAYER_A;
            }
        }
    } else {
        LOG_ERROR("Unknown state format");
        initializeDefaultState();  // !J!
        //        return; // Match non-PROTOOLS behavior: silently fail
    }

    // Restore non-parameter state
    initializeBuffers();  // !J!

    if (vtsParams.state.hasProperty("ffDfEq")) {
        Value val = vtsParams.state.getPropertyAsValue("ffDfEq", nullptr);
        if (val.getValue().toString() != "") {
            doEq = static_cast<int>(val.getValue());
        }
    }
    if (vtsParams.state.hasProperty("oldProxDistance")) {
        Value val = vtsParams.state.getPropertyAsValue("oldProxDistance", nullptr);
        if (val.getValue().toString() != "") {
            oldProxDistance = static_cast<float>(val.getValue());
        }
    }

    // Restore layerB properties
    if (layerB.hasProperty("ffDfEq")) {
        Value val = layerB.getPropertyAsValue("ffDfEq", nullptr);
        if (val.getValue().toString() != "") {
            doEqB = static_cast<int>(val.getValue());
        }
    }
    if (layerB.hasProperty("oldProxDistance")) {
        Value val = layerB.getPropertyAsValue("oldProxDistance", nullptr);
        if (val.getValue().toString() != "") {
            oldProxDistanceB = static_cast<float>(val.getValue());
        }
    }
    if (layerB.hasProperty("oldNrBands")) {
        Value val = layerB.getPropertyAsValue("oldNrBands", nullptr);
        if (val.getValue().toString() != "") {
            oldNrBandsB = static_cast<float>(val.getValue());
        }
    }

    // Restore layerA properties
    if (layerA.hasProperty("ffDfEq")) {
        Value val = layerA.getPropertyAsValue("ffDfEq", nullptr);
        if (val.getValue().toString() != "") {
            doEqA = static_cast<int>(val.getValue());
        }
    }
    if (layerA.hasProperty("oldProxDistance")) {
        Value val = layerA.getPropertyAsValue("oldProxDistance", nullptr);
        if (val.getValue().toString() != "") {
            oldProxDistanceA = static_cast<float>(val.getValue());
        }
    }
    if (layerA.hasProperty("oldNrBands")) {
        Value val = layerA.getPropertyAsValue("oldNrBands", nullptr);
        if (val.getValue().toString() != "") {
            oldNrBandsA = static_cast<float>(val.getValue());
        }
    }

    // Update nProcessorBands and oldNrBands
    if (nProcessorBandsPtr) {
        nProcessorBands = static_cast<unsigned int>(static_cast<int>(nProcessorBandsPtr->load()) + 1);
        if (abLayerState == COMPARE_LAYER_A)
            oldNrBandsA = nProcessorBandsPtr->load();
        else
            oldNrBandsB = nProcessorBandsPtr->load();
        oldNrBands = nProcessorBandsPtr->load();
    }

    // Initialize buffers (match non-PROTOOLS behavior)
    filterBankBuffer.setSize(N_CH_IN * MAX_NUM_EQS, currentBlockSize, false, false, true);
    filterBankBuffer.clear();
    firFilterBuffer.setSize(MAX_NUM_EQS, firLen, false, false, true);
    firFilterBuffer.clear();
    omniEightBuffer.setSize(MAX_NUM_INPUTS, currentBlockSize, false, false, true);
    omniEightBuffer.clear();

    // Set processing flags
    recomputeAllFilterCoefficients = true;
    recomputeFilterCoefficientsIfNeeded(); // Match non-PROTOOLS behavior
    activeBandsChanged = true;
    zeroLatencyModeChanged = true;
    ffDfEqChanged = true;
    repaintDEQ = true;
}
#endif

/* IMPORTANT: parameterChanged *will* be called by both the message thread AND the audio thread.
 * This varies by DAW, but in general means one should avoid doing anything expensive.
 * Instead:
 * * set atomic flags for the UI to pick up on a timer
 * * set atomic flags for processBlock to check at the start of each block
*/

// parameterChanged: Add thread-safety and consistent oldNrBands updates
void PolarDesignerAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{

#ifdef USE_EXTRA_DEBUG_DUMPS
    LOG_DEBUG("Parameter changed: " + parameterID + " new Value: " + std::to_string(newValue) + " syncChannel: " + std::to_string(syncChannelPtr->load()));
#endif

    TRACE_EVENT("dsp", "parameterChanged", "paramID:", perfetto::DynamicString{parameterID.toStdString()});

    if (parameterID.startsWith("trimSlider"))
    {
        // TODO: update trimSlider position according to automation
    }
    else if (parameterID == "nrBands")
    {
        if (nProcessorBandsPtr)
        {
            nProcessorBands = static_cast<unsigned int>(static_cast<int>(nProcessorBandsPtr->load()) + 1);
            if (abLayerState == COMPARE_LAYER_A)
                oldNrBandsA = nProcessorBandsPtr->load();
            else
                oldNrBandsB = nProcessorBandsPtr->load();
            oldNrBands = nProcessorBandsPtr->load();
            //            resetXoverFreqs();    // !J! deferred
            recomputeAllFilterCoefficients = true;
            activeBandsChanged = true;
            //            resetTrackingState(); // Reset tracking data due to band change // !J! deferred
        }
    }
    else if (parameterID.startsWith("xOverF") && !loadingFile)
    {
        unsigned int idx = static_cast<unsigned int>(parameterID.getTrailingIntValue() - 1);
        recomputeFilterCoefficients[idx] = true;
        repaintDEQ = true;
    }
    else if (parameterID.startsWith("solo"))
    {
        soloActive = false;
        for (unsigned int i = 0; i < nProcessorBands; ++i)
        {
            if ((std::round(soloBandPtr[i]->load()) >= 0.5))
                soloActive = true;
        }
    }
    else if (parameterID.startsWith("alpha"))
    {
        repaintDEQ = true;
    }
    else if (parameterID == "proximity")
    {
        setProxCompCoefficients(proxDistancePtr->load());
    }
    else if (parameterID == "zeroLatencyMode")
    {
        TRACE_EVENT("dsp", "zeroLatencyMode");

        updateLatency();

        if (newValue < 0.5f)
        {
            // Zero Latency Mode turned off
            if (abLayerState == COMPARE_LAYER_B)
            {
                vtsParams.getParameter("proximity")->setValueNotifyingHost(vtsParams.getParameter("proximity")->convertTo0to1(oldProxDistanceB));
                vtsParams.getParameter("nrBands")->setValueNotifyingHost(vtsParams.getParameter("nrBands")->convertTo0to1(oldNrBandsB));
            }
            else
            {
                vtsParams.getParameter("proximity")->setValueNotifyingHost(vtsParams.getParameter("proximity")->convertTo0to1(oldProxDistanceA));
                vtsParams.getParameter("nrBands")->setValueNotifyingHost(vtsParams.getParameter("nrBands")->convertTo0to1(oldNrBandsA));
            }
            recomputeAllFilterCoefficients = true;
            zeroLatencyModeChanged = true;
        }
        else
        {
            // Zero Latency Mode turned on
            if (!abLayerChanged.get())
            {
                if (abLayerState == COMPARE_LAYER_B)
                {
                    oldProxDistanceB = proxDistancePtr->load();
                    oldNrBandsB = nProcessorBandsPtr->load();
                }
                else
                {
                    oldProxDistanceA = proxDistancePtr->load();
                    oldNrBandsA = nProcessorBandsPtr->load();
                }
            }

            // Set zero-latency parameters
            vtsParams.getParameter("nrBands")->setValueNotifyingHost(vtsParams.getParameter("nrBands")->convertTo0to1(1));
            vtsParams.getParameter("proximity")->setValueNotifyingHost(vtsParams.getParameter("proximity")->convertTo0to1(0));

            zeroLatencyModeChanged = true;
        }

        if (zeroLatencyModeChanged.get() && std::round(zeroLatencyModePtr->load()) > 0.5f)
        {
            vtsParams.state.setProperty("oldZeroLatencyMode", var(zeroLatencyModePtr->load()), nullptr);
            zeroLatencyModeChanged = true;
        }
    }
    else if (parameterID == "syncChannel")
    {
        int ch = (int)syncChannelPtr->load() - 1;

        if (ch > 0)
        {
            ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference (ch);
#ifdef USE_EXTRA_DEBUG_DUMPS
            LOG_DEBUG("syncChannel params updated, ch: " + std::to_string(ch) + " paramsValid: " + std::to_string(paramsToSync.paramsValid));
#endif

            if (!paramsToSync.paramsValid) // Initialize all params
            {

                for (int i = 0; i < MAX_NUM_EQS; ++i)
                {
                    //                paramsToSync.solo[i] = soloBandPtr[i] && !exactlyEqual(std::round(soloBandPtr[i]->load()), 0.0f);
                    //                paramsToSync.mute[i] = muteBandPtr[i] && !exactlyEqual(std::round(muteBandPtr[i]->load()), 0.0f);

                    paramsToSync.solo[i] = soloBandPtr[i] && std::round (soloBandPtr[i]->load()) > 0.5f;
                    paramsToSync.mute[i] = muteBandPtr[i] && std::round (muteBandPtr[i]->load()) > 0.5f;

                    paramsToSync.dirFactors[i] = dirFactorsPtr[i] ? dirFactorsPtr[i]->load() : 0.0f;
                    paramsToSync.gains[i] = bandGainsPtr[i] ? bandGainsPtr[i]->load() : 0.0f;

                    if (i < 4)
                    {
                        paramsToSync.xOverFreqs[i] = xOverFreqsPtr[i] ? xOverFreqsPtr[i]->load() : 0.0f;
                    }
                }

                paramsToSync.nrActiveBands = nProcessorBandsPtr ? static_cast<int> (nProcessorBandsPtr->load()) : 0;
                paramsToSync.proximity = proxDistancePtr ? proxDistancePtr->load() : 0.0f;
                //            paramsToSync.proximityOnOff = proxOnOffPtr && !exactlyEqual(std::round(proxOnOffPtr->load()), 0.0f);
                paramsToSync.proximityOnOff = proxOnOffPtr && std::round (proxOnOffPtr->load()) > 0.5f;

                //            paramsToSync.allowBackwardsPattern = allowBackwardsPatternPtr && !exactlyEqual(std::round(allowBackwardsPatternPtr->load()), 0.0f);
                //            paramsToSync.allowBackwardsPattern = allowBackwardsPatternPtr && std::round(allowBackwardsPatternPtr->load()) > 0.5f;

                paramsToSync.allowBackwardsPattern = true; // !J! ALWAYS TRUE

                paramsToSync.zeroLatencyMode = zeroLatencyModePtr && (std::round (zeroLatencyModePtr->load()) > 0.5f);
                paramsToSync.ffDfEq = doEq;
            }

            paramsToSync.paramsValid = true;
        }
        else {
#ifdef USE_EXTRA_DEBUG_DUMPS
            LOG_DEBUG("syncChannel params not updated, ch: " + std::to_string(ch));
#endif
        }
    }
#ifdef USE_EXTRA_DEBUG_DUMPS
    else {
        LOG_DEBUG("Some Other Parameter? :: " + parameterID);
    }
#endif

    // Update shared parameters if synced
    if ((syncChannelPtr->load() > 0) && !readingSharedParams)
    {
#ifdef USE_EXTRA_DEBUG_DUMPS
        LOG_DEBUG("SharedParams Reading:" + std::to_string(syncChannelPtr->load()));
#endif

        int ch = (int)syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference(ch);

        if (parameterID.startsWith("xOverF") && !loadingFile)
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.xOverFreqs[idx] = xOverFreqsPtr[idx]->load();
        }
        else if (parameterID.startsWith("solo"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.solo[idx] = (std::round(soloBandPtr[idx]->load()) > 0.5f);
        }
        else if (parameterID.startsWith("mute"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.mute[idx] = (std::round(muteBandPtr[idx]->load()) > 0.5f);
        }
        else if (parameterID.startsWith("alpha"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.dirFactors[idx] = dirFactorsPtr[idx]->load();
        }
        else if (parameterID == "nrBands")
        {
            paramsToSync.nrActiveBands = static_cast<int>(nProcessorBandsPtr->load());
        }
        else if (parameterID == "proximity")
        {
            paramsToSync.proximity = proxDistancePtr->load();
        }
        else if (parameterID == "proximityOnOff")
        {
            paramsToSync.proximityOnOff = (std::round(proxOnOffPtr->load()) > 0.5f);
        }
        else if (parameterID == "zeroLatencyMode")
        {
            paramsToSync.zeroLatencyMode = (std::round(zeroLatencyModePtr->load()) > 0.5f);
        }
        else if (parameterID.startsWith("gain"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.gains[idx] = bandGainsPtr[idx]->load();
        }
        /*
        else if (parameterID == "allowBackwardsPattern")
        {
            paramsToSync.allowBackwardsPattern = (std::round(allowBackwardsPatternPtr->load()) > 0.5f);
        }
        */
    }
#ifdef USE_EXTRA_DEBUG_DUMPS
    else {
        LOG_DEBUG("SharedParams Reading?" + std::to_string(syncChannelPtr->load()));
    }
#endif

//        recomputeFilterCoefficientsIfNeeded();
}

void PolarDesignerAudioProcessor::setEqState (int idx)
{
    doEq = idx;

    if ((syncChannelPtr->load() > 0) && !readingSharedParams)
    {
        int ch = (int) syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference (ch);
        paramsToSync.ffDfEq = doEq;
    }
}

void PolarDesignerAudioProcessor::resetXoverFreqs()
{
    TRACE_DSP();

    switch (nProcessorBands)
    {
        case 1:
            break;

        case 2:
            for (unsigned int i = 0; i < nProcessorBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))->setValueNotifyingHost (hzToZeroToOne (static_cast<int> (i), INIT_XOVER_FREQS_2B[i]));
            }
            break;

        case 3:
            for (unsigned int i = 0; i < nProcessorBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))->setValueNotifyingHost (hzToZeroToOne (static_cast<int> (i), INIT_XOVER_FREQS_3B[i]));
            }
            break;

        case 4:
            for (unsigned int i = 0; i < nProcessorBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))->setValueNotifyingHost (hzToZeroToOne ((int) i, INIT_XOVER_FREQS_4B[i]));
            }
            break;

        case 5:
            for (unsigned int i = 0; i < (nProcessorBands - 1); ++i)
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))->setValueNotifyingHost (hzToZeroToOne (static_cast<int> (i), INIT_XOVER_FREQS_5B[i]));
            }
            break;

        default:
            jassert (false);
            break;
    }
}

void PolarDesignerAudioProcessor::recomputeFilterCoefficientsIfNeeded()
{

    TRACE_DSP();

    if (recomputeAllFilterCoefficients.get())
    {
        computeAllFilterCoefficients();
        recomputeAllFilterCoefficients = false;
        resetXoverFreqs();
//        resetTrackingState(); // Reset tracking data due to band change

        // we don't need to go through the individual bands if we're doing it in bulk
        return;
    }

    for (unsigned int i = 0; i < 4; ++i)
    {
        if (recomputeFilterCoefficients[static_cast<unsigned long> (i)].get())
        {
            TRACE_DSP("coEfficient:", i);

            computeFilterCoefficients (i);
            initConvolver (static_cast<int> (i));
            recomputeFilterCoefficients[i] = false;
        }
    }
}



void PolarDesignerAudioProcessor::computeFilterCoefficients (unsigned int crossoverNr)
{
    TRACE_DSP();

    // only one band: no filtering
    if (nProcessorBands == 1)
        return;

    // lowest band is simple lowpass
    if (crossoverNr == 0)
    {
        dsp::FilterDesign<float>::FIRCoefficientsPtr lowpass = dsp::FilterDesign<float>::designFIRLowpassWindowMethod (
            hzFromZeroToOne (0, xOverFreqsPtr[0]->load()), currentSampleRate, (static_cast<size_t> (firLen - 1)), dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lpCoeffs = lowpass->getRawCoefficients();
        firFilterBuffer.copyFrom (0, 0, lpCoeffs, firLen);
    }

    // all the other bands are bandpass filters
    for (unsigned int i = (unsigned int) std::max (1, (int) crossoverNr); i < std::min ((crossoverNr + 2), (nProcessorBands - 1)); ++i)
    {
        float halfBandwidth =
            (hzFromZeroToOne (static_cast<int> (i), xOverFreqsPtr[i]->load()) - hzFromZeroToOne (static_cast<int> (i - 1), xOverFreqsPtr[i - 1]->load())) / 2;
        dsp::FilterDesign<float>::FIRCoefficientsPtr lp2bp = dsp::FilterDesign<float>::designFIRLowpassWindowMethod (
            halfBandwidth, currentSampleRate, static_cast<size_t> (firLen - 1), dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lp2bpCoeffs = lp2bp->getRawCoefficients();
        auto* filterBufferPointer = firFilterBuffer.getWritePointer (static_cast<int> (i));
        for (int j = 0; j < firLen; j++) // bandpass transform
        {
            float fCenter = halfBandwidth + hzFromZeroToOne (static_cast<int> (i - 1), xOverFreqsPtr[i - 1]->load());
            // write bandpass transformed fir coeffs to buffer
            *(filterBufferPointer + j) = 2 * *(lp2bpCoeffs + j) * std::cosf (static_cast<float> (MathConstants<float>::twoPi * fCenter / currentSampleRate * (j - (firLen - 1) / 2)));
        }
    }

    if (crossoverNr == nProcessorBands - 2)
    {
        // highest band is highpass (via frequency transform)
        float hpBandwidth = static_cast<float> (currentSampleRate / 2 - hzFromZeroToOne (static_cast<int> (nProcessorBands - 2), xOverFreqsPtr[nProcessorBands - 2]->load()));
        auto* filterBufferPointer = firFilterBuffer.getWritePointer (static_cast<int> (nProcessorBands - 1));
        dsp::FilterDesign<float>::FIRCoefficientsPtr lp2hp = dsp::FilterDesign<float>::designFIRLowpassWindowMethod (
            hpBandwidth, currentSampleRate, static_cast<size_t> (firLen - 1), dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lp2hpCoeffs = lp2hp->getRawCoefficients();
        for (int i = 0; i < firLen; ++i) // highpass transform
        {
            *(filterBufferPointer + i) =
                *(lp2hpCoeffs + i) * std::cosf (MathConstants<float>::pi * (i - (firLen - 1) / 2));
        }
    }
}


#ifdef USE_NEW_INITCONVOLVERS
void PolarDesignerAudioProcessor::initAllConvolvers() {
    TRACE_DSP();
    convolversReady = false;

    if (nProcessorBands == 0 || currentBlockSize == 0) return;

    tempBuffer.setSize(1, firLen);

    for (unsigned int i = 0; i < nProcessorBands; ++i) {

        tempBuffer.clear();
        tempBuffer.copyFrom(0, 0, firFilterBuffer, static_cast<int>(i), 0, firLen);

        // Create a copy for moving
        AudioBuffer<float> convSingleBuff(1, firLen);
        convSingleBuff.copyFrom(0, 0, tempBuffer, 0, 0, firLen);
        convolvers[2 * i].loadImpulseResponse(std::move(convSingleBuff), currentSampleRate,  dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

        // Reuse tempBuffer for the second convolver
        tempBuffer.clear();
        tempBuffer.copyFrom(0, 0, firFilterBuffer, static_cast<int>(i), 0, firLen);

        AudioBuffer<float> convSingleBuffEight(1, firLen);
        convSingleBuffEight.copyFrom(0, 0, tempBuffer, 0, 0, firLen);
        convolvers[2 * i + 1].loadImpulseResponse(std::move(convSingleBuffEight), currentSampleRate,  dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    }
    convolversReady = true;
}

void PolarDesignerAudioProcessor::initConvolver(int convNr) {
    TRACE_DSP();
    convolversReady = false;

    if (currentBlockSize == 0) return;

    tempBuffer.setSize(1, firLen);

    for (int i = convNr; i < convNr + 2 && i < static_cast<int>(nProcessorBands); ++i) {
        tempBuffer.clear();
        tempBuffer.copyFrom(0, 0, firFilterBuffer, i, 0, firLen);

        AudioBuffer<float> convSingleBuff(1, firLen);
        convSingleBuff.copyFrom(0, 0, tempBuffer, 0, 0, firLen);
        convolvers[2 * i].loadImpulseResponse(std::move(convSingleBuff), currentSampleRate,  dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

        tempBuffer.clear();
        tempBuffer.copyFrom(0, 0, firFilterBuffer, i, 0, firLen);

        AudioBuffer<float> convSingleBuffEight(1, firLen);
        convSingleBuffEight.copyFrom(0, 0, tempBuffer, 0, 0, firLen);
        convolvers[2 * i + 1].loadImpulseResponse(std::move(convSingleBuffEight), currentSampleRate,  dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    }
    convolversReady = true;
}

void PolarDesignerAudioProcessor::computeAllFilterCoefficients() {
    TRACE_DSP();
    for (unsigned int i = 0; i < 4; ++i) {
        computeFilterCoefficients(i);
    }
    loadFilterBankImpulseResponses();
}

#else

void PolarDesignerAudioProcessor::initAllConvolvers()
{

    TRACE_DSP();

    if (nProcessorBands == 0)
        return; // Some DAW's initialize the plugin on project-load with nProcessorBands == 0
    if (currentBlockSize == 0)
        return; // Cubase calls initAllConvolvers() (through setStateInformation) early, before we are ready ..

    // build filters and fill firFilterBuffer
    dsp::AudioBlock<float> convBlk (firFilterBuffer);
    dsp::ProcessSpec convSpec { currentSampleRate, static_cast<uint32> (currentBlockSize), 1 };

    convolversReady = false;

    for (unsigned int i = 0; i < nProcessorBands; ++i) // prepare nProcessorBands mono convolvers
    {
        AudioBuffer<float> convSingleBuffOmni (1, firLen);
        convSingleBuffOmni.copyFrom (0, 0, firFilterBuffer, static_cast<int> (i), 0, firLen);

        AudioBuffer<float> convSingleBuffEight (1, firLen);
        convSingleBuffEight.copyFrom (0, 0, firFilterBuffer, static_cast<int> (i), 0, firLen);

        // omni convolver
        convolvers[2 * i].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i].loadImpulseResponse (std::move (convSingleBuffOmni), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);

        // eight convolver
        convolvers[2 * i + 1].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i + 1].loadImpulseResponse (std::move (convSingleBuffEight), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
    }

    convolversReady = true;
}


void PolarDesignerAudioProcessor::initConvolver (int convNr)
{
    convolversReady = false;

    // build filters and fill firFilterBuffer
    dsp::AudioBlock<float> convBlk (firFilterBuffer);
    dsp::ProcessSpec convSpec { currentSampleRate, static_cast<uint32> (currentBlockSize), 1 };

    // update two convolvers: if one crossover frequency changes, two neighbouring bands need new filters
    for (int i = convNr; i < convNr + 2; ++i)
    {
        AudioBuffer<float> convSingleBuffOmni (1, firLen);
        convSingleBuffOmni.copyFrom (0, 0, firFilterBuffer, i, 0, firLen);

        AudioBuffer<float> convSingleBuffEight (1, firLen);
        convSingleBuffEight.copyFrom (0, 0, firFilterBuffer, i, 0, firLen);

        // omni convolver
        convolvers[2 * i].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i].loadImpulseResponse (std::move (convSingleBuffOmni), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);

        // eight convolver
        convolvers[2 * i + 1].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i + 1].loadImpulseResponse (std::move (convSingleBuffEight), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
    }
    convolversReady = true;
}

// compute filter bank filter coeffs and store in firFilterBuffer
void PolarDesignerAudioProcessor::computeAllFilterCoefficients()
{
    TRACE_DSP();

    for (unsigned int i = 0; i < 4; ++i)
    {
        computeFilterCoefficients (i);
    }
    initAllConvolvers();
}
#endif

void PolarDesignerAudioProcessor::loadFilterBankImpulseResponses() {
    initAllConvolvers(); // Reuse existing method for consistency
}

unsigned int PolarDesignerAudioProcessor::getNProcessorBands()
{
    return nProcessorBands;
}

void PolarDesignerAudioProcessor::createOmniAndEightSignals (AudioBuffer<float>& buffer)
{
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

void PolarDesignerAudioProcessor::createPolarPatterns (AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    buffer.clear();

    unsigned int nActiveBands = nProcessorBands;
    if (std::round(zeroLatencyModePtr->load()) > 0.5f)
        nActiveBands = 1;

    for (unsigned int i = 0; i < nActiveBands; ++i)
    {
        if ((std::round(muteBandPtr[i]->load()) > 0.5 && std::round(soloBandPtr[i]->load()) < 0.5) || (soloActive && std::round(soloBandPtr[i]->load()) < 0.5))
            continue;

        // calculate patterns and add to output buffer
        const float* readPointerOmni = filterBankBuffer.getReadPointer (static_cast<int> (2 * i));
        const float* readPointerEight = filterBankBuffer.getReadPointer (static_cast<int> (2 * i + 1));

        float oldGain = Decibels::decibelsToGain (oldBandGains[i], -59.91f);
        float gain = Decibels::decibelsToGain (bandGainsPtr[i]->load(), -59.91f);

        // add with ramp to prevent crackling noises
        buffer.addFromWithRamp (0, 0, readPointerOmni, numSamples, (1 - std::abs (oldDirFactors[i])) * oldGain, (1 - std::abs (dirFactorsPtr[i]->load())) * gain);
        buffer.addFromWithRamp (0, 0, readPointerEight, numSamples, oldDirFactors[i] * oldGain, dirFactorsPtr[i]->load() * gain);

        oldDirFactors[i] = dirFactorsPtr[i]->load();
        oldBandGains[i] = bandGainsPtr[i]->load();
    }

    // delay needs to be running constantly to prevent clicks
    delayBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
    dsp::AudioBlock<float> delayBlock (delayBuffer);
    dsp::ProcessContextReplacing<float> delayContext (delayBlock);
    delay.process (delayContext);

    if ((nActiveBands == 1) && (std::round(zeroLatencyModePtr->load()) < 0.5f))
    {
        buffer.copyFrom (0, 0, delayBuffer, 0, 0, numSamples);
    }

    // copy to second output channel -> this generates loud glitches in pro tools if mono output configuration is used
    // -> check getMainBusNumOutputChannels() !J!

    // !J! TODO: Check this on ProTools
    int numOutputChannels = getMainBusNumOutputChannels();

    if (buffer.getNumChannels() >= 2 && numOutputChannels >= 2) {
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    } else if (numOutputChannels == 1) {
        // Ensure mono output is handled cleanly
        buffer.clear(1, 0, numSamples); // Clear unused channel
    } else {
        LOG_ERROR("Unexpected output channel configuration: " + String(numOutputChannels));
    }

}

void PolarDesignerAudioProcessor::setLastDir (File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue ("presetFolder", v);
}

Result PolarDesignerAudioProcessor::loadPreset (const File& presetFile)
{
    var parsedJson;
    if (!presetFile.exists())
        return Result::fail ("File does not exist!");

    String jsonString = presetFile.loadFileAsString();
    Result result = JSON::parse (jsonString, parsedJson);
    if (!result.wasOk())
        return Result::fail ("File could not be parsed: Please provide valid JSON!");

    for (auto& it : presetProperties)
    {
        if (!parsedJson.hasProperty (it))
            return Result::fail ("Corrupt preset file: No '" + it + "' property found.");
    }

    loadingFile = true;

    float x = parsedJson.getProperty ("nrActiveBands", parsedJson);
    vtsParams.getParameter ("nrBands")->setValueNotifyingHost (vtsParams.getParameter ("nrBands")->convertTo0to1 (x - 1));

    for (int i = 0; i < 4; ++i)
    {
        x = parsedJson.getProperty ("xOverF" + String (i + 1), parsedJson);
        vtsParams.getParameter ("xOverF" + String (i + 1))->setValueNotifyingHost (hzToZeroToOne (i, x));
    }

    NormalisableRange<float> dfRange = vtsParams.getParameter ("alpha1")->getNormalisableRange();

    for (int i = 0; i < MAX_NUM_EQS; ++i)
    {
        x = parsedJson.getProperty ("dirFactor" + String (i + 1), parsedJson);
        if (x < dfRange.start || x > dfRange.end)
            return Result::fail (
                "DirFactor" + String (i + 1) + " needs to be between " + String (dfRange.start) + " and " + String (dfRange.end) + ".");
        vtsParams.getParameter ("alpha" + String (i + 1))->setValueNotifyingHost (dfRange.convertTo0to1 (x));

        x = parsedJson.getProperty ("gain" + String (i + 1), parsedJson);
        vtsParams.getParameter ("gain" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameter ("gain" + String (i + 1))->convertTo0to1 (x));

        x = parsedJson.getProperty ("solo" + String (i + 1), parsedJson);
        vtsParams.getParameter ("solo" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameter ("solo" + String (i + 1))->convertTo0to1 (x));

        x = parsedJson.getProperty ("mute" + String (i + 1), parsedJson);
        vtsParams.getParameter ("mute" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameter ("mute" + String (i + 1))->convertTo0to1 (x));
    }

    doEq = parsedJson.getProperty ("ffDfEq", parsedJson);

    x = parsedJson.getProperty ("proximity", parsedJson);
    vtsParams.getParameter ("proximity")->setValueNotifyingHost (vtsParams.getParameter ("proximity")->convertTo0to1 (x));

    x = parsedJson.getProperty ("proximityOnOff", parsedJson);
    vtsParams.getParameter ("proximityOnOff")->setValueNotifyingHost (vtsParams.getParameter ("proximityOnOff")->convertTo0to1 (x));

    //    x = parsedJson.getProperty("trimPosition", parsedJson);
    //    vtsParams.getParameter("trimPosition")->setValueNotifyingHost(
    //            vtsParams.getParameter("trimPosition")->convertTo0to1(x));

    x = parsedJson.getProperty ("zeroLatencyMode", parsedJson);
    vtsParams.getParameter ("zeroLatencyMode")->setValueNotifyingHost (vtsParams.getParameter ("zeroLatencyMode")->convertTo0to1 (x));

    loadingFile = false;

    // set parameters
    nProcessorBands = static_cast<unsigned int> (nProcessorBandsPtr->load() + 1);

    activeBandsChanged = true;
    computeAllFilterCoefficients();
    repaintDEQ = true;

    return Result::ok();
}

Result PolarDesignerAudioProcessor::savePreset (File destination)
{
    auto jsonObj = std::make_unique<DynamicObject>();

    jsonObj->setProperty ("Description",
        var ("This preset file was created with the Austrian Audio PolarDesigner plugin v" + String (JucePlugin_VersionString) + ", for more information see www.austrian.audio ."));
    jsonObj->setProperty ("nrActiveBands", (int) nProcessorBands);
    jsonObj->setProperty ("zeroLatencyMode", zeroLatencyModePtr->load());
    jsonObj->setProperty ("trimPosition", trimPositionPtr->load());
    jsonObj->setProperty ("xOverF1", static_cast<int> (hzFromZeroToOne (0, xOverFreqsPtr[0]->load())));
    jsonObj->setProperty ("xOverF2", static_cast<int> (hzFromZeroToOne (1, xOverFreqsPtr[1]->load())));
    jsonObj->setProperty ("xOverF3", static_cast<int> (hzFromZeroToOne (2, xOverFreqsPtr[2]->load())));
    jsonObj->setProperty ("xOverF4", static_cast<int> (hzFromZeroToOne (3, xOverFreqsPtr[3]->load())));
    jsonObj->setProperty ("dirFactor1", dirFactorsPtr[0]->load());
    jsonObj->setProperty ("dirFactor2", dirFactorsPtr[1]->load());
    jsonObj->setProperty ("dirFactor3", dirFactorsPtr[2]->load());
    jsonObj->setProperty ("dirFactor4", dirFactorsPtr[3]->load());
    jsonObj->setProperty ("dirFactor5", dirFactorsPtr[4]->load());
    jsonObj->setProperty ("gain1", bandGainsPtr[0]->load());
    jsonObj->setProperty ("gain2", bandGainsPtr[1]->load());
    jsonObj->setProperty ("gain3", bandGainsPtr[2]->load());
    jsonObj->setProperty ("gain4", bandGainsPtr[3]->load());
    jsonObj->setProperty ("gain5", bandGainsPtr[4]->load());
    jsonObj->setProperty ("solo1", soloBandPtr[0]->load());
    jsonObj->setProperty ("solo2", soloBandPtr[1]->load());
    jsonObj->setProperty ("solo3", soloBandPtr[2]->load());
    jsonObj->setProperty ("solo4", soloBandPtr[3]->load());
    jsonObj->setProperty ("solo5", soloBandPtr[4]->load());
    jsonObj->setProperty ("mute1", muteBandPtr[0]->load());
    jsonObj->setProperty ("mute2", muteBandPtr[1]->load());
    jsonObj->setProperty ("mute3", muteBandPtr[2]->load());
    jsonObj->setProperty ("mute4", muteBandPtr[3]->load());
    jsonObj->setProperty ("mute5", muteBandPtr[4]->load());
    jsonObj->setProperty ("ffDfEq", doEq);
    jsonObj->setProperty ("proximity", proxDistancePtr->load());
    jsonObj->setProperty ("proximityOnOff", proxOnOffPtr->load());

    String jsonString = JSON::toString (var (jsonObj.get()), false, 2);
    if (destination.replaceWithText (jsonString))
        return Result::ok();
    else
        return Result::fail ("Could not write preset file. Check file access permissions.");
}

float PolarDesignerAudioProcessor::hzToZeroToOne (int idx, float hz)
{
    switch (nProcessorBands)
    {
        case 1:
            return 0;
            break;

        case 2:
            return (hz - XOVER_RANGE_START_2B[idx]) / (XOVER_RANGE_END_2B[idx] - XOVER_RANGE_START_2B[idx]);
            break;

        case 3:
            return (hz - XOVER_RANGE_START_3B[idx]) / (XOVER_RANGE_END_3B[idx] - XOVER_RANGE_START_3B[idx]);
            break;

        case 4:
            return (hz - XOVER_RANGE_START_4B[idx]) / (XOVER_RANGE_END_4B[idx] - XOVER_RANGE_START_4B[idx]);
            break;

        case 5:
            return (hz - XOVER_RANGE_START_5B[idx]) / (XOVER_RANGE_END_5B[idx] - XOVER_RANGE_START_5B[idx]);
            break;

        default:
            jassert (false);
            break;
    }
    return 0;
}

float PolarDesignerAudioProcessor::hzFromZeroToOne (int idx, float val)
{
    switch (nProcessorBands)
    {
        case 1:
            return 0;
            break;

        case 2:
            return XOVER_RANGE_START_2B[idx] + val * (XOVER_RANGE_END_2B[idx] - XOVER_RANGE_START_2B[idx]);
            break;

        case 3:
            return XOVER_RANGE_START_3B[idx] + val * (XOVER_RANGE_END_3B[idx] - XOVER_RANGE_START_3B[idx]);
            break;

        case 4:
            return XOVER_RANGE_START_4B[idx] + val * (XOVER_RANGE_END_4B[idx] - XOVER_RANGE_START_4B[idx]);
            break;

        case 5:
            return XOVER_RANGE_START_5B[idx] + val * (XOVER_RANGE_END_5B[idx] - XOVER_RANGE_START_5B[idx]);
            break;

        default:
            jassert (false);
            break;
    }
    return 0;
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
#ifdef USE_EXTRA_DEBUG_DUMPS
    LOG_DEBUG("START TRACKING...");
#endif

    signalRecorded = false;
    disturberRecorded = false;
    trackingDisturber = trackDisturber;
    for (int i = 0; i < MAX_NUM_EQS; ++i)
    {
        omniSqSumSig[i] = 0.0f;
        eightSqSumSig[i] = 0.0f;
        omniEightSumSig[i] = 0.0f;
        omniSqSumDist[i] = 0.0f;
        eightSqSumDist[i] = 0.0f;
        omniEightSumDist[i] = 0.0f;
    }
    nrBlocksRecorded = 0;
    trackingActive = true;
}

void PolarDesignerAudioProcessor::resetTrackingState()
{
#ifdef USE_EXTRA_DEBUG_DUMPS
    LOG_DEBUG("RESET TRACKING...");
#endif

    trackingActive = false;
    trackingDisturber = false;
    signalRecorded = false;
    disturberRecorded = false;
    nrBlocksRecorded = 0;
    for (int i = 0; i < MAX_NUM_EQS; ++i)
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
#ifdef USE_EXTRA_DEBUG_DUMPS
    LOG_DEBUG("STOP TRACKING...");
#endif

    trackingActive = false;
    if (nrBlocksRecorded == 0)
        return; // Skip if no blocks recorded

    if (applyOptimalPattern == 1)
    {
#ifdef USE_EXTRA_DEBUG_DUMPS
        LOG_DEBUG("TRACKING: State <1>");
#endif
        if (trackingDisturber)
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < MAX_NUM_EQS; ++i)
                {
                    omniSqSumDist[i] = omniSqSumDist[i] / nrBlocksRecorded;
                    eightSqSumDist[i] = eightSqSumDist[i] / nrBlocksRecorded;
                    omniEightSumDist[i] = omniEightSumDist[i] / nrBlocksRecorded;
                }
            }
            setMinimumDisturbancePattern();
        }
        else
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < MAX_NUM_EQS; ++i)
                {
                    omniSqSumSig[i] = omniSqSumSig[i] / nrBlocksRecorded;
                    eightSqSumSig[i] = eightSqSumSig[i] / nrBlocksRecorded;
                    omniEightSumSig[i] = omniEightSumSig[i] / nrBlocksRecorded;
                }
            }
            setMaximumSignalPattern();
        }
    }
    else if (applyOptimalPattern == 2) // max sig-to-dist
    {
#ifdef USE_EXTRA_DEBUG_DUMPS
        LOG_DEBUG("TRACKING: State <2>");
#endif
        if (trackingDisturber)
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < MAX_NUM_EQS; ++i)
                {
                    omniSqSumDist[i] = omniSqSumDist[i] / nrBlocksRecorded;
                    eightSqSumDist[i] = eightSqSumDist[i] / nrBlocksRecorded;
                    omniEightSumDist[i] = omniEightSumDist[i] / nrBlocksRecorded;
                }
            }
            disturberRecorded = true;
        }
        else
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < MAX_NUM_EQS; ++i)
                {
                    omniSqSumSig[i] = omniSqSumSig[i] / nrBlocksRecorded;
                    eightSqSumSig[i] = eightSqSumSig[i] / nrBlocksRecorded;
                    omniEightSumSig[i] = omniEightSumSig[i] / nrBlocksRecorded;
                }
            }
            signalRecorded = true;
        }
        maximizeSigToDistRatio();
    }
}

void PolarDesignerAudioProcessor::trackSignalEnergy()
{
#ifdef USE_EXTRA_DEBUG_DUMPS
    LOG_DEBUG("TRACKING SIGNAL ENERGY...");
#endif

    int numSamples = filterBankBuffer.getNumSamples();
    if (numSamples == 0)
        return; // avoid division by zero

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        const float* readPointerOmni = filterBankBuffer.getReadPointer (static_cast<int> (2 * i));
        const float* readPointerEight = filterBankBuffer.getReadPointer (static_cast<int> (2 * i + 1));
        if (trackingDisturber)
        {
            for (int j = 0; j < numSamples; ++j)
            {
                float omniSample = readPointerOmni[j];
                omniSqSumDist[i] += omniSample * omniSample / numSamples;
                float eightSample = readPointerEight[j];
                eightSqSumDist[i] += eightSample * eightSample / numSamples;
                omniEightSumDist[i] += omniSample * eightSample / numSamples;
            }
        }
        else
        {
            for (int j = 0; j < numSamples; ++j)
            {
                float omniSample = readPointerOmni[j];
                omniSqSumSig[i] += omniSample * omniSample / numSamples;
                float eightSample = readPointerEight[j];
                eightSqSumSig[i] += eightSample * eightSample / numSamples;
                omniEightSumSig[i] += omniSample * eightSample / numSamples;
            }
        }
    }
    ++nrBlocksRecorded;
}

void PolarDesignerAudioProcessor::setMinimumDisturbancePattern()
{
    float disturberPower = 0.0f;
    float minPowerAlpha = 0.0f;
    float alphaStart = 0.0f;

#ifdef USE_EXTRA_DEBUG_DUMPS
    LOG_DEBUG("MINUMUM DISTURBANCE PATTERN...");
#endif
    // !J! NOTE: allowBackwardsPattern is ALWAYS true
    //if (std::round(allowBackwardsPatternPtr->load()) == 1.0f)
    alphaStart = -0.5f;

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentPower =
                static_cast<float> (std::pow ((1 - std::abs (alpha)), 2) * omniSqSumDist[i] + std::pow (alpha, 2) * eightSqSumDist[i] + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumDist[i]);
            if (exactlyEqual (alpha, alphaStart) || (currentPower < disturberPower))
            {
                disturberPower = currentPower;
                minPowerAlpha = alpha;
            }
        }
        if (disturberPower != 0.0f) // do not apply changes, if playback is not active
        {
            vtsParams.getParameter ("alpha" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameter ("alpha1")->convertTo0to1 (minPowerAlpha));
            disturberRecorded = true;
        }
    }
}

void PolarDesignerAudioProcessor::setMaximumSignalPattern()
{
    float signalPower = 0.0f;
    float maxPowerAlpha = 0.0f;
    float alphaStart = 0.0f;

    // !J! NOTE: allowBackwardsPattern is ALWAYS true
    //if (std::round(allowBackwardsPatternPtr->load()) == 1.0f)
    alphaStart = -0.5f;

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentPower =
                static_cast<float> (std::pow ((1 - std::abs (alpha)), 2) * omniSqSumSig[i] + std::pow (alpha, 2) * eightSqSumSig[i] + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumSig[i]);
            if (exactlyEqual (alpha, alphaStart) || (currentPower > signalPower))
            {
                signalPower = currentPower;
                maxPowerAlpha = alpha;
            }
        }
        if (signalPower != 0.0f)
        {
            vtsParams.getParameter ("alpha" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameter ("alpha1")->convertTo0to1 (maxPowerAlpha));
            signalRecorded = true;
        }
    }
}

void PolarDesignerAudioProcessor::maximizeSigToDistRatio()
{
    if (!signalRecorded || !disturberRecorded)
        return; // Skip if both signal and disturber haven't been recorded

    bool validData = true;
    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        if (omniSqSumSig[i] == 0.0f && eightSqSumSig[i] == 0.0f && omniEightSumSig[i] == 0.0f &&
            omniSqSumDist[i] == 0.0f && eightSqSumDist[i] == 0.0f && omniEightSumDist[i] == 0.0f)
        {
            validData = false;
            break;
        }
    }
    if (!validData)
        return;

    float distToSigRatio = 0.0f;
    float maxDistToSigAlpha = 0.0f;
    float alphaStart = 0.0f;

    // !J! should ALWAYS be true
    if (std::round(allowBackwardsPatternPtr->load()) > 0.5f)
        alphaStart = -0.5f;

    for (unsigned int i = 0; i < nProcessorBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentSigPower =
                static_cast<float> (std::pow ((1 - std::abs (alpha)), 2) * omniSqSumSig[i] + std::pow (alpha, 2) * eightSqSumSig[i] + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumSig[i]);
            float currentDistPower =
                static_cast<float> (std::pow ((1 - std::abs (alpha)), 2) * omniSqSumDist[i] + std::pow (alpha, 2) * eightSqSumDist[i] + 2 * (1 - std::abs (alpha)) * alpha * omniEightSumDist[i]);
            float currentRatio;
            if (exactlyEqual (currentDistPower, 0.0f))
            {
                currentRatio = 0.0f;
            }
            else
                currentRatio = currentSigPower / currentDistPower;

            if (exactlyEqual (alpha, alphaStart) || (currentRatio > distToSigRatio))
            {
                distToSigRatio = currentRatio;
                maxDistToSigAlpha = alpha;
            }
        }
        if (distToSigRatio != 0.0f)
            vtsParams.getParameter ("alpha" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameter ("alpha1")->convertTo0to1 (maxDistToSigAlpha));
    }
}

void PolarDesignerAudioProcessor::setProxCompCoefficients (float distance)
{
    int c = 343;
    double fs = getSampleRate();

    //    float b0 = -c / (fs * 4 * distance) + 1;
    //    float b1 = -exp(-c / (fs * 2 * distance)) * (1 + c / (fs * 4 * distance));
    //    float a0 = 1;
    //    float a1 = -exp(-c / (fs * 2 * distance));

    // use logarithmic fader impact: equation is for fader between -1.0 .. 1.0
    // returns values between 1 .. 0.1
    float a = (0.05f - 1.0f) / (-log (1.1f) + log (0.1f));
    float b = 1.0f + a * log (0.1f);

//    float r = -a * log (std::abs (distance) + 0.1f) + b;
    float r = -a * log(std::max(std::abs(distance) + 0.1f, 0.0001f)) + b;

    float b0, b1, a0, a1;

    // normalized to r_ref = 1m
    if (distance <= 0) //bass cut
    {
        if (r < 0.01f)
            r = 0.01f;

        b0 = static_cast<float> (c * (r - 1.0f) / (fs * 2.0f * r) + 1.0f);
        b1 = static_cast<float> (-exp (-c / (fs * r)) * (1.0f - c * (r - 1.0f) / (fs * 2.0f * r)));
        a0 = 1.0f;
        a1 = static_cast<float> (-exp (-c / (fs * r)));
    }
    else // bass boost, careful: instable for r<0.05
    {
        if (r < 0.05f)
            r = 0.05f;

        b0 = static_cast<float> (static_cast<float> (c) * (1.0f - r) / (fs * 2.0f * r) + 1.0f);
        b1 = static_cast<float> (-exp (-c / fs) * (1.0f - c * (1.0f - r) / (fs * 2.0f * r)));
        a0 = 1.0f;
        a1 = static_cast<float> (-exp (-c / fs));
    }

    *proxCompIIR.coefficients = dsp::IIR::Coefficients<float> (b0, b1, a0, a1);
}

void PolarDesignerAudioProcessor::timerCallback()
{
    TRACE_DSP();
    if (recomputeAllFilterCoefficients.get() || std::any_of(recomputeFilterCoefficients.begin(), recomputeFilterCoefficients.end(), [](auto flag) { return flag.get(); })) {
        recomputeFilterCoefficientsIfNeeded();
    }

    if (zeroLatencyModeChanged.exchange(false)) {
        updateLatency();
    }


    if (syncChannelPtr->load() > 0)
    {

        readingSharedParams = true;

        int ch = (int) syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference (ch);

        if (!exactlyEqual (nProcessorBandsPtr->load(), static_cast<float> (paramsToSync.nrActiveBands)))
        {
            vtsParams.getParameter ("nrBands")->setValueNotifyingHost (
                vtsParams.getParameterRange ("nrBands").convertTo0to1 (paramsToSync.nrActiveBands * 1.0f));
        }

        for (int i = 0; i < MAX_NUM_EQS; ++i)
        {
            if (!exactlyEqual (dirFactorsPtr[i]->load(), paramsToSync.dirFactors[i]))
            {
                vtsParams.getParameter ("alpha" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameterRange ("alpha" + String (i + 1)).convertTo0to1 (paramsToSync.dirFactors[i]));
            }

            if (!exactlyEqual (soloBandPtr[i]->load(), paramsToSync.solo[i] ? 1.0f : 0.0f))
            {
                vtsParams.getParameter ("solo" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameterRange ("solo" + String (i + 1)).convertTo0to1 (paramsToSync.solo[i]));
            }

            if (!exactlyEqual (muteBandPtr[i]->load(), paramsToSync.mute[i] ? 1.0f : 0.0f))
            {
                vtsParams.getParameter ("mute" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameterRange ("mute" + String (i + 1)).convertTo0to1 (paramsToSync.mute[i]));
            }

            if (!exactlyEqual (bandGainsPtr[i]->load(), paramsToSync.gains[i]))
            {
                vtsParams.getParameter ("gain" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameterRange ("gain" + String (i + 1)).convertTo0to1 (paramsToSync.gains[i]));
            }

            if ((i < 4) && !exactlyEqual (xOverFreqsPtr[i]->load(), paramsToSync.xOverFreqs[i]))
            {
                vtsParams.getParameter ("xOverF" + String (i + 1))->setValueNotifyingHost (vtsParams.getParameterRange ("xOverF" + String (i + 1)).convertTo0to1 (paramsToSync.xOverFreqs[i]));
            }
        }

        if (!exactlyEqual (proxDistancePtr->load(), paramsToSync.proximity))
        {
            vtsParams.getParameter ("proximity")->setValueNotifyingHost (vtsParams.getParameterRange ("proximity").convertTo0to1 (paramsToSync.proximity));
        }

        /* !J! allowBackwardsPatternPtr should ALWAYS be true - it has been deprecated in the UI but may persist in saved settings/sessions:
  if (!exactlyEqual (allowBackwardsPatternPtr->load(), paramsToSync.allowBackwardsPattern ? 1.0f : 0.0f) &&
            std::round(allowBackwardsPatternPtr->load()) != (paramsToSync.allowBackwardsPattern ? 1.0f : 0.0f))
        {
            vtsParams.getParameter ("allowBackwardsPattern")->setValueNotifyingHost (vtsParams.getParameterRange ("allowBackwardsPattern").convertTo0to1 (paramsToSync.allowBackwardsPattern));
        }
*/

        if (!exactlyEqual (proxOnOffPtr->load(), paramsToSync.proximityOnOff ? 1.0f : 0.0f))
        {
            vtsParams.getParameter ("proximityOnOff")->setValueNotifyingHost (vtsParams.getParameterRange ("proximityOnOff").convertTo0to1 (paramsToSync.proximityOnOff));
        }

        if (paramsToSync.ffDfEq != doEq)
        {
            setEqState (paramsToSync.ffDfEq);
            ffDfEqChanged = true;
        }

        if ((std::round(zeroLatencyModePtr->load()) > 0.5f ? true : false) != paramsToSync.zeroLatencyMode)
        {
            vtsParams.getParameter ("zeroLatencyMode")->setValueNotifyingHost (vtsParams.getParameterRange ("zeroLatencyMode").convertTo0to1 (paramsToSync.zeroLatencyMode));

            paramsToSync.zeroLatencyMode = (std::round(zeroLatencyModePtr->load()) > 0.5f);

#ifdef USE_EXTRA_DEBUG_DUMPS
            LOG_DEBUG(String::formatted("PLUGINPROCESSOR %p: zeroLatencyModePtr update", this));
#endif
        }

        readingSharedParams = false;
    }

}

#ifdef USE_NEW_UPDATELATENCY
void PolarDesignerAudioProcessor::updateLatency() {
    TRACE_DSP();

    if (isBypassed || std::round(zeroLatencyModePtr->load()) > 0.5f) {
        setLatencySamples(0);
    } else {
        jassert(firLen % 2 == 1); // Ensure firLen is odd
        int latency = (firLen - 1) / 2;
        // Verify EQ convolution latency (padded to firLen, so same latency)
        setLatencySamples(latency);
    }
}
#else

void PolarDesignerAudioProcessor::updateLatency() {
    TRACE_DSP();
    int newLatency = (isBypassed || std::round(zeroLatencyModePtr->load()) > 0.5f) ? 0 : (firLen - 1) / 2;
    if (getLatencySamples() != newLatency) {
        setLatencySamples(newLatency);
        LOG_DEBUG("Latency updated to: " + String(newLatency));
    }
}
#endif

void PolarDesignerAudioProcessor::changeABLayerState(int state)
{

    // Validate abLayerState
    jassert(state == COMPARE_LAYER_A || state == COMPARE_LAYER_B);

    abLayerState = state;
    abLayerChanged = true;
    ffDfEqChanged = true;

    resetTrackingState(); // Clear tracking data

#ifdef USE_EXTRA_DEBUG_DUMPS
    juce::String treeAsXmlString = vtsParams.state.toXmlString();
    LOG_DEBUG(treeAsXmlString.toStdString());
#endif

    if (abLayerState == COMPARE_LAYER_B)
    {
        layerA = vtsParams.copyState();
        doEqA = doEq;

        if (!zeroLatencyModeActive() && proxDistancePtr && nProcessorBandsPtr)
        {
            if (proxDistancePtr && nProcessorBandsPtr)
            {
                oldProxDistanceA = proxDistancePtr->load();
                oldNrBandsA = nProcessorBandsPtr->load();
            }
        }
        readingSharedParams = true;

        vtsParams.state = layerB.createCopy();
        doEq = doEqB;

        if (zeroLatencyModeActive())
        {
            oldProxDistance = 0;
        }
        else
        {
            oldProxDistance = oldProxDistanceB;
            oldNrBands = oldNrBandsB;
        }
    }
    else
    {
        layerB = vtsParams.copyState();
        doEqB = doEq;

        if (!zeroLatencyModeActive())
        {
            if (proxDistancePtr && nProcessorBandsPtr)
            {
                oldProxDistanceB = proxDistancePtr->load();
                oldNrBandsB = nProcessorBandsPtr->load();
            }
        }
        readingSharedParams = true;

        vtsParams.state = layerA.createCopy();
        doEq = doEqA;

        if (zeroLatencyModeActive())
        {
            oldProxDistance = 0;
        }
        else
        {
            oldProxDistance = oldProxDistanceA;
            oldNrBands = oldNrBandsA;
        }
    }

    if (proxDistancePtr)
        vtsParams.getParameter("proximity")->setValueNotifyingHost(vtsParams.getParameter("proximity")->convertTo0to1(oldProxDistance));
    if (nProcessorBandsPtr)
        vtsParams.getParameter("nrBands")->setValueNotifyingHost(vtsParams.getParameter("nrBands")->convertTo0to1(oldNrBands));

    abLayerChanged = false;

#ifdef USE_EXTRA_DEBUG_DUMPS
    juce::String treeAsXmlString2 = vtsParams.state.toXmlString();
    LOG_DEBUG(treeAsXmlString2.toStdString());
#endif
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolarDesignerAudioProcessor();
}
