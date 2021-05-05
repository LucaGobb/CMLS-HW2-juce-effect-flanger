/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MyPlugInAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      private juce::Slider::Listener
{
public:
    MyPlugInAudioProcessorEditor (MyPlugInAudioProcessor&);
    ~MyPlugInAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged(juce::Slider* slider) override;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MyPlugInAudioProcessor& audioProcessor;


    juce::Slider freqDepth;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthSliderAttachment;
    juce::Slider rate;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateSliderAttachment;
    juce::Slider delay;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delaySliderAttachment;
    juce::Slider phaseBalance;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseSliderAttachment;
    juce::Slider dryWet;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetSliderAttachment;
    juce::Slider regenGain;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> regenGainSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPlugInAudioProcessorEditor)
};