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

    dbuf.setSize(getTotalNumOutputChannels(), 100000); // todo: set the buffer dimension properly
    dbuf.clear();

    dw = 0; //write pointer
    // dr = 1; //read pointer
    ds = 100000; //dealy value // todo: just make it equal to the buffer size
    // fs = 44100;
    fs = sampleRate; // todo: correct?

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

// todo: do we need midibuffer as input? or it is as default?
// void StereoFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
void StereoFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
      auto* channelData = buffer.getWritePointer (channel);

      int numSamples= buffer.getNumSamples();



      float dry_now = dry;
      // float wet_now = wet;
      float wet_now = 1 - dry_now;
      float fb_now = feedback;
      int ds_now = ds;
      // width_now is the sweep in sample
      float width_now = wet * fs * 0.01; // todo: rename
                                         // delaytimeMax : 10ms as pdf
      float freq_now = freq;


      float* channelOutDataL = buffer.getWritePointer(0);
      float* channelOutDataR = buffer.getWritePointer(1);
      const float* channelInDataL = buffer.getReadPointer(0);
      const float* channelInDataR = buffer.getReadPointer(1);

      dbuf.setSample(0, dw, channelInDataL[0]); // todo: cosa fa? copio il primo sample nel buffer di uscita?
      dbuf.setSample(1, dw, channelInDataR[0]);

      // Delay line
      for (int i=0; i<numSamples; ++i) {

          float interpolatedSample = 0.0;
          float currentDelay = width_now * ( 0.5f + 0.5f * sinf(2.0 * M_PI * phase) );
          // todo: add the minimum delay

          // delay in sample
          // todo: capire se width e quindi current delay li vogliamo normalizzati a 1 o misurati giá in samples
          // todo: a cosa serve sto delay di 3 sample
          dr = fmodf((float)dw - (float)(currentDelay /* * fs*/) + (float)ds - 3.0, (float)ds);

          // Linear interpolation
          float fraction = dr - floorf(dr);
          int previousSample = (int)floorf(dr);
          int nextSample = (previousSample + 1) % ds;
          interpolatedSample = fraction * dbuf.getSample(0, nextSample) + (1.0f - fraction) * dbuf.getSample(0,previousSample);
          // todo: do it also for the right channel


          // Feedback
          // delayData[dpw] = in + (interpolatedSample * feedback_);
          dbuf.setSample(0, dw, channelInDataL[i] + interpolatedSample * fb_now);
          dbuf.setSample(1, dw, channelInDataR[i] + interpolatedSample * fb_now); //todo: stereo, change interpolatedSampleRIGHT

          channelOutDataL[i] = dry_now * channelInDataL[i] + wet_now* interpolatedSample;
          channelOutDataR[i] = dry_now * channelInDataR[i] + wet_now* interpolatedSample; // todo: stereoify

          dw = (dw + 1 ) % ds_now;
          // dr = (dr + 1 ) % ds_now;

          //dbuf.setSample(0,dr,channelInDataL[i]) = interpolatedSample;

          phase += freq_now / fs;
          if(phase >= 1.0)
              phase -= 1.0;
      }

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
