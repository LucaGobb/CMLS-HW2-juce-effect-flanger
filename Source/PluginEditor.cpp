depth/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoFlangerAudioProcessorEditor::StereoFlangerAudioProcessorEditor (StereoFlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // depth
    depthSlider.setRange (0.0, 1.0);
    depthSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    depthSlider.addListener(this);
    depthLabel.setText ("Dry/Wet Level", juce::dontSendNotification);

    addAndMakeVisible (depthSlider);
    addAndMakeVisible (depthLabel);

    // delay Time
    timeSlider.setRange (0, 5, 0.1);
    timeSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    timeSlider.addListener(this);
    timeLabel.setText ("Delay time", juce::dontSendNotification);

    addAndMakeVisible (timeSlider);
    addAndMakeVisible (timeLabel);

    // feedback gain
    feedbackSlider.setRange(0.0,0.9);
    feedbackSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    feedbackSlider.addListener(this);
    feedbackLabel.setText("Feedback gain", juce::dontSendNotification);

    addAndMakeVisible (feedbackSlider);
    addAndMakeVisible (feedbackLabel);

    // frequency
    freqSlider.setRange(0.0, 10);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    freqSlider.addListener(this);
    freqLabel.setText("Frequency", juce::dontSendNotification);

    addAndMakeVisible(freqSlider);
    addAndMakeVisible(freqLabel);

    // phase
    phaseSlider.setRange(0.0, 0.9);
    phaseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    phaseSlider.addListener(this);
    phaseLabel.setText("Phase", juce::dontSendNotification);

    addAndMakeVisible(phaseSlider);
    addAndMakeVisible(phaseLabel);

    // sweep
    sweepSlider.setRange (0.0, 1.0);
    sweepSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    sweepSlider.addListener(this);
    sweepLabel.setText ("Sweep width", juce::dontSendNotification);

    addAndMakeVisible (sweepSlider);
    addAndMakeVisible (sweepLabel);

    setSize (400, 300);
}

StereoFlangerAudioProcessorEditor::~StereoFlangerAudioProcessorEditor()
{
}

//==============================================================================
void StereoFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Feedback Delay", getLocalBounds(), juce::Justification::centred, 1);
}

void StereoFlangerAudioProcessorEditor::resized()
{

    sweepLabel.setBounds (10, 10, 90, 20);
    sweepSlider.setBounds (100, 10, getWidth() - 110, 20);

    depthLabel.setBounds (10, 50, 90, 20);
    depthSlider.setBounds (100, 50, getWidth() - 110, 20);

    timeLabel.setBounds (10, 90, 90, 20);
    timeSlider.setBounds (100, 90, getWidth() - 110, 20);

    feedbackLabel.setBounds (10, 130, 90, 20);
    feedbackSlider.setBounds (100, 130, getWidth() - 110, 20);

    freqLabel.setBounds (10, 170, 90, 20);
    freqSlider.setBounds (100, 170, getWidth() - 110, 20);

    phaseLabel.setBounds (10, 210, 90, 20);
    phaseSlider.setBounds (100, 210, getWidth() - 110, 20);

}

void StereoFlangerAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &sweepSlider)
        audioProcessor.set_sweep(sweepSlider.getValue());
    else if (slider == &depthSlider)
        audioProcessor.set_depth(depthSlider.getValue());
    else if (slider == &timeSlider)
        audioProcessor.set_delayTime(timeSlider.getValue());
    else if (slider == &feedbackSlider)
        audioProcessor.set_feedback(feedbackSlider.getValue());
    else if (slider == &freqSlider)
        audioProcessor.set_freq(freqSlider.getValue());
    else if (slider == &phaseSlider)
        audioProcessor.set_phase(phaseSlider.getValue());
}
