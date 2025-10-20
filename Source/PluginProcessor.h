/*
 ==============================================================================
 PluginProcessor.h
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

#include <JuceHeader.h>

#include "../resources/Delay.h"
#include <math.h>
#include <memory> // for unique_ptr
#include <vector>

#include <cfloat>

// For performance tuning
#ifdef PERFETTO
    #include "melatonin_perfetto/melatonin_perfetto.h"
#endif

static const int PD_DEFAULT_BLOCK_SIZE = 1024;

/* PolarDesigner has a maximum of 5 EQ's .. */
static const int MAX_NUM_EQS = 5;
/* .. and functions on a maximum of 2 inputs only. */
static const int MAX_NUM_INPUTS = 2;


static constexpr float PROXIMITY_THRESHOLD = 0.05f;
static constexpr float CROSSFADE_DURATION_SECONDS = 0.02f;
static constexpr int REPAINT_RATE_HZ = 30;

using namespace juce;

// these params can be synced between plugin instances
struct ParamsToSync
{
    int nrActiveBands, ffDfEq;
    float xOverFreqs[4], dirFactors[MAX_NUM_EQS], gains[MAX_NUM_EQS], proximity;
    bool solo[MAX_NUM_EQS], mute[MAX_NUM_EQS], allowBackwardsPattern, proximityOnOff, zeroLatencyMode, abLayer;
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
    Array<ParamsToSync> syncParams;
    unsigned int instanceCount;
};

// the A/B compare button layers
enum {
    COMPARE_LAYER_A = 0,
    COMPARE_LAYER_B = 1
};

enum eqBandStates : unsigned int {
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
class PolarDesignerAudioProcessor : public AudioProcessor, public AudioProcessorValueTreeState::Listener, private Timer

{
public:
    //==============================================================================
    PolarDesignerAudioProcessor();
    ~PolarDesignerAudioProcessor() override;

    void validateSampleRateAndBlockSize();

    void registerParameterListeners();

        // This is the ProTools PageFile for PolarDesigner3
    String getPageFileName() const override { return "PolarDesigner3.xml"; }

    //==============================================================================
    void resampleBuffer(const AudioBuffer<float>& src, AudioBuffer<float>& dst, float srcSampleRate, double dstSampleRate);
    void resampleBufferLagrange(const AudioBuffer<float>& src, AudioBuffer<float>& dst, float srcSampleRate, float dstSampleRate);

    void loadEqImpulseResponses();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    void processBlockBypassed (AudioBuffer<float>&, MidiBuffer&) override;
    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void resizeBuffersIfNeeded(int newFirLen, int newBlockSize);

    void initializeBuffers();
    void initializeDefaultState();
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    int getSyncChannelIdx();

    //==============================================================================
    void parameterChanged (const String& parameterID, float newValue) override;

    //==============================================================================
    Result loadPreset (const File& presetFile);
    Result savePreset (File destination);
    File getLastDir() { return lastDir; }
    void setLastDir (File newLastDir);

    void startTracking (bool trackDisturber);
    void stopTracking (int applyOptimalPattern);

    void setNProcessorBands(unsigned int numBands)
    {
        if (numBands >= 1 && numBands <= MAX_NUM_EQS) {
            nProcessorBands = numBands;
            activeBandsChanged = true;
            // Update any internal state as needed
            recomputeAllFilterCoefficients = true;
        }
    }

    unsigned int getNProcessorBands();

    CriticalSection convolutionLock;

    float getXoverSliderRangeStart (int sliderNum);
    float getXoverSliderRangeEnd (int sliderNum);

    std::atomic<bool> repaintDEQ = true;
    std::atomic<bool> activeBandsChanged = true;
    std::atomic<bool> zeroLatencyModeChanged = true;
    std::atomic<bool> ffDfEqChanged = true;
    std::array<std::atomic<bool>, 4> recomputeFilterCoefficients;
    std::atomic<bool> recomputeAllFilterCoefficients;

    bool getDisturberRecorded() { return disturberRecorded; }
    bool getSignalRecorded() { return signalRecorded; }

    void changeABLayerState (int state);
    bool disturberRecorded;
    bool signalRecorded;

    bool abLayerState = COMPARE_LAYER_A;

    Identifier saveTree = "save";
    Identifier nodeA = "layerA";
    Identifier nodeB = "layerB";
    Identifier nodeParams = "vtsParams";
    ValueTree layerA;
    ValueTree layerB;
    ValueTree saveStates;
    int doEq;
    int doEqA;
    int doEqB;

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


    AudioVisualiserComponent termControlWaveform;

    AudioPlayHead* audioPlayHead;
    AudioPlayHead::PositionInfo playHeadPosition;

    UndoManager undoManager;

    // initial xover frequencies for several numbers of bands
    const float INIT_XOVER_FREQS_2B[1] = { 1000.0f };
    const float INIT_XOVER_FREQS_3B[2] = { 250.0f, 3000.0f };
    const float INIT_XOVER_FREQS_4B[3] = { 200.0f, 1000.0f, 5000.0f };
    const float INIT_XOVER_FREQS_5B[4] = { 150.0f, 600.0f, 2600.0f, 8000.0f };

    // xover ranges for several numbers of bands
    const float XOVER_RANGE_START_2B[1] = { 120.0f };
    const float XOVER_RANGE_END_2B[1] = { 12000.0f };
    const float XOVER_RANGE_START_3B[2] = { 120.0f, 2000.0f };
    const float XOVER_RANGE_END_3B[2] = { 1000.0f, 12000.0f };
    const float XOVER_RANGE_START_4B[3] = { 120.0f, 900.0f, 4000.0f };
    const float XOVER_RANGE_END_4B[3] = { 450.0f, 2500.0f, 12000.0f };
    const float XOVER_RANGE_START_5B[4] = { 120.0f, 500.0f, 2200.0f, 7000.0f };
    const float XOVER_RANGE_END_5B[4] = { 200.0f, 1100.0f, 4000.0f, 12000.0f };

    int getEqState() { return doEq; }
    void setEqState (int idx);
    float hzToZeroToOne (int idx, float hz);
    float hzFromZeroToOne (int idx, float val);

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

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessor)

    std::unique_ptr<PropertiesFile> properties;

    std::atomic<unsigned int> nProcessorBands;

    AudioProcessorValueTreeState vtsParams;
    SharedResourcePointer<SharedParams> sharedParams;

    static const int N_CH_IN = 2;

    // use odd FIR_LEN for even filter order (FIR_LEN = N+1)
    // (lowpass and highpass need even filter order to put a zero at f=0 and f=pi)
    std::atomic<int> firLen{FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE};

    // free field / diffuse field eq
    dsp::Convolution dfEqOmniConv;
    dsp::Convolution dfEqEightConv;
    AudioBuffer<float> dfEqOmniBuffer;
    AudioBuffer<float> dfEqEightBuffer;
    dsp::Convolution ffEqOmniConv;
    dsp::Convolution ffEqEightConv;
    AudioBuffer<float> ffEqOmniBuffer;
    AudioBuffer<float> ffEqEightBuffer;

    // Single reusable temporary buffer - used during resampling
    AudioBuffer<float> tempBuffer;

    // proximity compensation filter
    dsp::IIR::Filter<float> proxCompIIR;

    std::atomic<bool> convolversReady;

    // delay (in case of 1 active band)
    Delay delay;
    AudioBuffer<float> delayBuffer;

    std::atomic<float>* nProcessorBandsPtr;
    std::atomic<float>* syncChannelPtr;
    //    float oldSyncChannelPtr;
    std::atomic<float>* xOverFreqsPtr[MAX_NUM_EQS - 1];
    std::atomic<float>* dirFactorsPtr[MAX_NUM_EQS];
    float oldDirFactors[MAX_NUM_EQS];
    std::atomic<float>* bandGainsPtr[MAX_NUM_EQS];
    float oldBandGains[MAX_NUM_EQS];
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
    std::atomic<bool>readingSharedParams;
    bool trackingActive;
    bool trackingDisturber;
    int nrBlocksRecorded;

    float omniSqSumDist[MAX_NUM_EQS], eightSqSumDist[MAX_NUM_EQS], omniEightSumDist[MAX_NUM_EQS],
        omniSqSumSig[MAX_NUM_EQS], eightSqSumSig[MAX_NUM_EQS], omniEightSumSig[MAX_NUM_EQS];

    AudioBuffer<float> filterBankBuffer; // holds filtered data, size: N_CH_IN*5
    AudioBuffer<float> firFilterBuffer; // holds filter coefficients, size: 5
    AudioBuffer<float> omniEightBuffer; // holds omni and fig-of-eight signals, size: 2

    //std::array<dsp::Convolution, 2 * MAX_NUM_EQS> convolvers;
    juce::OwnedArray<juce::dsp::Convolution> convolvers;


    // convolver cache
    // New members for optimization
    dsp::ProcessSpec lastEqSpec{0.0, 0, 0}; // Last spec for EQ convolvers
    dsp::ProcessSpec lastConvSpec{0.0, 0, 0}; // Track last convolver spec
    std::array<bool, MAX_NUM_EQS> bandCoefficientsChanged{false}; // Track which bands need updating
    AudioBuffer<float> cachedDfEqOmniBuffer; // Cached resampled EQ buffers
    AudioBuffer<float> cachedDfEqEightBuffer;
    AudioBuffer<float> cachedFfEqOmniBuffer;
    AudioBuffer<float> cachedFfEqEightBuffer;
    double lastEqSampleRate{0.0}; // Track last sample rate for EQ buffers

    double currentSampleRate = 0.0f;
    double previousSampleRate = 0.0f;

    // This is intentionally set to match Pro Tools expectations ...
    int currentBlockSize = PD_DEFAULT_BLOCK_SIZE;
    File lastDir;
    std::atomic<bool> resetXoverPending{false};

    //==============================================================================
    void resetXoverFreqs(bool useGestures);
    void computeAllFilterCoefficients();
    void computeFilterCoefficients (unsigned int crossoverNr);
    void setProxCompCoefficients (float distance);
    void initAllConvolvers();
    void initConvolver (int convNr);
    void loadFilterBankImpulseResponses();

    void createOmniAndEightSignals (AudioBuffer<float>& buffer);
    void createPolarPatterns (AudioBuffer<float>& buffer);
    void trackSignalEnergy();
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
    const String presetProperties[27] = { "nrActiveBands", "xOverF1", "xOverF2", "xOverF3", "xOverF4", "dirFactor1", "dirFactor2", "dirFactor3", "dirFactor4", "dirFactor5", "gain1", "gain2", "gain3", "gain4", "gain5", "solo1", "solo2", "solo3", "solo4", "solo5", "mute1", "mute2", "mute3", "mute4", "mute5", "ffDfEq", "proximity" };

    static const int FILTER_BANK_NATIVE_SAMPLE_RATE = 48000;
    static const int FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE = 401;

    static const int DF_EQ_LEN = 512;
    static const int FF_EQ_LEN = 512;
    static const int EQ_SAMPLE_RATE = 48000;
};

// DF = Diffuse Field
static const float DFEQ_COEFFS_OMNI[512] = { 0.893128F, 0.226831F, -0.305175F, 0.193475F, 0.104180F, -0.097501F, -0.221658F, -0.083621F, 0.113967F, 0.072912F, -0.036269F, -0.024528F, 0.038009F, 0.019054F, -0.045728F, -0.038984F, -0.016470F, -0.000284F, -0.016679F, -0.020151F, -0.010668F, 0.006171F, 0.002938F, -0.007214F, -0.006806F, -0.000164F, 0.003387F, 0.001469F, 0.001008F, 0.001173F, -0.002319F, -0.006493F, -0.010854F, -0.007776F, -0.004961F, 0.000228F, 0.000629F, 0.000716F, -0.003751F, -0.008161F, -0.012587F, -0.013443F, -0.010819F, -0.006502F, -0.002251F, -0.000186F, -0.000436F, -0.003019F, -0.007202F, -0.010357F, -0.012093F, -0.010233F, -0.008092F, -0.004354F, -0.002160F, 0.000321F, 0.000617F, 0.000590F, -0.000459F, -0.001157F, -0.001654F, -0.001893F, -0.001894F, -0.002119F, -0.002361F, -0.002649F, -0.002989F, -0.003048F, -0.003470F, -0.003290F, -0.003676F, -0.003413F, -0.003996F, -0.004074F, -0.004820F, -0.005060F, -0.005534F, -0.005585F, -0.005500F, -0.005177F, -0.004636F, -0.004128F, -0.003610F, -0.003249F, -0.003044F, -0.002773F, -0.002731F, -0.002350F, -0.002301F, -0.001796F, -0.001760F, -0.001366F, -0.001480F, -0.001361F, -0.001568F, -0.001626F, -0.001781F, -0.001899F, -0.001988F, -0.002136F, -0.002262F, -0.002461F, -0.002708F, -0.002900F, -0.003205F, -0.003276F, -0.003544F, -0.003469F, -0.003648F, -0.003474F, -0.003562F, -0.003379F, -0.003377F, -0.003207F, -0.003102F, -0.002939F, -0.002752F, -0.002581F, -0.002370F, -0.002192F, -0.002033F, -0.001867F, -0.001814F, -0.001663F, -0.001709F, -0.001576F, -0.001683F, -0.001577F, -0.001710F, -0.001666F, -0.001804F, -0.001839F, -0.001972F, -0.002073F, -0.002182F, -0.002301F, -0.002384F, -0.002475F, -0.002546F, -0.002578F, -0.002647F, -0.002619F, -0.002694F, -0.002588F, -0.002625F, -0.002482F, -0.002502F, -0.002348F, -0.002306F, -0.002184F, -0.002120F, -0.002031F, -0.001946F, -0.001879F, -0.001798F, -0.001757F, -0.001718F, -0.001669F, -0.001660F, -0.001624F, -0.001697F, -0.001650F, -0.001735F, -0.001708F, -0.001835F, -0.001829F, -0.001921F, -0.001940F, -0.002019F, -0.002056F, -0.002102F, -0.002130F, -0.002147F, -0.002173F, -0.002186F, -0.002182F, -0.002174F, -0.002122F, -0.002130F, -0.002054F, -0.002062F, -0.001961F, -0.001972F, -0.001876F, -0.001871F, -0.001782F, -0.001760F, -0.001704F, -0.001674F, -0.001643F, -0.001614F, -0.001605F, -0.001591F, -0.001586F, -0.001592F, -0.001577F, -0.001611F, -0.001588F, -0.001645F, -0.001620F, -0.001690F, -0.001671F, -0.001733F, -0.001722F, -0.001766F, -0.001767F, -0.001787F, -0.001799F, -0.001806F, -0.001820F, -0.001821F, -0.001826F, -0.001831F, -0.001817F, -0.001834F, -0.001803F, -0.001832F, -0.001792F, -0.001829F, -0.001790F, -0.001823F, -0.001797F, -0.001829F, -0.001826F, -0.001851F, -0.001871F, -0.001893F, -0.001924F, -0.001948F, -0.001982F, -0.002020F, -0.002050F, -0.002106F, -0.002126F, -0.002193F, -0.002203F, -0.002274F, -0.002278F, -0.002344F, -0.002348F, -0.002397F, -0.002403F, -0.002430F, -0.002439F, -0.002447F, -0.002456F, -0.002452F, -0.002452F, -0.002445F, -0.002429F, -0.002424F, -0.002387F, -0.002384F, -0.002330F, -0.002325F, -0.002260F, -0.002247F, -0.002179F, -0.002154F, -0.002091F, -0.002052F, -0.001996F, -0.001944F, -0.001893F, -0.001837F, -0.001788F, -0.001736F, -0.001683F, -0.001641F, -0.001579F, -0.001550F, -0.001485F, -0.001469F, -0.001407F, -0.001400F, -0.001349F, -0.001345F, -0.001311F, -0.001303F, -0.001280F, -0.001267F, -0.001256F, -0.001242F, -0.001234F, -0.001224F, -0.001216F, -0.001219F, -0.001211F, -0.001229F, -0.001212F, -0.001234F, -0.001218F, -0.001253F, -0.001239F, -0.001263F, -0.001258F, -0.001291F, -0.001296F, -0.001317F, -0.001333F, -0.001351F, -0.001377F, -0.001396F, -0.001418F, -0.001440F, -0.001458F, -0.001500F, -0.001514F, -0.001553F, -0.001558F, -0.001603F, -0.001608F, -0.001647F, -0.001657F, -0.001691F, -0.001711F, -0.001738F, -0.001761F, -0.001782F, -0.001809F, -0.001829F, -0.001850F, -0.001875F, -0.001891F, -0.001921F, -0.001933F, -0.001976F, -0.001982F, -0.002012F, -0.002012F, -0.002046F, -0.002052F, -0.002071F, -0.002076F, -0.002093F, -0.002100F, -0.002110F, -0.002112F, -0.002111F, -0.002115F, -0.002119F, -0.002121F, -0.002123F, -0.002107F, -0.002112F, -0.002095F, -0.002101F, -0.002071F, -0.002072F, -0.002047F, -0.002042F, -0.002014F, -0.001997F, -0.001975F, -0.001953F, -0.001931F, -0.001903F, -0.001881F, -0.001856F, -0.001830F, -0.001808F, -0.001773F, -0.001755F, -0.001714F, -0.001698F, -0.001655F, -0.001640F, -0.001601F, -0.001584F, -0.001548F, -0.001525F, -0.001494F, -0.001466F, -0.001440F, -0.001413F, -0.001390F, -0.001367F, -0.001344F, -0.001326F, -0.001299F, -0.001287F, -0.001259F, -0.001254F, -0.001226F, -0.001227F, -0.001205F, -0.001205F, -0.001189F, -0.001188F, -0.001179F, -0.001175F, -0.001173F, -0.001170F, -0.001171F, -0.001170F, -0.001169F, -0.001170F, -0.001164F, -0.001169F, -0.001158F, -0.001167F, -0.001155F, -0.001166F, -0.001154F, -0.001163F, -0.001154F, -0.001158F, -0.001152F, -0.001151F, -0.001150F, -0.001146F, -0.001146F, -0.001143F, -0.001141F, -0.001139F, -0.001132F, -0.001135F, -0.001124F, -0.001130F, -0.001116F, -0.001122F, -0.001107F, -0.001110F, -0.001097F, -0.001096F, -0.001088F, -0.001084F, -0.001077F, -0.001069F, -0.001063F, -0.001053F, -0.001045F, -0.001035F, -0.001023F, -0.001016F, -0.000999F, -0.000993F, -0.000975F, -0.000971F, -0.000950F, -0.000937F, -0.000916F, -0.000906F, -0.000891F, -0.000872F, -0.000856F, -0.000840F, -0.000824F, -0.000806F, -0.000784F, -0.000768F, -0.000747F, -0.000733F, -0.000712F, -0.000695F, -0.000669F, -0.000657F, -0.000634F, -0.000618F, -0.000593F, -0.000577F, -0.000556F, -0.000538F, -0.000517F, -0.000497F, -0.000479F, -0.000459F, -0.000439F, -0.000420F, -0.000401F, -0.000384F, -0.000364F, -0.000348F, -0.000328F, -0.000313F, -0.000293F, -0.000279F, -0.000260F, -0.000246F, -0.000230F, -0.000216F, -0.000201F, -0.000187F, -0.000174F, -0.000161F, -0.000149F, -0.000137F, -0.000125F, -0.000115F, -0.000104F, -0.000095F, -0.000085F, -0.000077F, -0.000069F, -0.000061F, -0.000053F, -0.000047F, -0.000041F, -0.000035F, -0.000030F, -0.000025F, -0.000021F, -0.000017F, -0.000013F, -0.000010F, -0.000007F, -0.000005F, -0.000003F, -0.000002F, -0.000001F, -0.000000F, -0.000000F };

static const float DFEQ_COEFFS_EIGHT[512] = { 1.099678F, 0.020727F, -0.218195F, 0.184517F, -0.013301F, 0.124938F, -0.088281F, -0.006001F, 0.053207F, 0.021356F, 0.030027F, -0.034382F, -0.041157F, -0.009659F, 0.010998F, 0.039171F, 0.015203F, 0.012945F, -0.009529F, -0.000595F, 0.004295F, 0.017541F, 0.014295F, 0.004369F, -0.006579F, -0.016732F, -0.019793F, -0.018894F, -0.010698F, -0.001548F, 0.003371F, 0.003905F, -0.002354F, -0.003260F, -0.006177F, -0.001519F, -0.001397F, 0.002962F, 0.002250F, 0.003706F, 0.001933F, 0.000684F, -0.001085F, -0.002236F, -0.001775F, -0.001165F, 0.000456F, 0.001712F, 0.002394F, 0.003130F, 0.002068F, 0.002155F, -0.000057F, -0.000102F, -0.002292F, -0.002009F, -0.003448F, -0.003178F, -0.004058F, -0.004035F, -0.004208F, -0.004130F, -0.003756F, -0.003639F, -0.003251F, -0.003088F, -0.003000F, -0.002668F, -0.002904F, -0.002318F, -0.002766F, -0.001991F, -0.002472F, -0.001668F, -0.001991F, -0.001288F, -0.001378F, -0.000977F, -0.000980F, -0.000990F, -0.001057F, -0.001348F, -0.001510F, -0.001803F, -0.002067F, -0.002200F, -0.002625F, -0.002591F, -0.003185F, -0.003044F, -0.003689F, -0.003477F, -0.003988F, -0.003758F, -0.004038F, -0.003873F, -0.003941F, -0.003872F, -0.003777F, -0.003729F, -0.003545F, -0.003431F, -0.003270F, -0.003057F, -0.003021F, -0.002723F, -0.002841F, -0.002495F, -0.002716F, -0.002384F, -0.002623F, -0.002371F, -0.002556F, -0.002431F, -0.002544F, -0.002564F, -0.002635F, -0.002766F, -0.002845F, -0.003004F, -0.003124F, -0.003219F, -0.003398F, -0.003387F, -0.003629F, -0.003524F, -0.003796F, -0.003636F, -0.003875F, -0.003703F, -0.003846F, -0.003703F, -0.003738F, -0.003654F, -0.003609F, -0.003574F, -0.003494F, -0.003463F, -0.003396F, -0.003333F, -0.003333F, -0.003220F, -0.003303F, -0.003150F, -0.003298F, -0.003152F, -0.003328F, -0.003215F, -0.003357F, -0.003303F, -0.003400F, -0.003420F, -0.003473F, -0.003546F, -0.003570F, -0.003648F, -0.003685F, -0.003728F, -0.003795F, -0.003772F, -0.003878F, -0.003792F, -0.003920F, -0.003801F, -0.003917F, -0.003785F, -0.003862F, -0.003754F, -0.003785F, -0.003719F, -0.003701F, -0.003665F, -0.003619F, -0.003602F, -0.003573F, -0.003555F, -0.003554F, -0.003510F, -0.003551F, -0.003480F, -0.003571F, -0.003484F, -0.003599F, -0.003515F, -0.003626F, -0.003573F, -0.003659F, -0.003649F, -0.003696F, -0.003717F, -0.003732F, -0.003767F, -0.003774F, -0.003799F, -0.003820F, -0.003811F, -0.003856F, -0.003803F, -0.003867F, -0.003781F, -0.003855F, -0.003759F, -0.003822F, -0.003738F, -0.003771F, -0.003712F, -0.003710F, -0.003679F, -0.003652F, -0.003639F, -0.003606F, -0.003592F, -0.003572F, -0.003539F, -0.003546F, -0.003485F, -0.003520F, -0.003437F, -0.003490F, -0.003401F, -0.003451F, -0.003372F, -0.003398F, -0.003335F, -0.003326F, -0.003283F, -0.003245F, -0.003212F, -0.003157F, -0.003115F, -0.003059F, -0.002993F, -0.002949F, -0.002851F, -0.002825F, -0.002702F, -0.002689F, -0.002556F, -0.002542F, -0.002416F, -0.002388F, -0.002283F, -0.002237F, -0.002160F, -0.002101F, -0.002051F, -0.001993F, -0.001957F, -0.001916F, -0.001882F, -0.001871F, -0.001831F, -0.001856F, -0.001812F, -0.001867F, -0.001829F, -0.001901F, -0.001880F, -0.001954F, -0.001959F, -0.002026F, -0.002061F, -0.002119F, -0.002179F, -0.002233F, -0.002303F, -0.002364F, -0.002428F, -0.002502F, -0.002550F, -0.002640F, -0.002668F, -0.002769F, -0.002780F, -0.002877F, -0.002880F, -0.002959F, -0.002962F, -0.003016F, -0.003031F, -0.003063F, -0.003085F, -0.003098F, -0.003116F, -0.003121F, -0.003130F, -0.003142F, -0.003131F, -0.003150F, -0.003116F, -0.003144F, -0.003096F, -0.003129F, -0.003070F, -0.003093F, -0.003036F, -0.003044F, -0.002996F, -0.002977F, -0.002939F, -0.002903F, -0.002871F, -0.002831F, -0.002797F, -0.002759F, -0.002714F, -0.002689F, -0.002628F, -0.002613F, -0.002536F, -0.002525F, -0.002443F, -0.002431F, -0.002356F, -0.002326F, -0.002257F, -0.002210F, -0.002153F, -0.002098F, -0.002051F, -0.001993F, -0.001940F, -0.001884F, -0.001821F, -0.001779F, -0.001707F, -0.001680F, -0.001600F, -0.001577F, -0.001499F, -0.001478F, -0.001405F, -0.001378F, -0.001317F, -0.001285F, -0.001238F, -0.001201F, -0.001164F, -0.001125F, -0.001091F, -0.001061F, -0.001031F, -0.001011F, -0.000976F, -0.000969F, -0.000929F, -0.000942F, -0.000902F, -0.000920F, -0.000885F, -0.000902F, -0.000882F, -0.000895F, -0.000894F, -0.000902F, -0.000912F, -0.000916F, -0.000930F, -0.000939F, -0.000953F, -0.000975F, -0.000983F, -0.001017F, -0.001017F, -0.001060F, -0.001055F, -0.001102F, -0.001101F, -0.001146F, -0.001153F, -0.001189F, -0.001206F, -0.001233F, -0.001258F, -0.001279F, -0.001307F, -0.001328F, -0.001352F, -0.001379F, -0.001393F, -0.001426F, -0.001429F, -0.001467F, -0.001461F, -0.001499F, -0.001489F, -0.001520F, -0.001513F, -0.001533F, -0.001531F, -0.001538F, -0.001541F, -0.001538F, -0.001541F, -0.001536F, -0.001535F, -0.001534F, -0.001525F, -0.001532F, -0.001513F, -0.001527F, -0.001502F, -0.001518F, -0.001493F, -0.001507F, -0.001486F, -0.001494F, -0.001481F, -0.001481F, -0.001476F, -0.001470F, -0.001469F, -0.001462F, -0.001459F, -0.001454F, -0.001444F, -0.001445F, -0.001426F, -0.001433F, -0.001406F, -0.001414F, -0.001384F, -0.001390F, -0.001363F, -0.001361F, -0.001339F, -0.001328F, -0.001310F, -0.001290F, -0.001276F, -0.001256F, -0.001241F, -0.001221F, -0.001199F, -0.001182F, -0.001154F, -0.001144F, -0.001111F, -0.001103F, -0.001069F, -0.001057F, -0.001027F, -0.001012F, -0.000987F, -0.000965F, -0.000942F, -0.000919F, -0.000898F, -0.000874F, -0.000852F, -0.000830F, -0.000804F, -0.000788F, -0.000759F, -0.000745F, -0.000712F, -0.000699F, -0.000668F, -0.000656F, -0.000627F, -0.000612F, -0.000585F, -0.000568F, -0.000546F, -0.000527F, -0.000507F, -0.000486F, -0.000467F, -0.000447F, -0.000428F, -0.000411F, -0.000391F, -0.000377F, -0.000355F, -0.000343F, -0.000321F, -0.000309F, -0.000289F, -0.000277F, -0.000260F, -0.000247F, -0.000231F, -0.000217F, -0.000203F, -0.000190F, -0.000177F, -0.000165F, -0.000152F, -0.000141F, -0.000129F, -0.000119F, -0.000107F, -0.000098F, -0.000088F, -0.000079F, -0.000070F, -0.000062F, -0.000054F, -0.000047F, -0.000040F, -0.000034F, -0.000028F, -0.000023F, -0.000018F, -0.000014F, -0.000010F, -0.000007F, -0.000005F, -0.000003F, -0.000001F, -0.000000F, -0.000000F };

// FF = Far Field
static const float FFEQ_COEFFS_OMNI[512] = { 0.923798F, 0.118459F, 0.084478F, 0.016795F, -0.161455F, -0.073335F, -0.000724F, 0.005185F, 0.021870F, -0.018465F, -0.045974F, -0.020138F, 0.015058F, 0.040742F, 0.029334F, -0.017267F, -0.056054F, -0.055077F, -0.020787F, 0.020071F, 0.032892F, 0.014205F, -0.017589F, -0.033386F, -0.018737F, 0.012689F, 0.034966F, 0.031579F, 0.008172F, -0.016984F, -0.025975F, -0.015404F, 0.002952F, 0.013997F, 0.010859F, -0.000461F, -0.009567F, -0.009338F, -0.002906F, 0.002231F, 0.000415F, -0.006530F, -0.012210F, -0.011394F, -0.004245F, 0.003709F, 0.006857F, 0.003191F, -0.004137F, -0.010197F, -0.011560F, -0.008325F, -0.003426F, 0.000356F, 0.001613F, 0.001306F, 0.000733F, 0.001050F, 0.001936F, 0.002661F, 0.002511F, 0.001487F, 0.000190F, -0.000792F, -0.001031F, -0.000757F, -0.000323F, -0.000202F, -0.000489F, -0.001001F, -0.001489F, -0.001706F, -0.001809F, -0.001891F, -0.002251F, -0.002764F, -0.003320F, -0.003576F, -0.003421F, -0.002887F, -0.002168F, -0.001529F, -0.001053F, -0.000806F, -0.000651F, -0.000539F, -0.000378F, -0.000160F, 0.000078F, 0.000337F, 0.000522F, 0.000674F, 0.000710F, 0.000724F, 0.000666F, 0.000605F, 0.000498F, 0.000355F, 0.000181F, -0.000025F, -0.000208F, -0.000401F, -0.000575F, -0.000779F, -0.000990F, -0.001191F, -0.001350F, -0.001414F, -0.001418F, -0.001343F, -0.001274F, -0.001180F, -0.001115F, -0.001023F, -0.000920F, -0.000778F, -0.000606F, -0.000423F, -0.000224F, -0.000054F, 0.000105F, 0.000219F, 0.000311F, 0.000374F, 0.000414F, 0.000442F, 0.000434F, 0.000420F, 0.000364F, 0.000312F, 0.000230F, 0.000150F, 0.000048F, -0.000060F, -0.000174F, -0.000289F, -0.000384F, -0.000471F, -0.000527F, -0.000574F, -0.000598F, -0.000616F, -0.000623F, -0.000608F, -0.000582F, -0.000511F, -0.000435F, -0.000339F, -0.000261F, -0.000182F, -0.000116F, -0.000043F, 0.000040F, 0.000114F, 0.000181F, 0.000224F, 0.000258F, 0.000276F, 0.000293F, 0.000295F, 0.000287F, 0.000263F, 0.000216F, 0.000178F, 0.000123F, 0.000072F, 0.000010F, -0.000045F, -0.000102F, -0.000145F, -0.000182F, -0.000216F, -0.000250F, -0.000278F, -0.000287F, -0.000283F, -0.000258F, -0.000235F, -0.000213F, -0.000190F, -0.000165F, -0.000121F, -0.000073F, -0.000015F, 0.000026F, 0.000065F, 0.000093F, 0.000130F, 0.000168F, 0.000203F, 0.000228F, 0.000232F, 0.000230F, 0.000222F, 0.000223F, 0.000222F, 0.000220F, 0.000207F, 0.000182F, 0.000157F, 0.000127F, 0.000107F, 0.000083F, 0.000068F, 0.000045F, 0.000027F, 0.000008F, -0.000007F, -0.000017F, -0.000026F, -0.000029F, -0.000036F, -0.000037F, -0.000042F, -0.000042F, -0.000040F, -0.000039F, -0.000036F, -0.000040F, -0.000039F, -0.000047F, -0.000050F, -0.000062F, -0.000077F, -0.000100F, -0.000127F, -0.000156F, -0.000189F, -0.000222F, -0.000263F, -0.000304F, -0.000352F, -0.000397F, -0.000444F, -0.000491F, -0.000537F, -0.000586F, -0.000629F, -0.000674F, -0.000708F, -0.000744F, -0.000773F, -0.000801F, -0.000823F, -0.000841F, -0.000854F, -0.000860F, -0.000865F, -0.000862F, -0.000858F, -0.000848F, -0.000835F, -0.000817F, -0.000792F, -0.000764F, -0.000728F, -0.000693F, -0.000652F, -0.000613F, -0.000567F, -0.000521F, -0.000470F, -0.000419F, -0.000370F, -0.000319F, -0.000271F, -0.000220F, -0.000170F, -0.000119F, -0.000072F, -0.000028F, 0.000014F, 0.000053F, 0.000092F, 0.000124F, 0.000152F, 0.000170F, 0.000188F, 0.000200F, 0.000213F, 0.000224F, 0.000232F, 0.000238F, 0.000241F, 0.000246F, 0.000248F, 0.000250F, 0.000246F, 0.000237F, 0.000229F, 0.000220F, 0.000214F, 0.000206F, 0.000195F, 0.000175F, 0.000159F, 0.000140F, 0.000126F, 0.000107F, 0.000085F, 0.000058F, 0.000029F, 0.000004F, -0.000021F, -0.000043F, -0.000069F, -0.000096F, -0.000125F, -0.000157F, -0.000185F, -0.000214F, -0.000239F, -0.000266F, -0.000291F, -0.000322F, -0.000352F, -0.000384F, -0.000415F, -0.000442F, -0.000467F, -0.000492F, -0.000520F, -0.000550F, -0.000583F, -0.000612F, -0.000639F, -0.000665F, -0.000689F, -0.000718F, -0.000742F, -0.000765F, -0.000785F, -0.000803F, -0.000817F, -0.000833F, -0.000847F, -0.000860F, -0.000871F, -0.000881F, -0.000889F, -0.000894F, -0.000899F, -0.000901F, -0.000904F, -0.000908F, -0.000907F, -0.000904F, -0.000896F, -0.000887F, -0.000875F, -0.000866F, -0.000856F, -0.000844F, -0.000827F, -0.000806F, -0.000785F, -0.000763F, -0.000747F, -0.000730F, -0.000713F, -0.000691F, -0.000666F, -0.000639F, -0.000614F, -0.000593F, -0.000571F, -0.000551F, -0.000526F, -0.000501F, -0.000476F, -0.000454F, -0.000432F, -0.000412F, -0.000391F, -0.000368F, -0.000347F, -0.000326F, -0.000309F, -0.000292F, -0.000277F, -0.000261F, -0.000245F, -0.000231F, -0.000219F, -0.000211F, -0.000203F, -0.000198F, -0.000192F, -0.000188F, -0.000185F, -0.000184F, -0.000185F, -0.000188F, -0.000192F, -0.000195F, -0.000199F, -0.000201F, -0.000204F, -0.000207F, -0.000208F, -0.000211F, -0.000212F, -0.000216F, -0.000218F, -0.000222F, -0.000223F, -0.000225F, -0.000227F, -0.000229F, -0.000231F, -0.000233F, -0.000234F, -0.000234F, -0.000237F, -0.000239F, -0.000243F, -0.000245F, -0.000247F, -0.000249F, -0.000250F, -0.000255F, -0.000258F, -0.000263F, -0.000264F, -0.000266F, -0.000268F, -0.000272F, -0.000278F, -0.000284F, -0.000287F, -0.000286F, -0.000287F, -0.000288F, -0.000291F, -0.000295F, -0.000297F, -0.000295F, -0.000293F, -0.000293F, -0.000291F, -0.000293F, -0.000291F, -0.000286F, -0.000282F, -0.000278F, -0.000277F, -0.000274F, -0.000271F, -0.000267F, -0.000261F, -0.000256F, -0.000251F, -0.000247F, -0.000241F, -0.000236F, -0.000231F, -0.000225F, -0.000219F, -0.000214F, -0.000206F, -0.000200F, -0.000194F, -0.000188F, -0.000182F, -0.000175F, -0.000167F, -0.000159F, -0.000153F, -0.000147F, -0.000142F, -0.000135F, -0.000128F, -0.000120F, -0.000113F, -0.000107F, -0.000102F, -0.000096F, -0.000090F, -0.000083F, -0.000077F, -0.000071F, -0.000067F, -0.000063F, -0.000058F, -0.000053F, -0.000048F, -0.000044F, -0.000040F, -0.000037F, -0.000034F, -0.000030F, -0.000027F, -0.000024F, -0.000021F, -0.000019F, -0.000017F, -0.000015F, -0.000013F, -0.000011F, -0.000009F, -0.000008F, -0.000006F, -0.000005F, -0.000004F, -0.000003F, -0.000002F, -0.000002F, -0.000001F, -0.000001F, -0.000000F, -0.000000F, -0.000000F };

static const float FFEQ_COEFFS_EIGHT[512] = { 0.973505F, -0.015743F, -0.119884F, 0.017401F, 0.193473F, -0.003962F, -0.089810F, 0.020386F, 0.083724F, 0.034610F, -0.023902F, -0.022050F, -0.008386F, -0.006994F, 0.006076F, 0.028268F, 0.032291F, 0.010319F, -0.006591F, -0.004401F, 0.009154F, 0.016144F, 0.015145F, 0.008107F, -0.002863F, -0.012199F, -0.016168F, -0.013329F, -0.006975F, 0.000669F, 0.005459F, 0.005197F, 0.001403F, -0.001754F, -0.001986F, -0.000479F, 0.001837F, 0.003630F, 0.005002F, 0.005146F, 0.004339F, 0.002790F, 0.001295F, 0.000556F, 0.000532F, 0.001364F, 0.002503F, 0.003788F, 0.004586F, 0.004866F, 0.004548F, 0.003746F, 0.002750F, 0.001657F, 0.000829F, 0.000144F, -0.000249F, -0.000665F, -0.000981F, -0.001248F, -0.001332F, -0.001228F, -0.001043F, -0.000750F, -0.000525F, -0.000285F, -0.000164F, -0.000040F, 0.000062F, 0.000179F, 0.000337F, 0.000471F, 0.000670F, 0.000825F, 0.001065F, 0.001271F, 0.001516F, 0.001686F, 0.001790F, 0.001790F, 0.001673F, 0.001508F, 0.001270F, 0.001063F, 0.000829F, 0.000630F, 0.000412F, 0.000187F, -0.000042F, -0.000295F, -0.000499F, -0.000702F, -0.000819F, -0.000925F, -0.000956F, -0.000980F, -0.000969F, -0.000943F, -0.000900F, -0.000818F, -0.000731F, -0.000601F, -0.000484F, -0.000348F, -0.000236F, -0.000133F, -0.000047F, 0.000011F, 0.000069F, 0.000089F, 0.000123F, 0.000121F, 0.000132F, 0.000110F, 0.000084F, 0.000033F, -0.000039F, -0.000125F, -0.000240F, -0.000347F, -0.000472F, -0.000577F, -0.000685F, -0.000778F, -0.000863F, -0.000945F, -0.001004F, -0.001068F, -0.001098F, -0.001127F, -0.001119F, -0.001108F, -0.001075F, -0.001041F, -0.001005F, -0.000961F, -0.000924F, -0.000873F, -0.000834F, -0.000787F, -0.000753F, -0.000720F, -0.000702F, -0.000693F, -0.000680F, -0.000699F, -0.000718F, -0.000749F, -0.000767F, -0.000806F, -0.000841F, -0.000886F, -0.000940F, -0.000988F, -0.001039F, -0.001086F, -0.001146F, -0.001184F, -0.001220F, -0.001251F, -0.001285F, -0.001305F, -0.001306F, -0.001319F, -0.001317F, -0.001308F, -0.001282F, -0.001267F, -0.001241F, -0.001218F, -0.001193F, -0.001162F, -0.001130F, -0.001105F, -0.001094F, -0.001077F, -0.001067F, -0.001054F, -0.001053F, -0.001056F, -0.001060F, -0.001074F, -0.001088F, -0.001113F, -0.001130F, -0.001159F, -0.001187F, -0.001223F, -0.001252F, -0.001281F, -0.001312F, -0.001336F, -0.001365F, -0.001388F, -0.001415F, -0.001433F, -0.001451F, -0.001463F, -0.001469F, -0.001474F, -0.001472F, -0.001475F, -0.001467F, -0.001465F, -0.001452F, -0.001443F, -0.001428F, -0.001413F, -0.001399F, -0.001381F, -0.001368F, -0.001350F, -0.001337F, -0.001320F, -0.001306F, -0.001291F, -0.001275F, -0.001262F, -0.001246F, -0.001236F, -0.001219F, -0.001209F, -0.001190F, -0.001174F, -0.001153F, -0.001132F, -0.001109F, -0.001082F, -0.001056F, -0.001022F, -0.000990F, -0.000952F, -0.000914F, -0.000871F, -0.000827F, -0.000784F, -0.000736F, -0.000691F, -0.000640F, -0.000596F, -0.000545F, -0.000501F, -0.000454F, -0.000412F, -0.000370F, -0.000331F, -0.000297F, -0.000264F, -0.000238F, -0.000214F, -0.000196F, -0.000181F, -0.000172F, -0.000169F, -0.000169F, -0.000177F, -0.000186F, -0.000204F, -0.000222F, -0.000249F, -0.000277F, -0.000311F, -0.000348F, -0.000388F, -0.000433F, -0.000478F, -0.000528F, -0.000576F, -0.000627F, -0.000676F, -0.000726F, -0.000776F, -0.000823F, -0.000871F, -0.000913F, -0.000955F, -0.000989F, -0.001024F, -0.001052F, -0.001081F, -0.001107F, -0.001130F, -0.001149F, -0.001163F, -0.001177F, -0.001187F, -0.001198F, -0.001205F, -0.001210F, -0.001213F, -0.001210F, -0.001208F, -0.001206F, -0.001203F, -0.001189F, -0.001181F, -0.001168F, -0.001154F, -0.001135F, -0.001116F, -0.001094F, -0.001071F, -0.001052F, -0.001026F, -0.001000F, -0.000972F, -0.000950F, -0.000923F, -0.000892F, -0.000863F, -0.000833F, -0.000803F, -0.000771F, -0.000742F, -0.000707F, -0.000670F, -0.000631F, -0.000592F, -0.000555F, -0.000519F, -0.000483F, -0.000441F, -0.000402F, -0.000361F, -0.000320F, -0.000279F, -0.000241F, -0.000205F, -0.000162F, -0.000122F, -0.000084F, -0.000047F, -0.000006F, 0.000031F, 0.000068F, 0.000103F, 0.000136F, 0.000169F, 0.000204F, 0.000237F, 0.000263F, 0.000290F, 0.000315F, 0.000342F, 0.000366F, 0.000384F, 0.000401F, 0.000417F, 0.000432F, 0.000442F, 0.000453F, 0.000461F, 0.000466F, 0.000464F, 0.000463F, 0.000462F, 0.000457F, 0.000453F, 0.000446F, 0.000437F, 0.000424F, 0.000412F, 0.000397F, 0.000380F, 0.000365F, 0.000347F, 0.000331F, 0.000310F, 0.000290F, 0.000268F, 0.000246F, 0.000224F, 0.000202F, 0.000179F, 0.000155F, 0.000134F, 0.000110F, 0.000088F, 0.000064F, 0.000043F, 0.000022F, 0.000001F, -0.000018F, -0.000036F, -0.000052F, -0.000068F, -0.000081F, -0.000094F, -0.000105F, -0.000115F, -0.000122F, -0.000129F, -0.000134F, -0.000137F, -0.000141F, -0.000142F, -0.000145F, -0.000145F, -0.000147F, -0.000148F, -0.000147F, -0.000148F, -0.000147F, -0.000147F, -0.000144F, -0.000145F, -0.000143F, -0.000143F, -0.000141F, -0.000140F, -0.000139F, -0.000138F, -0.000137F, -0.000134F, -0.000133F, -0.000131F, -0.000129F, -0.000127F, -0.000124F, -0.000121F, -0.000117F, -0.000114F, -0.000109F, -0.000106F, -0.000102F, -0.000098F, -0.000093F, -0.000087F, -0.000080F, -0.000073F, -0.000069F, -0.000064F, -0.000059F, -0.000053F, -0.000046F, -0.000040F, -0.000034F, -0.000029F, -0.000024F, -0.000021F, -0.000014F, -0.000009F, -0.000005F, -0.000003F, 0.000002F, 0.000007F, 0.000010F, 0.000013F, 0.000016F, 0.000019F, 0.000022F, 0.000025F, 0.000026F, 0.000026F, 0.000029F, 0.000032F, 0.000032F, 0.000032F, 0.000034F, 0.000034F, 0.000034F, 0.000034F, 0.000035F, 0.000034F, 0.000033F, 0.000032F, 0.000031F, 0.000031F, 0.000031F, 0.000030F, 0.000028F, 0.000027F, 0.000025F, 0.000024F, 0.000022F, 0.000021F, 0.000019F, 0.000018F, 0.000016F, 0.000014F, 0.000013F, 0.000011F, 0.000010F, 0.000009F, 0.000008F, 0.000007F, 0.000005F, 0.000004F, 0.000004F, 0.000003F, 0.000002F, 0.000001F, 0.000001F, 0.000000F, -0.000000F, -0.000000F, -0.000000F, -0.000001F, -0.000001F, -0.000001F, -0.000001F, -0.000001F, -0.000001F, -0.000001F, -0.000000F, -0.000000F, -0.000000F, -0.000000F, -0.000000F, -0.000000F, -0.000000F, -0.000000F };
