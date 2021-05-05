#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#define M_PI 3.1416

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

    dbuf.setSize(getTotalNumOutputChannels(), 100000);
    dbuf.clear();

    dw = 0; //write pointer
    dr = 1; //read pointer
    ds = 50000; //dealy value
    fs = 44100;

    feedback = 0.5; //feedback gain

    phase = 0.0;

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


    float wet_now = wet;
    float dry_now = dry;
    float fb_now = feedback;
    int ds_now = ds;


    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    const float* channelInDataL = buffer.getReadPointer(0);
    const float* channelInDataR = buffer.getReadPointer(1);

    dbuf.setSample(0, dw, channelInDataL[0]);
    dbuf.setSample(1, dw, channelInDataR[0]);

    // Delay line

    for (int i=0; i<numSamples; ++i) {

        channelOutDataL[i] = dry_now * channelInDataL[i] + wet_now* dbuf.getSample(0, dr);
        channelOutDataR[i] = dry_now * channelInDataR[i] + wet_now* dbuf.getSample(1, dr);

        float interpolatedSample = 0.0;
        float currentDelay = width * 0.5f + 0.5f * sinf(2.0 * M_PI * phase);

        // Linear interpolation

        dr = fmodf((float)dw - (float)(currentDelay * fs) + (float)ds - 3.0, (float)ds);

        float fraction = dr = floorf(dr);

        int previousSample = (int)floorf(dr);
        int nextSample = (previousSample + 1) % ds;
        interpolatedSample = fraction * dbuf.getSample(0, nextSample) + (1.0f - fraction) * dbuf.getSample(0,previousSample);

        // Feedback

        dbuf.setSample(0, dw, channelInDataL[i] + dbuf.getSample(0, dr) * fb_now);
        dbuf.setSample(1, dw, channelInDataR[i] + dbuf.getSample(1, dr) * fb_now);

        dw = (dw + 1 ) % ds_now;
        dr = (dr + 1 ) % ds_now;

        //dbuf.setSample(0,dr,channelInDataL[i]) = interpolatedSample;

        phase += freq * T;
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


void StereoFlangerAudioProcessor::set_wet(float val)
{
    wet = val;
}


void StereoFlangerAudioProcessor::set_dry(float val)
{
    dry = val;
}


void StereoFlangerAudioProcessor::set_ds(int val)
{
    ds = val;
}


void StereoFlangerAudioProcessor::set_fb(float val)
{
    feedback = val;
}


void StereoFlangerAudioProcessor::set_freq(float val)
{
    freq = val;
}

void StereoFlangerAudioProcessor::set_width(float val)
{
    width = val;
}
