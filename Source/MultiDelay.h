/*
  ==============================================================================

    MultiDelay.h
    Created: 1 Nov 2022 6:59:24pm

  ==============================================================================
*/


#ifndef MultiDelay_h
#define MultiDelay_h

#include <JuceHeader.h>
#include <vector>
#include "DelayLine.h"
#include "Effects.h"

    /**
        Class to handle vector operations. 
    */
class MultiDelay
{
public:

    /**
         Set sample rate to assign delay buffer length.
         Along with initializing different objects in the class.
         @param Sample Rate
    */
    void delaySetup(float sr)
    {
        sampleRate = sr;
        size = delayVec.size();                                                             // Stores the size of the delay vector
                                                                                            
        smoothfilterFreq.reset(sampleRate, 0.000005);                                       // Sets the sample rate and rampLengthIn seconds                         
        smoothfilterFreq.setCurrentAndTargetValue(0);                                       // Set new Value to 0
                                                                                            
        bufferSize = sampleRate * 20;                                                       // The buffersize variable to set Max delay length
        for (int i = 0; i < size; i++)                                                      // Loop that runs through the delay Vectors        
        {
            maxDelayLength = bufferSize * (0.2 * (i + 1));
            delayVec[i]->setMaxSizeInSamples(maxDelayLength);                               // Assigns max delay buffer size to each delay buffer, setting size of 4 seconds on delay[0] to 80 seconds to delay[19] 
        }
            
    }                                                                                       
                                                                                            
                                                                                            
    /**                                                                                     
       Function to assign delay length and feedback for each buffer.                        
       @param delayLengthIn: Delay length input parameter                                   
       @param feedbackIn: Delay Feedback input parameter                                    
    */                                                                                      
    void delayAssignValue(float delayLengthIn, float feedbackIn)                            
    {                                                                                       
                                                                                            
        for (int i = 0; i < size; i++)                                                      
        {                                                                                   
            delayLength = bufferSize * ((0.1 * (i + 1)) * (delayLengthIn/ 40));             // Multiplies BufferSize variable with delayLength input param, max size of delay length is 2 seconds for delay[0] to 40 seonds for delay[19]           
            feedbackVal = (i + 0.1) * feedbackIn;                                           // Feedback value for each buffer, The feedbackIn parameter value is applied to all buffers.              
                                                                                            
            delayVec[i]->setDelayTimeInSamples(delayLength);                                // Sets the delay length for each buffer
            delayVec[i]->setFeedback(feedbackVal);                                          // Sets feedback for each buffer.   
        }                                                                                   
                                                                                            
    }                                                                                       
                                                                                            
                                                                                            
    /**                                                                                     
        Void function to clear samples in delay buffers.                                    
        @param delayToggleVal: Boolean variable to trigger the function.                    
    */                                                                                      
    void clearDelayBuffers(bool delayToggleVal)                                             
    {                                                                                       
        if (delayToggleVal == true)                                                         // If delayToggleVal is true then clear the buffers.
        {                                                                                   
            for (int i = 0; i < size; i++)                                                  
            {                                                                               
                delayVec[i]->clearDelayBuffer();                                            // Calls the clearDelayBuffer() from DelayLine.h for each buffer in the vector. 
            }                                                                               
        }                                                                                   
                                                                                            
    }                                                                                       
                                                                                            
                                                                                            
    /**                                                                                     
        Applies a filter on individual buffer                                               
        @param inSample: Input sample                                                       
        @param index: index of buffer in the vector                                         
        @param type: Type of filter (Low-Pass, Wide-Band and High-Pass)                     
    */                                                                                      
    float delayBufferFilter(float inSample,int index, int type, float qVal)                 
    {                                                                                       
                                                                                            
        float cutOffIndex = index + 1;                                                                                                  // Shifts index range from 0-19 to 1-20
        switch (type)                                                                                                                   // Switch funtion that uses the type input variable to select a filter type.
        {                                                                                                                               
        case 0:                                                                                                                         
            filterFreq = 550 - (cutOffIndex * 25);                                                                                      // Low Band Pass filter, applying different low frequency values to each buffer to minimize crowding
            break;                                                                                                                      
                                                                                                                                        
        case 1:                                                                                                                         
            filterFreq = 20500 - (cutOffIndex * (20000 / size));                                                                        // Wide Band Pass filter, covers most ferquencies, each buffer has a specific frequency assigned to it.
            break;                                                                                                                      
                                                                                                                                        
        case 2:                                                                                                                         
            filterFreq = 10500 - (cutOffIndex * 500);                                                                                   // High Band Pass filter, Covers high frequency bands and each buffer has a specific frequency assigned to it.
            break;                                                                                                                      
        }                                                                                                                               
                                                                                                                                        
        smoothfilterFreq.setTargetValue(filterFreq);                                                                                    // filterFreq is set as new target value.

        filterVec[index]->setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, smoothfilterFreq.getNextValue(), qVal));      // JUCE filter class. Setting sample rate, frequency and Q.
        filterOut = filterVec[index]->processSingleSampleRaw(inSample);                                                                 // applies respective filter on the input signal.
                
        return filterOut;
    }
 

    /**
        Returns the sum of all the delay buffers.
        @param sample: input audio sample
        @param filterType: filter type to be assigned to the delay buffers
        @param qVal: Q for filter bands.
    */
    float delaySumAudioVectors(float sample, int filterType, double qVal)
    {      
        vectSum = 0;                                                                                // Sets vectorSum default to 0 for each iteration

        for (int i = 0; i < size; i++)                                                              // For loop to add the output sample of each Oscilator in the vector. 
        {
            filterSample = delayBufferFilter(delayVec[i]->process(sample), i, filterType, qVal);    // Calls the filter function for each buffer
            vectSum += filterSample * (0.2 * ((size / 2) - i));                                     // Adds tge proccessed sample to vectorSum in order to add al the samples from each buffer, and reducing the gain for each subsequent buffer.
        }

        outSample = vectSum / size;                                                                 // Divides the final output with the number of oscilators to avoid distortion. 
    
        return outSample;
    }
  
private:

    DelayLine delays[20];                                       // An array of 20 delayLine instances 
    // Initializing all the buffers into a vector of size 20. 
    std::vector <DelayLine*> delayVec{ &delays[0], &delays[1], &delays[2], &delays[3], &delays[4], &delays[5], &delays[6], &delays[7], &delays[8], &delays[9], &delays[10], &delays[11], &delays[12], &delays[13], &delays[14], &delays[15], &delays[16], &delays[17], &delays[18], &delays[19] };

    juce::IIRFilter filter[20];                                 // An array of 20 filter instances. (20 filters for 20 delayBuffers)
    // Initialiszing all the filters into a vector of size 20. 
    // *** For every delayBuffer, add a filter as well ***
    std::vector <juce::IIRFilter*> filterVec{ &filter[0], &filter[1], &filter[2], &filter[3], &filter[4], &filter[5], &filter[6], &filter[7], &filter[8], &filter[9], &filter[10], &filter[11], &filter[12], &filter[13], &filter[14], &filter[15], &filter[16], &filter[17], &filter[18], &filter[19] };

    float sampleRate;                                           // store Sample Rate
    float vectSum;                                              // store sum of the vectors.
    float outSample;                                            // store final output sample
    float size;                                                 // store size of the vectors.    
    float filterFreq;                                           // store frequency value in delayBufferFilter()
    float filterSample;                                         // store filtered sample 
    float bufferSize ;                                          // store bufferSize 
    float maxDelayLength;                                       // store Max Delay Length
    float delayLength;                                          // store Delay Length
    float feedbackVal;                                          // store feedback Value
    float filterOut;                                            // store the filtered sample in dealyBufferFilter()
   
    juce::SmoothedValue<float> smoothfilterFreq;                // Soomthed Value instance for filter frequency 

};

#endif // !MultiDelay_h

