/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioProg_assignment3AudioProcessor::AudioProg_assignment3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    parameters(*this, nullptr, "ParamTreeIdentifier",
        {
            std::make_unique<juce::AudioParameterFloat>("inputGain", "Input Gain", 0.0f, 1.0f, 0.2f),                                   // Input Gain, Range: 0.0 - 1.0, Default: 0.2 
            std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 30.0f, 0.5f),                                           // Drive, Range: 0.0 - 10.0, Default: 0.5       (Overdrive)         
            std::make_unique<juce::AudioParameterFloat>("outputGain", "Output Gain", 0.0f, 2.0f, 0.2f),                                 // Output Gain, Range: 0.0 - 2.0, Default: 0.2
            std::make_unique<juce::AudioParameterBool>("recLoop", "Loop", false),                                                       // Loop Record, Boolean, Default: false         (Enables looping)           
            std::make_unique<juce::AudioParameterFloat>("mix1", "Delay Mix", 0.0f, 1.0f, 0.2f),                                         // Delay Mix, Range: 0.0 - 1.0, Default: 0.2
            std::make_unique<juce::AudioParameterFloat>("delayLength", "Delay Length (sec)", 0.4f, 32.0f, 0.5f),                        // Delay Length, Range: 0.01 - 0.8, Default: 0.5            
            std::make_unique<juce::AudioParameterFloat>("delayFeedback", "Feedback", 0.01f, 0.9f, 0.05f),                               // Delay Feedback, Range: 0.01 - 0.9, Default: 0.05
            std::make_unique<juce::AudioParameterBool>("delayToggle", "Delay Clear", false),                                            // Delay Toggle, Range: 0.0 - 1.0, Default: 0.2 (Delay buffer Clear)
            std::make_unique<juce::AudioParameterChoice>("filterType", "Filter Type", juce::StringArray({"Bass", "Wide", "High"}), 0),  // Filter Type, Choice: (Bass, Wide, High), Default: 0
            std::make_unique<juce::AudioParameterFloat>("filterQ", "Filter Q", 0.1f, 18.0f, 0.5f)                                       // Q for filter, Range: 0.1 - 18.0, Default: 0.5           
        })
{
    // Link the input parameters to their respective variables
    inputGainParam = parameters.getRawParameterValue("inputGain");
    outputGainParam = parameters.getRawParameterValue("outputGain");
    driveParam = parameters.getRawParameterValue("drive");
    delayMixParam = parameters.getRawParameterValue("mix1");    
    delayLengthParam = parameters.getRawParameterValue("delayLength");
    delayFeedbackParam = parameters.getRawParameterValue("delayFeedback");
    filterQVal = parameters.getRawParameterValue("filterQ");
    filterChoiceParam = parameters.getRawParameterValue("filterType");
    delayToggleParam = parameters.getRawParameterValue("delayToggle");
    recLoopParam = parameters.getRawParameterValue("recLoop");    
}

AudioProg_assignment3AudioProcessor::~AudioProg_assignment3AudioProcessor()
{
}


//==============================================================================
void AudioProg_assignment3AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (int i = 0; i < 2; i++)
        vec[i].delaySetup(sampleRate);                  // Set sample rate for both instances of multiDelay 
    
    // Sets the sample Rate and RampLengthInSeconds
    smoother.reset(sampleRate, 0.000005);       
    smoother.setCurrentAndTargetValue(0);

    smootherQ.reset(sampleRate, 0.005);
    smootherQ.setCurrentAndTargetValue(0);    
}


void AudioProg_assignment3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    smoother.setTargetValue(*delayLengthParam);                                                                                         // Sets target value for delay Length smoother.
    smootherQ.setTargetValue(*filterQVal);                                                                                              // Set the target value for filter Q smoother.
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
       
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());                                                                                     // Clears the buffers in left and right channels
   
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float* inputData = buffer.getReadPointer(channel);                                                                        
        float* outputData = buffer.getWritePointer(channel);

        vec[channel].clearDelayBuffers(*delayToggleParam);                                                                              // Clears the delay buffer if *delayToggleParam is true
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {           
            vec[channel].delayAssignValue(smoother.getNextValue(), *delayFeedbackParam);                                                // Assigns delay length and feedback for the delayBufferVector
            float inputSample = inputData[sample];                                                                                      // stores the sample from the input buffer                                                                                                    // 
            const auto input = inputSample * *inputGainParam;                                                                           // Applie gain to the input sample
            const auto softClip = od1.process(input, *driveParam);                                                                      // Applies subtle overdrive to the signal
            float delayIn = 0.0;                                                                                                        // the input signal bypasses the delay process and allows the user to play over the loop without affecting it. 
             
            if (*recLoopParam == true)                                                                                                  // Saves the input audio to be sent to delay buffer 
                delayIn = softClip;

            const auto delayedSample = vec[channel].delaySumAudioVectors(delayIn, *filterChoiceParam, smootherQ.getNextValue());        // sends the sample into the MultiDelay class for looping, along with filter choice, and filter Q.
            
            auto blend = softClip * (1.0 - *delayMixParam) + delayedSample * *delayMixParam;                                            // To controll the mix of delayed signal with input.
            blend *= *outputGainParam;                                                                                                  // Applies output gain on the output signal

            if (blend > 1)                                                                                                              // Limiter to keep the samples level below 1
                blend = 1;
                       
            outputData[sample] = blend;                                                                                                 // Sends procesed sample to output buffer
        }        
    }   
}


//==============================================================================
const juce::String AudioProg_assignment3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioProg_assignment3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioProg_assignment3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioProg_assignment3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioProg_assignment3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioProg_assignment3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioProg_assignment3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioProg_assignment3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioProg_assignment3AudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioProg_assignment3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================


void AudioProg_assignment3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioProg_assignment3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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



//==============================================================================
bool AudioProg_assignment3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioProg_assignment3AudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AudioProg_assignment3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioProg_assignment3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioProg_assignment3AudioProcessor();
}
