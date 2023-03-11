//
//  Oscillators.h
//  Week4MoreClasses
//
//  Created by tmudd on 03/02/2020.
//  Copyright © 2020 tmudd. All rights reserved.
//

#ifndef Oscillators_h
#define Oscillators_h

// BASE class - a basic
class Phasor
{
public:
    
    // Our parent oscillator class does the key things required for most oscillators:
    // -- handles phase
    // -- handles setters and getters for frequency and samplerate
    
    /// update the phase and output the next sample from the oscillator
    float process()
    {
        phase += phaseDelta;
        
        if (phase > 1.0f)
            phase -= 1.0f;
        
        return output(phase);
    }

    float process(int ctrl)
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return output(phase, ctrl);
    }

    float process(float mod, float mod2, float lvl)
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return output(phase, mod, mod2, lvl);
    }
    
    /// this function is the one that we will replace in the classes that inherit from Phasor
    virtual float output(float p)
    {
        return p;
    }

    virtual float output(float p, int ctrl)
    {
        return p;
    }

    virtual float output(float p, float m, float m2, float lvl)
    {
        return p;
    }

    
    void setSampleRate(float sr)
    {
        sampleRate = sr;
    }
    
    void setFrequency(float freq)
    {
        frequency = freq;
        phaseDelta = frequency / sampleRate;
    }
    
private:
    float frequency;
    float sampleRate;
    float phase = 0.0f;
    float phaseDelta;
};

// CHILD Class
class TriOsc : public Phasor
{
    // redefine (override) the output function so that we can return a different function of the phase (p)
    float output(float p) override
    {
        return fabsf(p - 0.5f) - 0.5f;
    }
};


/// <summary>
/// Sin Oscilator class
/// </summary>
class SinOsc : public Phasor
{
    float output(float p) override
    {
        return std::sin(p * 2 * 3.14159);
    }
};

/// <summary>
/// Modulator class, that applies phase modulation to the one signal based on the output of the second.
/// </summary>
class Modulator : public Phasor
{
    /// <summary>
    /// Applies phase modulation to the one signal based on the output of the second.
    /// </summary>
    /// <param name="p"> phase</param>
    /// <param name="m"> Oscilator 1</param>
    /// <param name="m2"> Oscilator 2</param>
    /// <param name="lvl"> Gain level</param>
    /// <returns></returns>
    float output(float p , float m, float m2, float lvl) override
    {
        float m2Out = (pow(m2,2) * lvl) + 1;                        // Squares the sample to allow positive oscilation, along with amplitude gain and shift by 1 unit to prevent the sample to drop to 0.
        return std::sin((m * m2Out) + (p * 2 * 3.14159));           // Returns a phase modulated sample.  
    }
};


/**
 Squarewave Oscillator built on Phasor base class

 Includes setPulseWidth to change the waveform shape
 */
class SquareOsc : public Phasor
{
public:
    float output(float p) override
    {
        float outVal = 0.5;
        if (p > pulseWidth)
            outVal = -0.5;
        return outVal;
    }

    /// set square wave pulse width (0-1))
    void setPulseWidth(float pw)
    {
        pulseWidth = pw;
    }
private:
    float pulseWidth = 0.5f;
};



#endif /* Oscillators_h */
