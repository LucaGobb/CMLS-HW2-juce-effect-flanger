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
class StereoFlangerAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Slider::Listener
{
public:
    StereoFlangerAudioProcessorEditor (StereoFlangerAudioProcessor&);
    ~StereoFlangerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StereoFlangerAudioProcessor& audioProcessor;

    juce::Slider wetSlider;
    juce::Label wetLabel;

    juce::Slider drySlider;
    juce::Label dryLabel;

    juce::Slider timeSlider;
    juce::Label timeLabel;

    juce::Slider freqSlider;
    juce::Label freqLabel;

    juce::Slider widthSlider;
    juce::Label widthLabel;

    juce::Slider feedbackSlider;
    juce::Label feedbackLabel;

    void sliderValueChanged(juce::Slider* slider) override;
    //********************************************************************************************//

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoFlangerAudioProcessorEditor)
};
