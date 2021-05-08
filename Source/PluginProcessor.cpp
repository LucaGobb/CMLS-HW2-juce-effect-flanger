#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#define M_PI 3.14159 26536 //dovrebbe essere già incluso in <math.h>

//==============================================================================
StereoFlangerAudioProcessor::StereoFlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    sweep = 0.0f;
    phase = 0.0f;
    freq = 0.0f;
    dry = 0.0f;
    feedback = 0.0f;
    phaseRL = 0.0f;
    delayTime = 0.0f;
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

void StereoFlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StereoFlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void StereoFlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StereoFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //buffer inizialization:
    //   (delay minimum + sweep) in samples + 1 sample for interpolation
    ds = (int)((5 + 10) *0.001f * sampleRate) + 1; // todo: do more parametric
    dbuf.setSize(getTotalNumOutputChannels(), ds); 
    dbuf.clear();

    dw = 0; //write pointer
    fs = sampleRate;
}

void StereoFlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StereoFlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

void StereoFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
      auto* channelData = buffer.getWritePointer (channel);
    }

    int numSamples= buffer.getNumSamples();

    // freeze variables
    float dry_now = dry; // values: from 0 to 1
    float fb_now = feedback; // 0 to 0.9
    float sweep_sample = sweep * getSampleRate() * 0.01f; // sweep: 0 to 1
                                 // delaytimeMax : 10ms as pdf
    float freq_now = freq;
    float delay_min_sample = delayTime * 0.001f * getSampleRate(); // 0 to 5 [ms]


    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    const float* channelInDataL = buffer.getReadPointer(0);
    const float* channelInDataR = buffer.getReadPointer(1);

    // Delay line
    for (int i=0; i<numSamples; ++i) {

        float interpolatedSample = 0.0;
        float currentDelay = delay_min_sample + sweep_sample * ( 0.5f + 0.5f * sinf(2.0f * 3.14f * phase) );
        // Triangle
        // float currentDelay = delay_min_sample + sweep_sample * ( 0.5f + 0.5f * phase );

        // delay in sample
        dr = fmodf((float)dw - (float)(currentDelay) + (float)ds - 1.0f, (float)ds);

        // Linear interpolation
        float fraction = dr - floorf(dr);
        int previousSample = (int)floorf(dr);
        int nextSample = (previousSample + 1) % ds;
        interpolatedSample = fraction * dbuf.getSample(0, nextSample) + (1.0f - fraction) * dbuf.getSample(0,previousSample);
        // todo: do it also for the right channel


        // Feedback
        dbuf.setSample(0, dw, channelInDataL[i] + interpolatedSample * fb_now);
        dbuf.setSample(1, dw, channelInDataR[i] + interpolatedSample * fb_now);

        channelOutDataL[i] = channelInDataL[i] + (1 - dry_now)* interpolatedSample;
        channelOutDataR[i] = channelInDataR[i] + (1 - dry_now)* interpolatedSample;

        dw = (dw + 1 ) % ds;

        phase += freq_now / fs;
        if(phase >= 1.0)
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
    return new StereoFlangerAudioProcessorEditor (*this);
}


//==============================================================================
void StereoFlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void StereoFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

void StereoFlangerAudioProcessor::set_dry_wet(float val)
{
    dry = val;
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
    //phase = val;
}
//==========================================
