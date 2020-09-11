/*
 ==============================================================================
 PluginProcessor.cpp
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PolarDesignerAudioProcessor::PolarDesignerAudioProcessor() :
    AudioProcessor (BusesProperties()
           .withInput  ("Input",  AudioChannelSet::stereo(), true)
           .withOutput ("Output", AudioChannelSet::stereo(), true)
           ),
    nBands(5),
    vtsParams(*this, nullptr, "AAPolarDesigner",
           {
               std::make_unique<AudioParameterFloat> ("xOverF1", "Xover1", NormalisableRange<float>(0.0f, 1.0f, 0.0001f),
                                                      hzToZeroToOne(0, INIT_XOVER_FREQS_5B[0]), "",
                                                      AudioProcessorParameter::genericParameter,
                                                      [&](float value, int maximumStringLength) {return String(std::roundf(hzFromZeroToOne(0, value))) + " Hz";},
                                                      nullptr),
               std::make_unique<AudioParameterFloat> ("xOverF2", "Xover2", NormalisableRange<float>(0.0f, 1.0f, 0.0001f),
                                                      hzToZeroToOne(1, INIT_XOVER_FREQS_5B[1]), "",
                                                      AudioProcessorParameter::genericParameter,
                                                      [&](float value, int maximumStringLength) {return String(std::roundf(hzFromZeroToOne(1, value))) + " Hz";},
                                                      nullptr),
               std::make_unique<AudioParameterFloat> ("xOverF3", "Xover3", NormalisableRange<float>(0.0f, 1.0f, 0.0001f),
                                                      hzToZeroToOne(2, INIT_XOVER_FREQS_5B[2]), "",
                                                      AudioProcessorParameter::genericParameter,
                                                      [&](float value, int maximumStringLength) {return String(std::roundf(hzFromZeroToOne(2, value))) + " Hz";},
                                                      nullptr),
               std::make_unique<AudioParameterFloat> ("xOverF4", "Xover4", NormalisableRange<float>(0.0f, 1.0f, 0.0001f),
                                                      hzToZeroToOne(3, INIT_XOVER_FREQS_5B[3]), "",
                                                      AudioProcessorParameter::genericParameter,
                                                      [&](float value, int maximumStringLength) {return String(std::roundf(hzFromZeroToOne(3, value))) + " Hz";},
                                                      nullptr),
               std::make_unique<AudioParameterFloat> ("alpha1", "Polar1", NormalisableRange<float>(-0.5f, 1.0f, 0.01f),
                                                      0.0f, "", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
               std::make_unique<AudioParameterFloat> ("alpha2", "Polar2", NormalisableRange<float>(-0.5f, 1.0f, 0.01f),
                                                      0.0f, "", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
               std::make_unique<AudioParameterFloat> ("alpha3", "Polar3", NormalisableRange<float>(-0.5f, 1.0f, 0.01f),
                                                      0.0f, "", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
               std::make_unique<AudioParameterFloat> ("alpha4", "Polar4", NormalisableRange<float>(-0.5f, 1.0f, 0.01f),
                                                      0.0f, "", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
               std::make_unique<AudioParameterFloat> ("alpha5", "Polar5", NormalisableRange<float>(-0.5f, 1.0f, 0.01f),
                                                      0.0f, "", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
               std::make_unique<AudioParameterBool>  ("solo1", "Solo1", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("solo2", "Solo2", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("solo3", "Solo3", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("solo4", "Solo4", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("solo5", "Solo5", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("mute1", "Mute1", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("mute2", "Mute2", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("mute3", "Mute3", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("mute4", "Mute4", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterBool>  ("mute5", "Mute5", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterFloat> ("gain1", "Gain1", NormalisableRange<float>(-24.0f, 18.0f, 0.1f),
                                                      0.0f, "dB", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
               std::make_unique<AudioParameterFloat> ("gain2", "Gain2", NormalisableRange<float>(-24.0f, 18.0f, 0.1f),
                                                      0.0f, "dB", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
               std::make_unique<AudioParameterFloat> ("gain3", "Gain3", NormalisableRange<float>(-24.0f, 18.0f, 0.1f),
                                                      0.0f, "dB", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
               std::make_unique<AudioParameterFloat> ("gain4", "Gain4", NormalisableRange<float>(-24.0f, 18.0f, 0.1f),
                                                      0.0f, "dB", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
               std::make_unique<AudioParameterFloat> ("gain5", "Gain5", NormalisableRange<float>(-24.0f, 18.0f, 0.1f),
                                                      0.0f, "dB", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
               std::make_unique<AudioParameterInt>   ("nrBands", "Nr. of Bands", 0, 4, 4, "",
                                                      [](int value, int maximumStringLength) {return String(value + 1);}, nullptr),
               std::make_unique<AudioParameterBool>  ("allowBackwardsPattern", "Allow Reverse Patterns", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterFloat> ("proximity", "Proximity", NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
                                                     0.0f, "", AudioProcessorParameter::genericParameter,
                                                      [](float value, int maximumStringLength) { return std::abs(value) < 0.05f ? "off" : String(value, 2); }, nullptr),
               std::make_unique<AudioParameterBool>  ("zeroDelayMode", "Zero Latency", false, "",
                                                      [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
               std::make_unique<AudioParameterInt>   ("syncChannel", "Sync to Channel", 0, 4, 0, "",
                                                      [](int value, int maximumStringLength) {return value == 0 ? "none" : String(value);}, nullptr)
           }),
    firLen(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE),
    dfEqOmniBuffer(1, DF_EQ_LEN), dfEqEightBuffer(1, DF_EQ_LEN),
    ffEqOmniBuffer(1, FF_EQ_LEN), ffEqEightBuffer(1, FF_EQ_LEN), doEq(0), isBypassed(false),
    soloActive(false), loadingFile(false), readingSharedParams(false), trackingActive(false),
    trackingDisturber(false), disturberRecorded(false), signalRecorded(false), currentSampleRate(48000)
{
    
    vtsParams.addParameterListener("xOverF1", this);
    vtsParams.addParameterListener("xOverF2", this);
    vtsParams.addParameterListener("xOverF3", this);
    vtsParams.addParameterListener("xOverF4", this);
    xOverFreqs[0] = vtsParams.getRawParameterValue("xOverF1");
    xOverFreqs[1] = vtsParams.getRawParameterValue("xOverF2");
    xOverFreqs[2] = vtsParams.getRawParameterValue("xOverF3");
    xOverFreqs[3] = vtsParams.getRawParameterValue("xOverF4");
    for (int i = 0; i < 5; ++i)
    {
        vtsParams.addParameterListener("alpha"+String(i+1), this);
        dirFactors[i] = vtsParams.getRawParameterValue("alpha"+String(i+1));
        
        vtsParams.addParameterListener("solo"+String(i+1), this);
        soloBand[i] = vtsParams.getRawParameterValue("solo"+String(i+1));
        
        vtsParams.addParameterListener("mute"+String(i+1), this);
        muteBand[i] = vtsParams.getRawParameterValue("mute"+String(i+1));
        
        vtsParams.addParameterListener("gain"+String(i+1), this);
        bandGains[i] = vtsParams.getRawParameterValue("gain"+String(i+1));
    }
    vtsParams.addParameterListener("nrBands", this);
    nBandsPtr = vtsParams.getRawParameterValue("nrBands");
    vtsParams.addParameterListener("allowBackwardsPattern", this);
    allowBackwardsPattern = vtsParams.getRawParameterValue("allowBackwardsPattern");
    vtsParams.addParameterListener("proximity", this);
    proxDistance = vtsParams.getRawParameterValue("proximity");
    vtsParams.addParameterListener("zeroDelayMode", this);
    zeroDelayMode = vtsParams.getRawParameterValue("zeroDelayMode");
    vtsParams.addParameterListener("syncChannel", this);
    syncChannelPtr = vtsParams.getRawParameterValue("syncChannel");
    
    // properties file: saves user preset folder location
    PropertiesFile::Options options;
    options.applicationName     = "PolarDesigner";
    options.filenameSuffix      = "settings";
    options.folderName          = "AustrianAudio";
    options.osxLibrarySubFolder = "Preferences";
    
    properties = std::unique_ptr<PropertiesFile>(new PropertiesFile (options));
    lastDir = File(properties->getValue ("presetFolder"));
    
    dfEqOmniBuffer.copyFrom(0, 0, DFEQ_COEFFS_OMNI, DF_EQ_LEN);
    dfEqEightBuffer.copyFrom(0, 0, DFEQ_COEFFS_EIGHT, DF_EQ_LEN);
    ffEqOmniBuffer.copyFrom(0, 0, FFEQ_COEFFS_OMNI, FF_EQ_LEN);
    ffEqEightBuffer.copyFrom(0, 0, FFEQ_COEFFS_EIGHT, FF_EQ_LEN);
    
    updateLatency();
    delay.setDelayTime (std::ceilf(static_cast<float>(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE) / 2 - 1) / FILTER_BANK_NATIVE_SAMPLE_RATE);
    
    oldProxDistance = proxDistance->load();
    
    startTimer(50);
}

PolarDesignerAudioProcessor::~PolarDesignerAudioProcessor()
{
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PolarDesignerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PolarDesignerAudioProcessor::setCurrentProgram (int index)
{
}

const String PolarDesignerAudioProcessor::getProgramName (int index)
{
    return {};
}

void PolarDesignerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void PolarDesignerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (sampleRate != currentSampleRate)
    {
        firLen = std::ceil(static_cast<float>(FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE) / FILTER_BANK_NATIVE_SAMPLE_RATE * sampleRate);
        if (firLen % 2 == 0) // make sure firLen is odd
            firLen++;
        
        updateLatency();
    }
    
    currentBlockSize = samplesPerBlock;
    currentSampleRate = sampleRate;
    
    dsp::ProcessSpec delaySpec {currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    delay.prepare (delaySpec);
    delayBuffer.clear();
    delayBuffer.setSize(1, currentBlockSize);
    
    // filter bank
    filterBankBuffer.setSize(N_CH_IN * 5, currentBlockSize);
    filterBankBuffer.clear();
    firFilterBuffer.setSize(5, firLen);
    firFilterBuffer.clear();
    omniEightBuffer.setSize(2, currentBlockSize);
    omniEightBuffer.clear();
    
    computeAllFilterCoefficients();
    initAllConvolvers();
    for (auto &conv : convolvers)
    {
        conv.reset();
    }
    
    // diffuse field eq
    dsp::ProcessSpec eqSpec {currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    dfEqOmniConv.prepare (eqSpec); // must be called before loading an ir
    dfEqOmniConv.loadImpulseResponse(std::move(dfEqOmniBuffer), EQ_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    
	dsp::ProcessSpec eqSpec2{ currentSampleRate, static_cast<uint32>(currentBlockSize), 1 };
    dfEqEightConv.prepare (eqSpec2);
    dfEqOmniConv.loadImpulseResponse(std::move(dfEqEightBuffer), EQ_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    
	dsp::ProcessSpec eqSpec3{ currentSampleRate, static_cast<uint32>(currentBlockSize), 1 };
    ffEqOmniConv.prepare (eqSpec3); // must be called before loading an ir
    dfEqOmniConv.loadImpulseResponse(std::move(ffEqOmniBuffer), EQ_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    
	dsp::ProcessSpec eqSpec4{ currentSampleRate, static_cast<uint32>(currentBlockSize), 1 };
    ffEqEightConv.prepare (eqSpec4);
    dfEqOmniConv.loadImpulseResponse(std::move(dfEqEightBuffer), EQ_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    
    dfEqOmniConv.reset();
    dfEqEightConv.reset();
    ffEqOmniConv.reset();
    ffEqEightConv.reset();
    
    for (int i = 0; i < 5; ++i)
    {
        oldDirFactors[i] = dirFactors[i]->load();
        oldBandGains[i] = bandGains[i]->load();
    }
    
    // proximity compensation IIR
    dsp::ProcessSpec specProx { currentSampleRate, static_cast<uint32> (currentBlockSize), 1 };
    proxCompIIR.prepare(specProx);
    
    proxCompIIR.reset();
    setProxCompCoefficients(proxDistance->load());
    
}

void PolarDesignerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

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

void PolarDesignerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    
    if (isBypassed) {
        isBypassed = false;
        updateLatency();
    }
    
    int numSamples = buffer.getNumSamples();
    
    // create omni and eight signals
    createOmniAndEightSignals (buffer);
    
    // proximity compensation filter
    if (zeroDelayMode->load() < 0.5f && proxDistance->load() < -0.05) // reduce proximity effect only on figure-of-eight
    {
        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> eightBlock(&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> contextProx(eightBlock);
        proxCompIIR.process(contextProx);
    }
    else if (zeroDelayMode->load() < 0.5f && proxDistance->load() > 0.05) // apply proximity to omni
    {
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> omniBlock(&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> contextProx(omniBlock);
        proxCompIIR.process(contextProx);
    }
    
    if (doEq == 1 && zeroDelayMode->load() < 0.5f )
    {
        // free field equalization
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> ffEqOmniBlk(&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> ffEqOmniCtx (ffEqOmniBlk);
        ffEqOmniConv.process(ffEqOmniCtx);

        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> ffEqEightBlk(&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> ffEqEightCtx (ffEqEightBlk);
        ffEqEightConv.process(ffEqEightCtx);
    }
    else if (doEq == 2 && zeroDelayMode->load() < 0.5f )
    {
        // diffuse field equalization
        float* writePointerOmni = omniEightBuffer.getWritePointer (0);
        dsp::AudioBlock<float> dfEqOmniBlk(&writePointerOmni, 1, numSamples);
        dsp::ProcessContextReplacing<float> dfEqOmniCtx (dfEqOmniBlk);
        dfEqOmniConv.process(dfEqOmniCtx);
        
        float* writePointerEight = omniEightBuffer.getWritePointer (1);
        dsp::AudioBlock<float> dfEqEightBlk(&writePointerEight, 1, numSamples);
        dsp::ProcessContextReplacing<float> dfEqEightCtx (dfEqEightBlk);
        dfEqEightConv.process(dfEqEightCtx);
    }
    
    int nActiveBands = nBands;
    if (zeroDelayMode->load() > 0.5f )
        nActiveBands = 1;
    
    for (int i = 0; i < nActiveBands; ++i)
    {
        // copy input buffer for each band
        filterBankBuffer.copyFrom (2*i, 0, omniEightBuffer, 0, 0, numSamples);
        filterBankBuffer.copyFrom (2*i+1, 0, omniEightBuffer, 1, 0, numSamples);
    }
    
    if (zeroDelayMode->load() < 0.5f && nActiveBands > 1)
    {
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
    }
    
    if (trackingActive)
        trackSignalEnergy();
    
    createPolarPatterns (buffer);
}

void PolarDesignerAudioProcessor::processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    if (!isBypassed) {
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
    return new PolarDesignerAudioProcessorEditor (*this, vtsParams);
}

//==============================================================================

void PolarDesignerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    vtsParams.state.setProperty("ffDfEq", var(doEq), nullptr);
    vtsParams.state.setProperty("oldProxDistance", var(oldProxDistance), nullptr);
    std::unique_ptr<XmlElement> xml (vtsParams.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PolarDesignerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (vtsParams.state.getType()))
        {
            vtsParams.state = ValueTree::fromXml (*xmlState);
        }
    }
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
    nBands = static_cast<int>(nBandsPtr->load()) + 1;
    nActiveBandsChanged = true;
    zeroDelayModeChanged = true;
    ffDfEqChanged = true;
    computeAllFilterCoefficients();
    initAllConvolvers();
    repaintDEQ = true;
}

void PolarDesignerAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID.startsWith("xOverF") && !loadingFile)
    {
        int idx = parameterID.getTrailingIntValue() - 1;
        computeFilterCoefficients(idx);
        initConvolver(idx);
        repaintDEQ = true;
    }
    else if (parameterID.startsWith("solo"))
    {
        soloActive = false;
        for (int i = 0; i<nBands; ++i) {
            if (soloBand[i]->load() >= 0.5)
                soloActive = true;
        }
    }
    else if (parameterID.startsWith("alpha"))
    {
        repaintDEQ = true;
    }
    else if (parameterID == "nrBands")
    {
        nBands = static_cast<int> (nBandsPtr->load()) + 1;
        resetXoverFreqs();
        nActiveBandsChanged = true;
        computeAllFilterCoefficients();
        initAllConvolvers();
    }
    else if (parameterID == "proximity")
    {
        setProxCompCoefficients(proxDistance->load());
    }
    else if (parameterID == "zeroDelayMode")
    {
        updateLatency();
        
        if (newValue == 0)
        {
            vtsParams.getParameter ("proximity")->setValueNotifyingHost (vtsParams.getParameter("proximity")->convertTo0to1(oldProxDistance));
            zeroDelayModeChanged = true;
            computeAllFilterCoefficients();
            initAllConvolvers();
        }
        else
        {
            oldProxDistance = proxDistance->load();
            vtsParams.getParameter ("proximity")->setValueNotifyingHost (vtsParams.getParameter("proximity")->convertTo0to1(0));
            zeroDelayModeChanged = true;
        }
    }
    else if (parameterID == "syncChannel" && syncChannelPtr->load() >= 0.5f)
    {
        int ch = (int) syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference(ch);
        
        if (!paramsToSync.paramsValid) // init all params
        {
            for (int i = 0; i < 5; ++i)
            {
                paramsToSync.solo[i] = soloBand[i]->load();
                paramsToSync.mute[i] = muteBand[i]->load();
                paramsToSync.dirFactors[i] = dirFactors[i]->load();
                paramsToSync.gains[i] = bandGains[i]->load();
                
                if (i < 4)
                    paramsToSync.xOverFreqs[i] = xOverFreqs[i]->load();
            }
            
            paramsToSync.nrActiveBands = nBandsPtr->load();
            paramsToSync.proximity = proxDistance->load();
            paramsToSync.zeroDelayMode = zeroDelayMode->load();
            paramsToSync.allowBackwardsPattern = allowBackwardsPattern->load();
            paramsToSync.ffDfEq = doEq;
        }
    }
    
    // if parameters are synced -> set sharedParams
    if (syncChannelPtr->load() >= 0.5f && !readingSharedParams)
    {
        int ch = (int) syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference(ch);
        
        if (parameterID.startsWith("xOverF") && !loadingFile)
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.xOverFreqs[idx] = xOverFreqs[idx]->load();
        }
        else if (parameterID.startsWith("solo"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.solo[idx] = soloBand[idx]->load();
        }
        else if (parameterID.startsWith("mute"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.mute[idx] = muteBand[idx]->load();
        }
        else if (parameterID.startsWith("alpha"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.dirFactors[idx] = dirFactors[idx]->load();
        }
        else if (parameterID == "nrBands")
        {
            paramsToSync.nrActiveBands = nBandsPtr->load();
        }
        else if (parameterID == "proximity")
        {
            paramsToSync.proximity = proxDistance->load();
        }
        else if (parameterID == "zeroDelayMode")
        {
            paramsToSync.zeroDelayMode = zeroDelayMode->load();
        }
        else if (parameterID.startsWith("gain"))
        {
            int idx = parameterID.getTrailingIntValue() - 1;
            paramsToSync.gains[idx] = bandGains[idx]->load();
        }
        else if (parameterID == "allowBackwardsPattern")
        {
            paramsToSync.allowBackwardsPattern = allowBackwardsPattern->load();
        }
        
    }
}

void PolarDesignerAudioProcessor::setEqState(int idx)
{
    doEq = idx;
    
    if (syncChannelPtr->load() >= 0.5f && !readingSharedParams)
    {
        int ch = (int) syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference(ch);
        paramsToSync.ffDfEq = doEq;
    }
}

void PolarDesignerAudioProcessor::resetXoverFreqs()
{
    switch (nBands) {
        case 1:
            break;
            
        case 2:
            for (int i = 0; i < nBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String(i+1))->setValueNotifyingHost (hzToZeroToOne(i, INIT_XOVER_FREQS_2B[i]));
            }
            break;
            
        case 3:
            for (int i = 0; i < nBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String(i+1))->setValueNotifyingHost (hzToZeroToOne(i, INIT_XOVER_FREQS_3B[i]));
            }
            break;
            
        case 4:
            for (int i = 0; i < nBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String(i+1))->setValueNotifyingHost (hzToZeroToOne(i, INIT_XOVER_FREQS_4B[i]));
            }
            break;
            
        case 5:
            for (int i = 0; i < nBands - 1; ++i)
            {
                vtsParams.getParameter ("xOverF" + String(i+1))->setValueNotifyingHost (hzToZeroToOne(i, INIT_XOVER_FREQS_5B[i]));
            }
            break;
            
        default:
            jassert(false);
            break;
    }
}

// compute filter bank filter coeffs and store in firFilterBuffer
void PolarDesignerAudioProcessor::computeAllFilterCoefficients()
{
    for (int i = 0; i < 4; ++i)
    {
        computeFilterCoefficients(i);
    }
}

void PolarDesignerAudioProcessor::computeFilterCoefficients(int crossoverNr)
{
    // only one band: no filtering
    if (nBands == 1)
        return;
    
    // lowest band is simple lowpass
    if (crossoverNr == 0)
    {
        dsp::FilterDesign<float>::FIRCoefficientsPtr lowpass = dsp::FilterDesign<float>::designFIRLowpassWindowMethod(hzFromZeroToOne(0, xOverFreqs[0]->load()), currentSampleRate, firLen - 1, dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lpCoeffs = lowpass->getRawCoefficients();
        firFilterBuffer.copyFrom(0, 0, lpCoeffs, firLen);
    }
    
    // all the other bands are bandpass filters
    for (int i = std::max(1, crossoverNr); i < std::min(crossoverNr + 2, nBands - 1); ++i)
    {
        float halfBandwidth = (hzFromZeroToOne(i, xOverFreqs[i]->load()) - hzFromZeroToOne(i-1, xOverFreqs[i-1]->load())) / 2;
        dsp::FilterDesign<float>::FIRCoefficientsPtr lp2bp = dsp::FilterDesign<float>::designFIRLowpassWindowMethod(halfBandwidth, currentSampleRate, firLen - 1, dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lp2bpCoeffs = lp2bp->getRawCoefficients();
        auto* filterBufferPointer = firFilterBuffer.getWritePointer(i);
        for (int j=0; j<firLen; j++) // bandpass transform
        {
            float fCenter = halfBandwidth + hzFromZeroToOne(i-1, xOverFreqs[i-1]->load());
            // write bandpass transformed fir coeffs to buffer
            *(filterBufferPointer+j) = 2 * *(lp2bpCoeffs+j) * std::cosf(MathConstants<float>::twoPi * fCenter / currentSampleRate * (j - (firLen - 1) / 2));
        }
    }
    
    if (crossoverNr == nBands - 2)
    {
        // highest band is highpass (via frequency transform)
        float hpBandwidth = currentSampleRate / 2 - hzFromZeroToOne(nBands - 2, xOverFreqs[nBands-2]->load());
        auto* filterBufferPointer = firFilterBuffer.getWritePointer(nBands-1);
        dsp::FilterDesign<float>::FIRCoefficientsPtr lp2hp = dsp::FilterDesign<float>::designFIRLowpassWindowMethod(hpBandwidth, currentSampleRate, firLen - 1, dsp::WindowingFunction<float>::WindowingMethod::hamming);
        float* lp2hpCoeffs = lp2hp->getRawCoefficients();
        for (int i=0; i<firLen; ++i) // highpass transform
        {
            *(filterBufferPointer+i) = *(lp2hpCoeffs+i) * std::cosf(MathConstants<float>::pi * (i - (firLen - 1) / 2));
        }
    }
    
}

void PolarDesignerAudioProcessor::initAllConvolvers()
{
    // build filters and fill firFilterBuffer
    dsp::AudioBlock<float> convBlk (firFilterBuffer);
    dsp::ProcessSpec convSpec {currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    for (int i = 0; i < nBands; ++i) // prepare nBands mono convolvers
    {
        AudioBuffer<float> convSingleBuffOmni(1, firLen);
        convSingleBuffOmni.copyFrom(0, 0, firFilterBuffer, i, 0, firLen);
        
        AudioBuffer<float> convSingleBuffEight(1, firLen);
        convSingleBuffEight.copyFrom(0, 0, firFilterBuffer, i, 0, firLen);

        // omni convolver
        convolvers[2 * i].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i].loadImpulseResponse(std::move(convSingleBuffOmni), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
        
        // eight convolver
        convolvers[2 * i + 1].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i + 1].loadImpulseResponse(std::move(convSingleBuffEight), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
    }
}

void PolarDesignerAudioProcessor::initConvolver(int convNr)
{
    // build filters and fill firFilterBuffer
    dsp::AudioBlock<float> convBlk (firFilterBuffer);
    dsp::ProcessSpec convSpec {currentSampleRate, static_cast<uint32>(currentBlockSize), 1};
    
    // update two convolvers: if one crossover frequency changes, two neighbouring bands need new filters
    for (int i = convNr; i < convNr + 2; ++i)
    {
        AudioBuffer<float> convSingleBuffOmni(1, firLen);
        convSingleBuffOmni.copyFrom(0, 0, firFilterBuffer, i, 0, firLen);
        
        AudioBuffer<float> convSingleBuffEight(1, firLen);
        convSingleBuffEight.copyFrom(0, 0, firFilterBuffer, i, 0, firLen);
        
        // omni convolver
        convolvers[2 * i].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i].loadImpulseResponse(std::move(convSingleBuffOmni), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
        
        // eight convolver
        convolvers[2 * i + 1].prepare (convSpec); // must be called before loading IR
        convolvers[2 * i + 1].loadImpulseResponse(std::move(convSingleBuffEight), currentSampleRate, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
    }
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

void PolarDesignerAudioProcessor::createPolarPatterns(AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    buffer.clear();
    
    int nActiveBands = nBands;
    if (zeroDelayMode->load() > 0.5f)
        nActiveBands = 1;
    
    for (int i = 0; i < nActiveBands; ++i)
    {
        if ((muteBand[i]->load() > 0.5 && soloBand[i]->load() < 0.5) || (soloActive && soloBand[i]->load() < 0.5))
            continue;
        
        // calculate patterns and add to output buffer
        const float* readPointerOmni = filterBankBuffer.getReadPointer (2 * i);
        const float* readPointerEight = filterBankBuffer.getReadPointer (2 * i + 1);
        
        float oldGain = Decibels::decibelsToGain(oldBandGains[i], -59.91f);
        float gain = Decibels::decibelsToGain(bandGains[i]->load(), -59.91f);
        
        // add with ramp to prevent crackling noises
        buffer.addFromWithRamp(0, 0, readPointerOmni, numSamples,
                               (1 - std::abs (oldDirFactors[i])) * oldGain,
                               (1 - std::abs (dirFactors[i]->load())) * gain);
        buffer.addFromWithRamp(0, 0, readPointerEight, numSamples,
                               oldDirFactors[i] * oldGain,
                               dirFactors[i]->load() * gain);
        
        oldDirFactors[i] = dirFactors[i]->load();
        oldBandGains[i] = bandGains[i]->load();
    }
    
    // delay needs to be running constantly to prevent clicks
    delayBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    dsp::AudioBlock<float> delayBlock(delayBuffer);
    dsp::ProcessContextReplacing<float> delayContext(delayBlock);
    delay.process(delayContext);
    
    if (nActiveBands == 1 && zeroDelayMode->load() < 0.5f) {
        buffer.copyFrom(0, 0, delayBuffer, 0, 0, numSamples);
    }
    
    // copy to second output channel -> this generates loud glitches in pro tools if mono output configuration is used
    // -> check getMainBusNumOutputChannels()
    if (buffer.getNumChannels() == 2 && getMainBusNumOutputChannels() == 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
}

void PolarDesignerAudioProcessor::setLastDir(File newLastDir)
{
    lastDir = newLastDir;
    const var v (lastDir.getFullPathName());
    properties->setValue ("presetFolder", v);
}

Result PolarDesignerAudioProcessor::loadPreset(const File& presetFile)
{
    var parsedJson;
    if (!presetFile.exists())
        return Result::fail ("File does not exist!");
    
    String jsonString = presetFile.loadFileAsString();
    Result result = JSON::parse (jsonString, parsedJson);
    if (!result.wasOk())
        return Result::fail ("File could not be parsed: Please provide valid JSON!");
    
    for (auto &it : presetProperties)
    {
        if (!parsedJson.hasProperty (it))
            return Result::fail ("Corrupt preset file: No '" + it + "' property found.");
    }
    
    loadingFile = true;
    
    float x = parsedJson.getProperty ("nrActiveBands", parsedJson);
    vtsParams.getParameter ("nrBands")->setValueNotifyingHost (vtsParams.getParameter ("nrBands")->convertTo0to1(x - 1));
    
    for (int i = 0; i < 4; ++i)
    {
        x = parsedJson.getProperty ("xOverF" + String(i+1), parsedJson);
        vtsParams.getParameter ("xOverF" + String(i+1))->setValueNotifyingHost (hzToZeroToOne(i, x));
    }
    
    NormalisableRange<float> dfRange = vtsParams.getParameter("alpha1")->getNormalisableRange();
    
    for (int i = 0; i < 5; ++i)
    {
        x = parsedJson.getProperty ("dirFactor" + String(i+1), parsedJson);
        if (x < dfRange.start || x > dfRange.end)
            return Result::fail ("DirFactor" + String(i+1) + " needs to be between " + String(dfRange.start) + " and " + String(dfRange.end) + ".");
        vtsParams.getParameter ("alpha" + String(i+1))->setValueNotifyingHost (dfRange.convertTo0to1(x));
        
        x = parsedJson.getProperty ("gain" + String(i+1), parsedJson);
        vtsParams.getParameter ("gain" + String(i+1))->setValueNotifyingHost (vtsParams.getParameter("gain1")->convertTo0to1(x));
        
        x = parsedJson.getProperty ("solo" + String(i+1), parsedJson);
        vtsParams.getParameter ("solo" + String(i+1))->setValueNotifyingHost (vtsParams.getParameter("solo1")->convertTo0to1(x));
        
        x = parsedJson.getProperty ("mute" + String(i+1), parsedJson);
        vtsParams.getParameter ("mute" + String(i+1))->setValueNotifyingHost (vtsParams.getParameter("solo1")->convertTo0to1(x));
    }
    
    doEq = parsedJson.getProperty ("ffDfEq", parsedJson);
    
    x = parsedJson.getProperty ("proximity", parsedJson);
    vtsParams.getParameter ("proximity")->setValueNotifyingHost (vtsParams.getParameter("proximity")->convertTo0to1(x));
    
    loadingFile = false;
    
    // set parameters
    nBands = static_cast<int>(nBandsPtr->load()) + 1;
    nActiveBandsChanged = true;
    computeAllFilterCoefficients();
    initAllConvolvers();
    repaintDEQ = true;
    
    return Result::ok();
}

Result PolarDesignerAudioProcessor::savePreset (File destination)
{
    DynamicObject* jsonObj = new DynamicObject();
    char versionString[10];
    strcpy(versionString, "v");
    strcat(versionString, JucePlugin_VersionString);
    jsonObj->setProperty("Description", var("This preset file was created with the Austrian Audio PolarDesigner plugin "
                                            + String(versionString) + ", for more information see www.austrian.audio ."));
    jsonObj->setProperty ("nrActiveBands", nBands);
    jsonObj->setProperty ("xOverF1", static_cast<int>(hzFromZeroToOne(0, xOverFreqs[0]->load())));
    jsonObj->setProperty ("xOverF2", static_cast<int>(hzFromZeroToOne(1, xOverFreqs[1]->load())));
    jsonObj->setProperty ("xOverF3", static_cast<int>(hzFromZeroToOne(2, xOverFreqs[2]->load())));
    jsonObj->setProperty ("xOverF4", static_cast<int>(hzFromZeroToOne(3, xOverFreqs[3]->load())));
    jsonObj->setProperty ("dirFactor1", dirFactors[0]->load());
    jsonObj->setProperty ("dirFactor2", dirFactors[1]->load());
    jsonObj->setProperty ("dirFactor3", dirFactors[2]->load());
    jsonObj->setProperty ("dirFactor4", dirFactors[3]->load());
    jsonObj->setProperty ("dirFactor5", dirFactors[4]->load());
    jsonObj->setProperty ("gain1", bandGains[0]->load());
    jsonObj->setProperty ("gain2", bandGains[1]->load());
    jsonObj->setProperty ("gain3", bandGains[2]->load());
    jsonObj->setProperty ("gain4", bandGains[3]->load());
    jsonObj->setProperty ("gain5", bandGains[4]->load());
    jsonObj->setProperty ("solo1", soloBand[0]->load());
    jsonObj->setProperty ("solo2", soloBand[1]->load());
    jsonObj->setProperty ("solo3", soloBand[2]->load());
    jsonObj->setProperty ("solo4", soloBand[3]->load());
    jsonObj->setProperty ("solo5", soloBand[4]->load());
    jsonObj->setProperty ("mute1", muteBand[0]->load());
    jsonObj->setProperty ("mute2", muteBand[1]->load());
    jsonObj->setProperty ("mute3", muteBand[2]->load());
    jsonObj->setProperty ("mute4", muteBand[3]->load());
    jsonObj->setProperty ("mute5", muteBand[4]->load());
    jsonObj->setProperty ("ffDfEq", doEq);
    jsonObj->setProperty ("proximity", proxDistance->load());

    String jsonString = JSON::toString (var (jsonObj), false, 2);
    if (destination.replaceWithText (jsonString))
        return Result::ok();
    else
        return Result::fail ("Could not write preset file. Check file access permissions.");
}

float PolarDesignerAudioProcessor::hzToZeroToOne(int idx, float hz)
{
    switch (nBands) {
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
            jassert(false);
            break;
    }
    return 0;
}

float PolarDesignerAudioProcessor::hzFromZeroToOne(int idx, float val)
{
    switch (nBands) {
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
            jassert(false);
            break;
    }
    return 0;
}

float PolarDesignerAudioProcessor::getXoverSliderRangeStart (int sliderNum)
{
    switch (nBands) {
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
            jassert(false);
            break;
    }
    return 0;
}

float PolarDesignerAudioProcessor::getXoverSliderRangeEnd (int sliderNum)
{
    switch (nBands) {
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
            jassert(false);
            break;
    }
    return 0;
}

void PolarDesignerAudioProcessor::startTracking(bool trackDisturber)
{
    if (trackDisturber)
    {
        trackingDisturber = true;
        for (int i = 0; i < 5; ++i)
        {
            omniSqSumDist[i] = 0.0f;
            eightSqSumDist[i] = 0.0f;
            omniEightSumDist[i] = 0.0f;
        }
    }
    else
    {
        trackingDisturber = false;
        for (int i = 0; i < 5; ++i)
        {
            omniSqSumSig[i] = 0.0f;
            eightSqSumSig[i] = 0.0f;
            omniEightSumSig[i] = 0.0f;
        }
    }
    
    nrBlocksRecorded = 0;
    trackingActive = true;
}

void PolarDesignerAudioProcessor::stopTracking(int applyOptimalPattern)
{
    trackingActive = false;
    if (applyOptimalPattern == 1)
    {
        if (trackingDisturber)
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < 5; ++i)
                {
                    omniSqSumDist[i] = omniSqSumDist[i]/nrBlocksRecorded;
                    eightSqSumDist[i] = eightSqSumDist[i]/nrBlocksRecorded;
                    omniEightSumDist[i] = omniEightSumDist[i]/nrBlocksRecorded;
                }
            }
            setMinimumDisturbancePattern();
        }
        else
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < 5; ++i)
                {
                    omniSqSumSig[i] = omniSqSumSig[i]/nrBlocksRecorded;
                    eightSqSumSig[i] = eightSqSumSig[i]/nrBlocksRecorded;
                    omniEightSumSig[i] = omniEightSumSig[i]/nrBlocksRecorded;
                }
            }
            setMaximumSignalPattern();
        }
    }
    else if (applyOptimalPattern == 2) // max sig-to-dist
    {
        if (trackingDisturber)
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < 5; ++i)
                {
                    omniSqSumDist[i] = omniSqSumDist[i]/nrBlocksRecorded;
                    eightSqSumDist[i] = eightSqSumDist[i]/nrBlocksRecorded;
                    omniEightSumDist[i] = omniEightSumDist[i]/nrBlocksRecorded;
                }
            }
            disturberRecorded = true;
        }
        else
        {
            if (nrBlocksRecorded != 0)
            {
                for (int i = 0; i < 5; ++i)
                {
                    omniSqSumSig[i] = omniSqSumSig[i]/nrBlocksRecorded;
                    eightSqSumSig[i] = eightSqSumSig[i]/nrBlocksRecorded;
                    omniEightSumSig[i] = omniEightSumSig[i]/nrBlocksRecorded;
                }
            }
            signalRecorded = true;
        }
        maximizeSigToDistRatio();
    }
}

void PolarDesignerAudioProcessor::trackSignalEnergy()
{
    int numSamples = filterBankBuffer.getNumSamples();
    for (int i = 0; i < nBands; ++i)
    {
        const float* readPointerOmni = filterBankBuffer.getReadPointer (2*i);
        const float* readPointerEight = filterBankBuffer.getReadPointer (2*i+1);
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
    float disturberPower;
    float minPowerAlpha;
    float alphaStart = 0.0f;
    if (allowBackwardsPattern->load() == 1.0f)
        alphaStart = -0.5f;
    
    for (int i = 0; i<nBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentPower = std::pow((1-std::abs(alpha)), 2) * omniSqSumDist[i] + std::pow(alpha, 2) * eightSqSumDist[i] + 2 * (1-std::abs(alpha)) * alpha * omniEightSumDist[i];
            if (alpha == alphaStart || currentPower < disturberPower)
            {
                disturberPower = currentPower;
                minPowerAlpha = alpha;
            }
        }
        if (disturberPower != 0.0f) // do not apply changes, if playback is not active
        {
            vtsParams.getParameter ("alpha" + String(i+1))->setValueNotifyingHost (vtsParams.getParameter("alpha1")->convertTo0to1 (minPowerAlpha));
            disturberRecorded = true;
        }
    }
}

void PolarDesignerAudioProcessor::setMaximumSignalPattern()
{
    float signalPower;
    float maxPowerAlpha;
    float alphaStart = 0.0f;
    if (allowBackwardsPattern->load() == 1.0f)
        alphaStart = -0.5f;
    
    for (int i = 0; i < nBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentPower = std::pow((1-std::abs(alpha)), 2) * omniSqSumSig[i] + std::pow(alpha, 2) * eightSqSumSig[i] + 2 * (1-std::abs(alpha)) * alpha * omniEightSumSig[i];
            if (alpha == alphaStart || currentPower > signalPower)
            {
                signalPower = currentPower;
                maxPowerAlpha = alpha;
            }
        }
        if (signalPower != 0.0f)
        {
            vtsParams.getParameter ("alpha" + String(i+1))->setValueNotifyingHost (vtsParams.getParameter("alpha1")->convertTo0to1 (maxPowerAlpha));
            signalRecorded = true;
        }
    }
}

void PolarDesignerAudioProcessor::maximizeSigToDistRatio()
{
    float distToSigRatio;
    float maxDistToSigAlpha;
    float alphaStart = 0.0f;
    if (allowBackwardsPattern->load() == 1.0f)
        alphaStart = -0.5f;
    
    for (int i = 0; i<nBands; ++i)
    {
        for (float alpha = alphaStart; alpha <= 1.0f; alpha += 0.01f)
        {
            float currentSigPower = std::pow((1-std::abs(alpha)), 2) * omniSqSumSig[i] + std::pow(alpha, 2) * eightSqSumSig[i] + 2 * (1-std::abs(alpha)) * alpha * omniEightSumSig[i];
            float currentDistPower = std::pow((1-std::abs(alpha)), 2) * omniSqSumDist[i] + std::pow(alpha, 2) * eightSqSumDist[i] + 2 * (1-std::abs(alpha)) * alpha * omniEightSumDist[i];
            float currentRatio;
            if (currentDistPower == 0.0f)
                currentRatio = 0.0f;
            else
                currentRatio = currentSigPower/currentDistPower;
            
            if (alpha == alphaStart || currentRatio > distToSigRatio)
            {
                distToSigRatio = currentRatio;
                maxDistToSigAlpha = alpha;
            }
        }
        if (distToSigRatio != 0.0f)
            vtsParams.getParameter ("alpha" + String(i+1))->setValueNotifyingHost (vtsParams.getParameter("alpha1")->convertTo0to1 (maxDistToSigAlpha));
    }
}

void PolarDesignerAudioProcessor::setProxCompCoefficients(float distance)
{
    int c = 343;
    double fs = getSampleRate();
    
//    float b0 = -c / (fs * 4 * distance) + 1;
//    float b1 = -exp(-c / (fs * 2 * distance)) * (1 + c / (fs * 4 * distance));
//    float a0 = 1;
//    float a1 = -exp(-c / (fs * 2 * distance));
    
    // use logarithmic fader impact: equation is for fader between -1.0 .. 1.0
    // returns values between 1 .. 0.1
    float a = (0.05f - 1.0f) / (-log(1.1f) + log(0.1f));
    float b = 1 + a * log(0.1f);
    float r = -a * log(std::abs(distance) + 0.1) + b;

    float b0, b1, a0, a1;
    
    // normalized to r_ref = 1m
    if (distance <= 0) //bass cut
    {
        if (r < 0.01)
            r = 0.01;
        
        b0 = c * (r - 1) / (fs * 2 * r) + 1;
        b1 = -exp(-c / (fs * r)) * (1 - c * (r - 1) / (fs * 2 * r));
        a0 = 1;
        a1 = -exp(-c / (fs * r));
    }
    else // bass boost, careful: instable for r<0.05
    {
        if (r < 0.05)
            r = 0.05;
        
        b0 = c * (1 - r) / (fs * 2 * r) + 1;
        b1 = -exp(-c / fs) * (1 - c * (1 - r) / (fs * 2 * r));
        a0 = 1;
        a1 = -exp(-c / fs);
    }

    *proxCompIIR.coefficients = dsp::IIR::Coefficients<float>(b0,b1,a0,a1);
}

void PolarDesignerAudioProcessor::timerCallback()
{
    if (syncChannelPtr->load() > 0.5f)
    {
        readingSharedParams = true;
        
        int ch = (int) syncChannelPtr->load() - 1;
        ParamsToSync& paramsToSync = sharedParams.get().syncParams.getReference(ch);
        
        if (nBandsPtr->load() != paramsToSync.nrActiveBands)
            vtsParams.getParameter ("nrBands")->setValueNotifyingHost (vtsParams.getParameterRange ("nrBands").convertTo0to1 (paramsToSync.nrActiveBands));
        
        for (int i = 0; i < 5; ++i)
        {
            if (dirFactors[i]->load() != paramsToSync.dirFactors[i])
                vtsParams.getParameter ("alpha" + String(i+1))->setValueNotifyingHost (vtsParams.getParameterRange ("alpha" + String(i+1)).convertTo0to1 (paramsToSync.dirFactors[i]));
            
            if (soloBand[i]->load() != paramsToSync.solo[i])
                vtsParams.getParameter ("solo" + String(i+1))->setValueNotifyingHost (vtsParams.getParameterRange ("solo" + String(i+1)).convertTo0to1 (paramsToSync.solo[i]));
            
            if (muteBand[i]->load() != paramsToSync.mute[i])
                vtsParams.getParameter ("mute" + String(i+1))->setValueNotifyingHost (vtsParams.getParameterRange ("mute" + String(i+1)).convertTo0to1 (paramsToSync.mute[i]));
            
            if (bandGains[i]->load() != paramsToSync.gains[i])
                vtsParams.getParameter ("gain" + String(i+1))->setValueNotifyingHost (vtsParams.getParameterRange ("gain" + String(i+1)).convertTo0to1 (paramsToSync.gains[i]));
            
            if (i < 4 && xOverFreqs[i]->load() != paramsToSync.xOverFreqs[i])
                vtsParams.getParameter ("xOverF" + String(i+1))->setValueNotifyingHost (vtsParams.getParameterRange ("xOverF" + String(i+1)).convertTo0to1 (paramsToSync.xOverFreqs[i]));
            

        }
        
        if (proxDistance->load() != paramsToSync.proximity)
            vtsParams.getParameter ("proximity")->setValueNotifyingHost (vtsParams.getParameterRange ("proximity").convertTo0to1 (paramsToSync.proximity));
        
        if (zeroDelayMode->load() != paramsToSync.zeroDelayMode)
            vtsParams.getParameter ("zeroDelayMode")->setValueNotifyingHost (vtsParams.getParameterRange ("zeroDelayMode").convertTo0to1 (paramsToSync.zeroDelayMode));
        
        if (allowBackwardsPattern->load() != paramsToSync.allowBackwardsPattern)
            vtsParams.getParameter ("allowBackwardsPattern")->setValueNotifyingHost (vtsParams.getParameterRange ("allowBackwardsPattern").convertTo0to1 (paramsToSync.allowBackwardsPattern));
        
        if (paramsToSync.ffDfEq != doEq)
        {
            setEqState(paramsToSync.ffDfEq);
            ffDfEqChanged = true;
        }
        
        readingSharedParams = false;
    }
}

void PolarDesignerAudioProcessor::updateLatency() {
    if (isBypassed)
    {
        setLatencySamples(0);
    }
    else
    {
        // set delay compensation to FIR_LEN/2-1 if FIR_LEN even and FIR_LEN/2 if odd
        if (zeroDelayMode->load() < 0.5f)
            setLatencySamples(std::ceilf(static_cast<float>(firLen) / 2 - 1));
        else
            setLatencySamples(0);
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolarDesignerAudioProcessor();
}
