/*
  ==============================================================================

    DelayLine.h
    Created: 5 Dec 2022 11:18:07am

  ==============================================================================
*/

#pragma once

class DelayLine
{

public:

    ~DelayLine()
    {
        if (data != nullptr)
            delete[] data;
    }


    /**
        Set values of the delay buffer to zero.
    */
    void clearDelayBuffer()
    {
        for (int i = 0; i < size; i++)  
        {
            data[i] = 0.0;
        }
    }


    /**
        Set maximum size of the delay line         
        @param newSize: Max buffer size
    */
    void setMaxSizeInSamples(int newSize) 
    {
        size = newSize;                         // store new size
        if (data != nullptr)                    // free up existing data
        {
            delete[] data;
        }

        data = new float[size];                 // initialize array

        clearDelayBuffer();                     // setting default values of the array to 0       
    }


    /**
        Set delay leangth in samples
        @param newDelayTime: Set delay length 
     */
    void setDelayTimeInSamples(float newDelayTime)
    {
        delayTime = newDelayTime;
        readIndex = writeIndex - delayTime;     // Set the readIndex behind the writeIndex by delayTime

        if (readIndex < 0)
            readIndex = readIndex + size;       // Keeps the readIndex from going out of bounds.
    }


    /**
        Sets the feedback for the delay
        @param newFeedback: Delay feedback value
    */
    void setFeedback(float newFeedback)
    {
        feedback = newFeedback;
        if (feedback < 0)
            feedback = 0;
        if (feedback > 1.0)
            feedback = 1.0;
    }


    /**
        Interpolates values between samples to reduce aliasing
    */
    float linearInterpolation()
    {
       
        // get the two indexes around our read index
        int indexA = int(readIndex);        
        int indexB = indexA + 1;            

        // wrap indexB if necessary
        if (indexB > size)
            indexB -= size;

        // get values at data indexes
        float valA = data[indexA];          
        float valB = data[indexB];          


        // calculate remainder
        float remainder = readIndex - indexA;   

        // work out interpolated sample between two indexes
        float interpolatedSample = (1 - remainder) * valA + remainder * valB;


        return interpolatedSample;
    }

    /**
        run through every sample:
        1) store new samples
        2) update/advance read/write index
        3) return the read index value 
        @param inputSample
    */
    float process(float inputSample)
    {
        float outputSample = linearInterpolation();                         // gets the value of the sample at readIndex

        data[writeIndex] = inputSample + (outputSample * feedback);         // stores the input sample to the data buffer and adds the feedback multiplied with feedback amount

        readIndex++;                                                        // advance the readIndex
        if (readIndex >= size)                                              // wrap the index to the start
            readIndex -= size;

        writeIndex++;                                                       // advance the writeIndex
        if (writeIndex >= size)                                             // wrap the index to the start
            writeIndex -= size;

        return outputSample;

    }



private:

    float* data = nullptr;      // For storing input buffer
    int delayTime;              // Leangth of delay in samples
    int size;                   // Maximum delay time 
    float readIndex = 0;        // Read position as an index 
    int writeIndex = 0;         // Write position as an index
    float feedback;             // Feedback amount
};