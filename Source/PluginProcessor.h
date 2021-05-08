/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class StereoFlangerAudioProcessor  : public juce::AudioProcessor

{

    public:

    StereoFlangerAudioProcessor();
    ~StereoFlangerAudioProcessor() override;


    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void set_sweep(float val);
    void set_depth(float val);
    void set_feedback(float val);
    void set_delayTime(float val);
    void set_freq(float val);
    void set_phase(float val);

    private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoFlangerAudioProcessor)

    // Delay Buffer
    //============================
    juce::AudioSampleBuffer dbuf;
    float drL; // reading index
    float drR; // reading index
    int dw { 0 }; // writing index
    int delayBufferLength; // buffer length in samples
    //============================

    float fs; // Sampling Rate
    float depth { 0.0f };
    float feedback { 0.0f }; // Feedback gain

    float freq { 0.0f };
    float sweep { 0.0f }; // Sweep Width (i.e. LFO amplitude)

    float phase { 0.0f }; // istantaneous phase of the LFO
    float phaseRL { 0.0f }; // Phase offset between L&R channels
    float delayTime { 0.0f }; // Minimum Delay Time
};
