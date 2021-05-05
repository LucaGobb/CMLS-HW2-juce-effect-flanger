/*
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
    //Wet
    wetSlider.setRange (0.0, 1.0);
    wetSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    wetSlider.addListener(this);
    wetLabel.setText ("Wet Level", juce::dontSendNotification);

    addAndMakeVisible (wetSlider);
    addAndMakeVisible (wetLabel);

    //Dry
    drySlider.setRange (0.0, 1.0);
    drySlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    drySlider.addListener(this);
    dryLabel.setText ("Dry Level", juce::dontSendNotification);

    addAndMakeVisible (drySlider);
    addAndMakeVisible (dryLabel);

    //Delay Time
    timeSlider.setRange (500, 50000, 100);
    timeSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    timeSlider.addListener(this);
    timeLabel.setText ("Time", juce::dontSendNotification);

    addAndMakeVisible (timeSlider);
    addAndMakeVisible (timeLabel);

    // Feedback Duration
    feedbackSlider.setRange(0.0,0.9);
    feedbackSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    feedbackSlider.addListener(this);
    feedbackLabel.setText("Feedback duration", juce::dontSendNotification);

    addAndMakeVisible (feedbackSlider);
    addAndMakeVisible (feedbackLabel);

    // Frequency
    freqSlider.setRange(0.1,10);
    freqSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    freqSlider.addListener(this);
    freqLabel.setText("freq", juce::dontSendNotification);

    addAndMakeVisible (freqSlider);
    addAndMakeVisible (freqLabel);

    // Width (aka sweep)
    widthSlider.setRange(0.0,0.9);
    widthSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);
    widthSlider.addListener(this);
    widthLabel.setText("do not use", juce::dontSendNotification);

    addAndMakeVisible (widthSlider);
    addAndMakeVisible (widthLabel);

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

    wetLabel.setBounds (10, 10, 90, 20);
    wetSlider.setBounds (100, 10, getWidth() - 110, 20);

    dryLabel.setBounds (10, 50, 90, 20);
    drySlider.setBounds (100, 50, getWidth() - 110, 20);

    timeLabel.setBounds (10, 90, 90, 20);
    timeSlider.setBounds (100, 90, getWidth() - 110, 20);

    feedbackLabel.setBounds (10, 130, 90, 20);
    feedbackSlider.setBounds (100, 130, getWidth() - 110, 20);

    freqLabel.setBounds (10, 170, 90, 20);
    freqSlider.setBounds (100, 170, getWidth() - 110, 20);

    widthLabel.setBounds (10, 210, 90, 20);
    widthSlider.setBounds (100, 210, getWidth() - 110, 20);


}

void StereoFlangerAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &wetSlider)
        audioProcessor.set_wet(wetSlider.getValue());
    else if (slider == &drySlider)
        audioProcessor.set_dry(drySlider.getValue());
    else if (slider == &timeSlider)
        audioProcessor.set_ds(timeSlider.getValue());
    else if (slider == &feedbackSlider)
        audioProcessor.set_fb(feedbackSlider.getValue());
    else if (slider == &freqSlider)
        audioProcessor.set_freq(freqSlider.getValue());
    else if (slider == &widthSlider)
        audioProcessor.set_width(widthSlider.getValue());
}
