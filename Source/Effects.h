/*
  ==============================================================================

    Effects.h
    Created: 2 Nov 2022 12:45:10am

    LFO Filters
    Oscilate the JUCE filter in the specfied range at a specified frequency.
  ==============================================================================
*/
#ifndef Effects_h
#define Effects_h

#include <JuceHeader.h>
#include "Oscillators.h"

/**
LFO filter class.
*/
class LFOFilter 
{
public:

    /**
    Set the parameters for the filter LFO.
    @param sr: Sample Rate of the project.
    @param freq: Frequency of the LFO.
    */
    void filterSetParams(float sr, float freq)
    {
        sampleRate = sr;
        lfo.setSampleRate(sampleRate);
        lfo.setFrequency(freq);
        filter.reset();                     // Clears the object. 
    }


    /**
    Function returns a filtered sample with an oscilating frequency cutt off value.
    @param sample:  The input sample
    @param range: The range of the filter. 
    @param shift: The positive shift of the oscilator that determines the lowest value of the oscilator.
    @param qVal: Q value for filter
    @param cutOffIn: Frequency value
    */
    float lfoFilter(float sample, float range, float shift, float qVal, float cutOffIn)
    {
        float lfoOut = lfo.process();                                                               // The output of the lfo oscilator
        cutOff = (pow(lfoOut, 2) * range) + shift;                                                  // Squares the signal, so that it stays positive. Then applies the gain and shift's the wave to the desired minimum value
        filter.setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, cutOffIn, qVal));    // JUCE filter class. Setting sample rate and cuttOff.
        float filterOut = filter.processSingleSampleRaw(sample);                                    // Applies the filter on the main sample.
        return filterOut;
    }

private:
    SinOsc lfo;                 // LFO for the filter
    float cutOff;               // Variable to store the cutt off value of the filter.
    juce::IIRFilter filter;     // Creating an instance of the JUCE filter class.
    float sampleRate;           // Variable to store the sample rate.
};


    /**
    Pulser class,
    Generates a signal that pulses in a long regular interval.
    */
class Pulser
{
public:

    /**
    Set the parameters of the oscilators.
    @param sr: Sample rate of the project.
    @param lev: The output level of the pulse oscilator.
    */
    void setSampleRate(float sr, float lev)
    {
        sampleRate = sr;
        level = lev;
        freqOsc.setSampleRate(sampleRate);
        lfo.setSampleRate(sampleRate);
        lfo.setFrequency(0.01f);
    }


    /**
    The output of the oscilator is raised to the power of 48 and multiplied by a gain level.
    */
    float process()
    {
        freqOsc.setFrequency(pow(lfo.process(), 48));   // Set's the frequency for freqOsc. The output of the lfo raised to 48 is used to allow the freqOsc to oscilate in less frequent intervals. 
        oscOut = freqOsc.process();
        return oscOut * level;
      
    }
private:
    float oscOut;
    float sampleRate;
    float level;
    SinOsc freqOsc;
    SinOsc lfo;
};


/**
    A simple class for audio distortion
*/
class Overdrive
{
public:

    /**
        Returns the preocessed signal
        @param inSample: input audio sample
        @param driveIn: The input drive parameter value
    */
    float process(float inSample, float driveIn)
    {
        return divPi * std::atanf(inSample * driveIn);  // A atan function is applied on the input sample.
    }

private:
    const float divPi = 2.0 / juce::MathConstants<float>::pi; // Saves the calue of 2/pi
};

#endif // !Effects_h

