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
class AudioProg_assignment3AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioProg_assignment3AudioProcessorEditor (AudioProg_assignment3AudioProcessor&);
    ~AudioProg_assignment3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:


    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioProg_assignment3AudioProcessor& audioProcessor;

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProg_assignment3AudioProcessorEditor)
};
