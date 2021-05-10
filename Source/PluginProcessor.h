/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


class PresetListBox;

//==============================================================================
/**
*/
class StereoFlangerAudioProcessor  : public foleys::MagicProcessor
{
public:
    //==============================================================================
    StereoFlangerAudioProcessor();



    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;


#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    void savePresetInternal();
    void loadPresetInternal(int index);

    //==============================================================================
    double getTailLengthSeconds() const override;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState treeState;
    std::atomic<float>* phaseRL = nullptr;
    std::atomic<float>* depth  = nullptr;
    std::atomic<float>* freq = nullptr;
    std::atomic<float>* delayTime  = nullptr;
    std::atomic<float>* feedback = nullptr;
    std::atomic<float>* sweep  = nullptr;
    std::atomic<float>* phaseSwitch = nullptr;
    std::atomic<float>* lfoWaveType = nullptr;


    juce::ValueTree  presetNode;

    // GUI MAGIC: define that as last member of your AudioProcessor
    foleys::MagicLevelSource*   outputMeter  = nullptr;
    foleys::MagicPlotSource*    oscilloscope = nullptr;
    foleys::MagicPlotSource*    analyser     = nullptr;

    PresetListBox*              presetList   = nullptr;

    // Delay Buffer
    //============================
    juce::AudioSampleBuffer dbuf;
    float drL; // reading index
    float drR; // reading index
    int dw{ 0 }; // writing index
    int delayBufferLength; // buffer length in samples
    //============================

    float samplingPeriod;
    float samplingFrequency;


    //LFO
    //============================

    float lfo(float phase, int waveform);


    float maxSweepWidth{ 10.0f };

    float phase{ 0.0f }; // istantaneous phase of the LFO

    float maxDelayTime{ 10.0f };
    int waveform{ 1 };
    //============================



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoFlangerAudioProcessor)
    int TRANS;
};