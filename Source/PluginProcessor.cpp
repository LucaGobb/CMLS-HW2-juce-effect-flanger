#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#define M_PI 3.14159 26536 //dovrebbe essere gi� incluso in <math.h>

//==============================================================================
StereoFlangerAudioProcessor::StereoFlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{

}

StereoFlangerAudioProcessor::~StereoFlangerAudioProcessor()
{
}

//==============================================================================
const juce::String StereoFlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StereoFlangerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool StereoFlangerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool StereoFlangerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double StereoFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StereoFlangerAudioProcessor::getNumPrograms()
{
    return 1;
}

int StereoFlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoFlangerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String StereoFlangerAudioProcessor::getProgramName(int index)
{
    return {};
}

void StereoFlangerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void StereoFlangerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    //buffer inizialization:
    //   (delay minimum + sweep) in samples + 1 sample for interpolation

    delayBufferLength = (int)((maxDelayTime + maxSweepWidth) * 0.001f * sampleRate) + samplesPerBlock; // todo: do more parametric
    dbuf.setSize(getTotalNumOutputChannels(), delayBufferLength);
    dbuf.clear();

    samplingFrequency = sampleRate;
    //samplingPeriod = 1 / sampleRate;
}

void StereoFlangerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StereoFlangerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void StereoFlangerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
    }

    int numSamples = buffer.getNumSamples();

    // freeze variables
    float depth_ = depth; // values: from 0 to 1
    float fb_now = feedback; // 0 to 0.9
    float sweep_sample = sweep * getSampleRate() * 0.01f; // sweep: 0 to 1
    int waveform_ = waveform;

    float freq_now = freq;
    float delay_min_sample = delayTime * 0.001f * getSampleRate(); // 0 to 5 [ms]
    float phaseRL_now = phaseRL;


    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    const float* channelInDataL = buffer.getReadPointer(0);
    const float* channelInDataR = buffer.getReadPointer(1);

    // Delay line
    for (int i = 0; i < numSamples; ++i) {

        float interpolatedSampleL = 0.0;
        float interpolatedSampleR = 0.0;

        float currentDelayL = delay_min_sample + sweep_sample * lfo(phase,waveform_);
        float currentDelayR = delay_min_sample + sweep_sample * lfo(phase + phaseRL_now,waveform_);

        // delay in sample
        drL = fmodf((float)dw - (float)(currentDelayL)+(float)delayBufferLength - 1.0f, (float)delayBufferLength);
        drR = fmodf((float)dw - (float)(currentDelayR)+(float)delayBufferLength - 1.0f, (float)delayBufferLength);

        // Linear interpolation
        float fractionL = drL - floorf(drL);
        float fractionR = drR - floorf(drR);
        int previousSampleL = (int)floorf(drL);
        int previousSampleR = (int)floorf(drR);
        int nextSampleL = (previousSampleL + 1) % delayBufferLength;
        int nextSampleR = (previousSampleR + 1) % delayBufferLength;
        interpolatedSampleL = fractionL * dbuf.getSample(0, nextSampleL) + (1.0f - fractionL) * dbuf.getSample(0, previousSampleL);
        interpolatedSampleR = fractionR * dbuf.getSample(1, nextSampleR) + (1.0f - fractionR) * dbuf.getSample(1, previousSampleR);


        // Feedback
        dbuf.setSample(0, dw, channelInDataL[i] + interpolatedSampleL * fb_now);
        dbuf.setSample(1, dw, channelInDataR[i] + interpolatedSampleR * fb_now);

        channelOutDataL[i] = channelInDataL[i] + depth_ * interpolatedSampleL;
        channelOutDataR[i] = channelInDataR[i] + depth_ * interpolatedSampleR;

        dw = (dw + 1) % delayBufferLength;

        phase += freq_now / samplingFrequency;
        if (phase >= 1.0)
            phase -= 1.0;
    }

}

//==============================================================================
bool StereoFlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}


juce::AudioProcessorEditor* StereoFlangerAudioProcessor::createEditor()
{
    return new StereoFlangerAudioProcessorEditor(*this);
}


//==============================================================================
void StereoFlangerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void StereoFlangerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoFlangerAudioProcessor();
}

//The functions that will be used by the Editor to update slider values
//==========================================
void StereoFlangerAudioProcessor::set_sweep(float val)
{
    sweep = val;
}

void StereoFlangerAudioProcessor::set_depth(float val)
{
    depth = val;
}

void StereoFlangerAudioProcessor::set_delayTime(float val)
{
    delayTime = val;
}

void StereoFlangerAudioProcessor::set_feedback(float val)
{
    feedback = val;
}

void StereoFlangerAudioProcessor::set_freq(float val)
{
    freq = val;
}

void StereoFlangerAudioProcessor::set_phase(float val)
{
    phaseRL = val;
}

void StereoFlangerAudioProcessor::set_wave(int val)
{
    waveform = val;
}

float StereoFlangerAudioProcessor::lfo(float phase, int waveform)
{
    switch (waveform)
    {
    case 1: // Triangle
        if (phase < 0.25f)
            return 0.5f + 2.0f * phase;
        else if (phase < 0.75f)
            return 1.0f - 2.0f * (phase - 0.25f);
        else
            return 2.0f * (phase - 0.75f);
    case 2: // Square
        if (phase < 0.5f)
            return 1.0f;
        else
            return 0.0f;
    case 3: // Sawtooth
        if (phase < 0.5f)
            return 0.5f + phase;
        else
            return phase - 0.5f;
    case 4: // Sin
    default:
        return 0.5f + 0.5f * sinf(2.0 * 3.1416 * phase);
    }
}


//==========================================
