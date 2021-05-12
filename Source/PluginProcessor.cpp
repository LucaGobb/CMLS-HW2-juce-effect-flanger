
/*
  ==============================================================================
    This file was auto-generated!
    It contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/


#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PresetListBox.h"
#include "./../JuceLibraryCode/BinaryData.h"



static juce::String waveformType {"lfoWaveType"};




StereoFlangerAudioProcessor::StereoFlangerAudioProcessor()
        : treeState(*this, nullptr, ProjectInfo::projectName, {


        std::make_unique<juce::AudioParameterFloat>("depth",            // parameterID
                                                    "Depth",            // parameter name
                                                    juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f ),
                                                    0.7f),// default value
        std::make_unique<juce::AudioParameterFloat>("phase",      // parameterID
                                                    "Phase",     // parameter name
                                                    juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f ),
                                                    0.5f),   // default value
        std::make_unique<juce::AudioParameterFloat>("frequency",            // parameterID
                                                    "Frequency",            // parameter name
                                                    juce::NormalisableRange<float> (0.01f, 10.0f, 0.001f , 0.150),
                                                    1.0f),             // default value
        std::make_unique<juce::AudioParameterFloat>("delay",      // parameterID
                                                    "Delay",     // parameter name
                                                    juce::NormalisableRange<float> (0.10f, 7.0f, 0.01f ),
                                                    2.5f),   // default value
        std::make_unique<juce::AudioParameterFloat>("feedback",            // parameterID
                                                    "Feedback",            // parameter name
                                                    juce::NormalisableRange<float> (0.0f, 0.99f, 0.01f),
                                                    0.8f),             // default value
        std::make_unique<juce::AudioParameterFloat>("sweep",      // parameterID
                                                    "Sweep",     // parameter name
                                                    juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
                                                    0.5f),   // default value

        std::make_unique<juce::AudioParameterBool> ("invertPhase",      // parameterID
                                                    "Invert Phase",     // parameter name
                                                    false),              // default value

        std::make_unique<juce::AudioParameterChoice>(waveformType, "Type", juce::StringArray ("Sine", "Square", "Sawtooth", "Triangle"), 0)




}) {
    phaseRL = treeState.getRawParameterValue("phase");
    depth = treeState.getRawParameterValue("depth");
    freq = treeState.getRawParameterValue("frequency");
    delayTime = treeState.getRawParameterValue("delay");
    feedback = treeState.getRawParameterValue("feedback");
    sweep = treeState.getRawParameterValue("sweep");
    phaseSwitch = treeState.getRawParameterValue ("invertPhase");
    lfoWaveType = treeState.getRawParameterValue("lfoWaveType");




    auto defaultGUI = magicState.createDefaultGUITree();
    magicState.setGuiValueTree(defaultGUI);

    auto file = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
            .getChildFile("Contents")
            .getChildFile("Resources")
            .getChildFile("newnewdefaultflanger.xml");

    if (file.existsAsFile())
        magicState.setGuiValueTree(file);
    else
        magicState.setGuiValueTree(BinaryData::newnewdefaultflanger_xml,
                                   BinaryData::newnewdefaultflanger_xmlSize
                                   );


    // MAGIC GUI: add a meter at the output
    outputMeter = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
    oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");

    analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing(analyser);

    presetList = magicState.createAndAddObject<PresetListBox>("presets");
    presetList->onSelectionChanged = [&](int number) {
        loadPresetInternal(number);
    };


    magicState.addTrigger("save-preset", [this] {
        savePresetInternal();
    });

    magicState.setApplicationSettingsFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                                  .getChildFile(ProjectInfo::companyName)
                                                  .getChildFile(ProjectInfo::projectName + juce::String(".settings")));

    magicState.setPlayheadUpdateFrequency(30);



}


//==============================================================================
void StereoFlangerAudioProcessor::prepareToPlay (double sampleRate, int blockSize)
{

    //buffer inizialization:
    //   (delay minimum + sweep) in samples + 3 sample for interpolation

    delayBufferLength = (int)ceil((maxDelayTime + maxSweepWidth) * 0.001f * sampleRate) + 3;
    dbuf.setSize(getTotalNumOutputChannels(), delayBufferLength);
    dbuf.clear();

    samplingFrequency = sampleRate;


    // MAGIC GUI: setup the output meter
    outputMeter->setupSource (getTotalNumOutputChannels(), sampleRate, 500, 200);
    oscilloscope->prepareToPlay (sampleRate, blockSize);
    analyser->prepareToPlay (sampleRate, blockSize);
}

void StereoFlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StereoFlangerAudioProcessor::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

    
    // MAGIC GUI: send playhead information to the GUI
    magicState.updatePlayheadInformation (getPlayHead());


    int numSamples = buffer.getNumSamples();

    // freeze variables
    float depth_ = *depth; // values: from 0 to 1
    float fb_now = *feedback; // 0 to 0.9
    float sweep_sample = *sweep * getSampleRate() * 0.01f; // sweep: 0 to 1
    int waveform_ = (int)*lfoWaveType;
    bool isPhaseInverted = (bool)*phaseSwitch;
    int sign = (isPhaseInverted) ? -1 : 1;

    float freq_now = *freq;
    float delay_min_sample = *delayTime * 0.001f * getSampleRate(); // 0 to 5 [ms]
    float phaseRL_now = *phaseRL;


    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    const float* channelInDataL = buffer.getReadPointer(0);
    const float* channelInDataR = buffer.getReadPointer(1);

    // Delay line
    for (int i = 0; i < numSamples; ++i) {

        float interpolatedSampleL = 0.0;
        float interpolatedSampleR = 0.0;

        float currentDelayL = delay_min_sample + sweep_sample * lfo(phase,waveform_);
        float currentDelayR = delay_min_sample + sweep_sample * lfo(fmodf(phase + phaseRL_now, 1.0f),waveform_);

        // delay in sample
        drL = fmodf((float)dw - (float)(currentDelayL)+(float)delayBufferLength - 1.0f, (float)delayBufferLength);
        drR = fmodf((float)dw - (float)(currentDelayR)+(float)delayBufferLength - 1.0f, (float)delayBufferLength);

        // interpolation

        float fractionL = drL - floorf(drL);
        float fractionR = drR - floorf(drR);

        // Linear interpolation
        // int previousSampleL = (int)floorf(drL);
        // int previousSampleR = (int)floorf(drR);
        // int nextSampleL = (previousSampleL + 1) % delayBufferLength;
        // int nextSampleR = (previousSampleR + 1) % delayBufferLength;
        // interpolatedSampleL = fractionL * dbuf.getSample(0, nextSampleL) + (1.0f - fractionL) * dbuf.getSample(0, previousSampleL);
        // interpolatedSampleR = fractionR * dbuf.getSample(1, nextSampleR) + (1.0f - fractionR) * dbuf.getSample(1, previousSampleR);

        // Cubic interpolation

        float frsqL = fractionL*fractionL;
        int sample1L = (int)floorf(drL);
        int sample2L = (sample1L + 1) % delayBufferLength;
        int sample3L = (sample2L + 1) % delayBufferLength;
        int sample0L = (sample1L - 1 + delayBufferLength) % delayBufferLength;

        float frsqR = fractionR*fractionR;
        int sample1R = (int)floorf(drR);
        int sample2R = (sample1R + 1) % delayBufferLength;
        int sample3R = (sample2R + 1) % delayBufferLength;
        int sample0R = (sample1R - 1 + delayBufferLength) % delayBufferLength;

        float a0L = -0.5f*dbuf.getSample(0, sample0L) + 1.5f*dbuf.getSample(0, sample1L)
                            - 1.5f*dbuf.getSample(0, sample2L) + 0.5f*dbuf.getSample(0, sample3L);
        float a1L = dbuf.getSample(0, sample0L) - 2.5f*dbuf.getSample(0, sample1L)
                    + 2.0f*dbuf.getSample(0, sample2L) - 0.5f*dbuf.getSample(0, sample3L);
        float a2L = -0.5f*dbuf.getSample(0, sample0L) + 0.5f*dbuf.getSample(0, sample2L);
        float a3L = dbuf.getSample(0, sample1L);

        interpolatedSampleL = a0L*fractionL*frsqL + a1L*frsqL + a2L*fractionL + a3L;

        float a0R = -0.5f*dbuf.getSample(1, sample0R) + 1.5f*dbuf.getSample(1, sample1R)
                            - 1.5f*dbuf.getSample(1, sample2R) + 0.5f*dbuf.getSample(1, sample3R);
        float a1R = dbuf.getSample(1, sample0R) - 2.5f*dbuf.getSample(1, sample1R)
                    + 2.0f*dbuf.getSample(1, sample2R) - 0.5f*dbuf.getSample(1, sample3R);
        float a2R = -0.5f*dbuf.getSample(1, sample0R) + 0.5f*dbuf.getSample(1, sample2R);
        float a3R = dbuf.getSample(1, sample1R);

        interpolatedSampleR = a0R*fractionR*frsqR + a1R*frsqR + a2R*fractionR + a3R;

        // Feedback
        dbuf.setSample(0, dw, channelInDataL[i] + interpolatedSampleL * fb_now);
        dbuf.setSample(1, dw, channelInDataR[i] + interpolatedSampleR * fb_now);

        channelOutDataL[i] = channelInDataL[i] + sign * depth_ * interpolatedSampleL;
        channelOutDataR[i] = channelInDataR[i] + sign * depth_ * interpolatedSampleR;

        dw = (dw + 1) % delayBufferLength;

        phase += freq_now / samplingFrequency;
        if (phase >= 1.0)
            phase -= 1.0;
    }

    // MAGIC GUI: send the finished buffer to the level meter
    outputMeter->pushSamples (buffer);
    oscilloscope->pushSamples (buffer);
    analyser->pushSamples (buffer);
}

//==============================================================================
void StereoFlangerAudioProcessor::savePresetInternal()
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);

    juce::ValueTree preset { "Preset" };
    preset.setProperty ("name", "Preset " + juce::String (presetNode.getNumChildren() + 1), nullptr);

    foleys::ParameterManager manager (*this);
    manager.saveParameterValues (preset);

    presetNode.appendChild (preset, nullptr);
}

void StereoFlangerAudioProcessor::loadPresetInternal(int index)
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);
    auto preset = presetNode.getChild (index);

    foleys::ParameterManager manager (*this);
    manager.loadParameterValues (preset);
}

//==============================================================================

double StereoFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoFlangerAudioProcessor();
}

float StereoFlangerAudioProcessor::lfo(float phase, int waveform)
{
    switch (waveform)
    {
        case 0:
            // Sine
        default:
            return 0.5f + 0.5f * sinf(2.0 * 3.1416 * phase);

        case 1: // Square
            if (phase < 0.5f)
                return 1.0f;
            else
                return 0.0f;
        case 2: // Sawtooth
            if (phase < 0.5f)
                return 0.5f + phase;
            else
                return phase - 0.5f;
        case 3: // Triangle
            if (phase < 0.25f)
                return 0.5f + 2.0f * phase;
            else if (phase < 0.75f)
                return 1.0f - 2.0f * (phase - 0.25f);
            else
                return 2.0f * (phase - 0.75f);
    }
}


//==========================================
