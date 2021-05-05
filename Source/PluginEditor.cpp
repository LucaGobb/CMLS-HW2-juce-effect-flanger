/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/



#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MyPlugInAudioProcessorEditor::MyPlugInAudioProcessorEditor (MyPlugInAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
{

    setSize (600, 400);

    // freqDepth parameters
    freqDepth.setSliderStyle(juce::Slider::Rotary);
    freqDepth.setRotaryParameters(-2.34, 2.34, true);
    //freqDepth.setRange(1.0, 1.059, 0.001);
    freqDepth.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    freqDepth.setPopupDisplayEnabled(true, false, this);
    freqDepth.setValue(1.0);
    freqDepth.setColour(23, juce::Colours::black) ;
    addAndMakeVisible(&freqDepth);
    depthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DEPTH", freqDepth );
    freqDepth.addListener(this);

    // rate parameters
    rate.setSliderStyle(juce::Slider::Rotary);
    rate.setRotaryParameters(-2.34, 2.34, true);
    //rate.setRange(0.1, 8, 0.1);
    rate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    rate.setPopupDisplayEnabled(true, false, this);
    rate.setTextValueSuffix(" Hz");
    rate.setValue(1.0);
    addAndMakeVisible(&rate);
    rateSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "RATE", rate );
    rate.addListener(this);



    // delay parameters
    delay.setSliderStyle(juce::Slider::Rotary);
    delay.setRotaryParameters(-2.34, 2.34, true);
    //delay.setRange(0, 30, 0.1);
    delay.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    delay.setPopupDisplayEnabled(true, false, this);
    delay.setTextValueSuffix(" ms");
    delay.setValue(10);
    addAndMakeVisible(&delay);
    delaySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DELAY", delay );
    delay.addListener(this);



    // phaseBalance parameters
    phaseBalance.setSliderStyle(juce::Slider::Rotary);
    phaseBalance.setRotaryParameters(-2.34, 2.34, true);
    //phaseBalance.setRange(0, 180, 1);
    phaseBalance.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    phaseBalance.setPopupDisplayEnabled(true, false, this);
    phaseBalance.setTextValueSuffix(" degrees");
    phaseBalance.setValue(0.0);
    addAndMakeVisible(&phaseBalance);
    phaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PHASE", phaseBalance );
    phaseBalance.addListener(this);

    // dryWet parameters
    dryWet.setSliderStyle(juce::Slider::Rotary);
    dryWet.setRotaryParameters(-2.34, 2.34, true);

    dryWet.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    dryWet.setPopupDisplayEnabled(true, false, this);
    dryWet.setTextValueSuffix("");
    dryWet.setValue(0.5);
    addAndMakeVisible(&dryWet);
    dryWetSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DRYWET", dryWet );
    dryWet.addListener(this);

    // regenGain parameters
    regenGain.setSliderStyle(juce::Slider::Rotary);
    regenGain.setRotaryParameters(-2.34, 2.34, true);
    //regenGain.setRange(0, 0.95, 0.01);
    regenGain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    regenGain.setPopupDisplayEnabled(true, false, this);
    regenGain.setTextValueSuffix("");
    regenGain.setValue(0.0);
    addAndMakeVisible(&regenGain);
    regenGainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "REGEN", regenGain );
    regenGain.addListener(this);
}

// Pull new values when a slider value changes
void MyPlugInAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    audioProcessor.f_ratio = (float)freqDepth.getValue();

    audioProcessor.left_LFO1.resetFrequency( (float)rate.getValue() );    //+ (float)rateFine.getValue()
    audioProcessor.right_LFO1.resetFrequency( (float)rate.getValue() );// + (float)rateFine.getValue()

    audioProcessor.delayMinimum = 48 * (int)delay.getValue() ; //+ (int)delayFine.getValue()


    audioProcessor.right_LFO1.setPhaseOffset(phaseBalance.getValue() * (float) M_PI / 180.0f);

    audioProcessor.delayGain = dryWet.getValue();

    audioProcessor.regenGain = regenGain.getValue();



};

MyPlugInAudioProcessorEditor::~MyPlugInAudioProcessorEditor()
{
}

//==============================================================================
void MyPlugInAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

    // Set labels and title
    g.setColour (juce::Colours::white);
    g.setFont(35.0f);
    g.drawFittedText("Flanger", 0, 0, getWidth(), getHeight()-290, juce::Justification::centred, 1);
    g.setFont (20.0f);
    g.drawFittedText("Depth", 35, 130, 60, 30, juce::Justification::centred, 2, 1.0f);
    g.drawFittedText("Rate", 125, 130, 60, 30, juce::Justification::centred, 2, 1.0f);
    g.drawFittedText("Delay", 215, 130, 60, 30, juce::Justification::centred, 2, 1.0f);
    g.drawFittedText("Phase Offset", 305, 130, 60, 30, juce::Justification::centred, 2, 1.0f);
    g.drawFittedText("Regen", 395, 130, 60, 30, juce::Justification::centred, 2, 1.0f);
    g.drawFittedText("Dry/Wet", 485, 130, 120, 30, juce::Justification::centred, 2, 1.0f);
}

void MyPlugInAudioProcessorEditor::resized()
{
    // Locations of all rotary sliders
    freqDepth.setBounds(0, 160, getWidth() * .22, getHeight() * .22);
    rate.setBounds(90, 160, getWidth() * .22, getHeight() * .22);
    delay.setBounds(180, 160, getWidth() * .22, getHeight()* .22);
    phaseBalance.setBounds(270, 160, getWidth() * .22, getHeight() * .22);
    regenGain.setBounds(360, 160, getWidth() * .22, getHeight() * .22);
    dryWet.setBounds(450, 160, getWidth() * .22, getHeight() * .22);
}
