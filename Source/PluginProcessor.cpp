#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#define M_PI 3.14159 26536 //dovrebbe essere già incluso in <math.h>

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
{}

StereoFlangerAudioProcessor::~StereoFlangerAudioProcessor()
{
}

//==============================================================================
const juce::String StereoFlangerAudioProcessor::getName() const{
    return JucePlugin_Name;
}

bool StereoFlangerAudioProcessor::acceptsMidi() const{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StereoFlangerAudioProcessor::producesMidi() const{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StereoFlangerAudioProcessor::isMidiEffect() const{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StereoFlangerAudioProcessor::getTailLengthSeconds() const{
    return 0.0;
}

int StereoFlangerAudioProcessor::getNumPrograms(){
    return 1;
}

int StereoFlangerAudioProcessor::getCurrentProgram(){
    return 0;
}

void StereoFlangerAudioProcessor::setCurrentProgram (int index){
}

const juce::String StereoFlangerAudioProcessor::getProgramName (int index){
    return {};
}

void StereoFlangerAudioProcessor::changeProgramName (int index, const juce::String& newName){
}

//==============================================================================
void StereoFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    f_s = sampleRate;
    dbuf.setSize(getTotalNumOutputChannels(), 100000); // La dimensione del delay buffer per un flanger è pochi ms
    dbuf.clear(); // dbuf credo sia il buffer che contiene i delayed samples

    dw = 1;
    dr = 0;
    //dr = (dw - delayTime + ds ) % ds;
    ds = dbuf.getNumSamples();
    feedback = 0.5;
    freq = 50;
    phase = 0.0;
}

void StereoFlangerAudioProcessor::releaseResources(){
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

    // Pointers to buffer
    //auto* channelOutL = buffer.getWritePointer(0); // write pointer
    //auto* channelOutR = buffer.getWritePointer(1);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        int numSamples = buffer.getNumSamples();
        float dryMix_ = dry;
        float wetMix_ = 1 - dryMix_;
        float feedback_ = feedback;
        float freq_ = freq;
        float phase_ = phase;
        int delayTime_ = delayTime;
        float sweep_ = sweep * f_s * delayTime_;

        // Pointers to buffer
        auto* channelOut = buffer.getWritePointer(channel); // write pointer
        const auto* channelIn = buffer.getReadPointer(channel); // read pointer

        for (int i=0; i<numSamples; ++i) {

            // Delay line
            dbuf.setSample(channel, dw, channelIn[i]);
            //dbuf.setSample(1, dw, channelIn[i]);

            // Linear interpolation
            float interpolatedSample = 0.0;
            float fraction = dr - floorf(dr);
            int previousSample = (int)floorf(dr);
            int nextSample = (previousSample + 1) % ds;
            interpolatedSample = fraction * dbuf.getSample(channel, nextSample) + (1.0f - fraction) * dbuf.getSample(channel,previousSample);

            // Feedback
            dbuf.setSample(channel, dw, channelIn[i] + interpolatedSample * feedback_);
            //dbuf.setSample(0, dw, channelIn[i] + interpolatedSampleR * feedback_);

            // Mix wet signal and dry signal
            channelOut[i] = dryMix_ * channelIn[i] + wetMix_ * interpolatedSample;
            //channelOutR[i] = dryMix_ * channelIn[i] + wetMix_ * interpolatedSampleR;

            // Circular buffer
            float currentDelay;
            if(channel==1){
                currentDelay = sweep_ * ( 0.5f + 0.5f * sinf(2.0f * 3.1416 * freq_ * i + phase_)) + delayTime_;
            }
            else if (channel == 0){
                currentDelay = sweep_ * ( 0.5f + 0.5f * sinf(2.0f * 3.1416 * freq_ * i)) + delayTime_;
            }

            dw = (dw + 1 ) % ds;
            dr = fmodf((float)dw - (float)(currentDelay) + (float)ds - 3.0, (float)ds);
        }
    }
}

//==============================================================================
bool StereoFlangerAudioProcessor::hasEditor() const{
    return true; // (change this to false if you choose to not supply an editor)
}
juce::AudioProcessorEditor* StereoFlangerAudioProcessor::createEditor(){
    return new StereoFlangerAudioProcessorEditor (*this);
}
void StereoFlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData){
}
void StereoFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes){
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){
    return new StereoFlangerAudioProcessor();
}

// The functions that will be used by the Editor to update slider values
//==========================================
void StereoFlangerAudioProcessor::set_sweep(float val){
    sweep = val;
}
void StereoFlangerAudioProcessor::set_dry_wet(float val){
    dry = val;
}
void StereoFlangerAudioProcessor::set_delayTime(int val){
    delayTime = val;
}
void StereoFlangerAudioProcessor::set_feedback(float val){
    feedback = val;
}
void StereoFlangerAudioProcessor::set_freq(float val){
    freq = val;
}
void StereoFlangerAudioProcessor::set_phase(float val){
    phase = val;
}
//==========================================
