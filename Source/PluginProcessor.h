/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"
#include "MultiDelay.h"

//==============================================================================
/**
*/
class AudioProg_assignment3AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AudioProg_assignment3AudioProcessor();
    ~AudioProg_assignment3AudioProcessor() override;

    //==============================================================================
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

private:

    MultiDelay vec[2];                                  // Two instances of MultiDelay, for two left and right channel.
    Overdrive od1;                                      // Overdrive instance
    juce::SmoothedValue<float> smoother;                // Smoother for Delay Length
    juce::SmoothedValue<float> smootherQ;               // Smoother for Filter Q
    
    // Initilializing input parameters.
    juce::AudioProcessorValueTreeState parameters;      
    std::atomic<float>* inputGainParam;                 
    std::atomic<float>* outputGainParam;                
    std::atomic<float>* driveParam;                     
    std::atomic<float>* delayMixParam;                  
    std::atomic<float>* delayToggleParam;               
    std::atomic<float>* recLoopParam;                
    std::atomic<float>* driveChoiceParam;               
    std::atomic<float>* delayFeedbackParam;             
    std::atomic<float>* delayLengthParam;               
    std::atomic<float>* filterChoiceParam;              
    std::atomic<float>* filterQVal;                     



    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProg_assignment3AudioProcessor)
};
