/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/
class ShifterAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    ShifterAudioProcessor();
    ~ShifterAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;
    void convertToPolar(float*, int);

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    int fftGetSize() {return fftSize_;}
    void initialize2DPhaseArray(float**&);
    void delete2DPhaseArray(float**&);
    void adjustPhaseForPitchShift(float*, int);
    float princArg(float);
    
    
    

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShifterAudioProcessor)
    // Array holding the samples for even-numbered windows taken from the input.
    // Since samples from more than one block will be used, we need a buffer to
    // store them before processing.
    
    
    AudioBuffer<float>* overlapWindowBuffer_;

    // Array holding buffers for taking the FFT of overlap windows.
    AudioBuffer<float>* overlapFftBuffer_;

    // Array holding buffers for taking the FFT of the current block.
    AudioBuffer<float>* blockFftBuffer_;

    // Buffer to hold the Hamming window. This will be initialized in
    // prepareToPlay.
    AudioBuffer<float>* windowFunction_;

    int windowLength_;
    int analysisHopSize_;
    float shiftRatio_;

    FFT* fft_;
    FFT* ifft_;

    int fftSize_;
    
    // Frequency Domain Variables
    float** prevAbsolutePhase_;
    float** prevAdjustedPhase_;

    
    // Output buffer
    AudioBuffer<float>* outputBuffer_;
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
