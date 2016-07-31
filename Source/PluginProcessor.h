/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include <vector>

//==============================================================================
/**
*/
class VaderizerAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    VaderizerAudioProcessor();
    ~VaderizerAudioProcessor();

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
    void adjustPhaseForPitchShift(float*, int);
    void resampleAndWindowBuffer(float*, float*, float);
    float princArg(float);

    void updatePitchShift();
    void resetPreviousPhaseData();
    
    int getNumParameters() override;
    
    const String getParameterName(int index) override;
    const String getParameterText(int index) override;
    
    float getParameter(int index) override;
    void setParameter(int index, float value) override;
    
    enum Parameters {
        coarsePitchParam,
        finePitchParam,
        numParameters
    };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaderizerAudioProcessor)
    // Array holding the samples for even-numbered windows taken from the input.
    // Since samples from more than one block will be used, we need a buffer to
    // store them before processing.


    std::shared_ptr<AudioBuffer<float>> overlapWindowBuffer_;

    // Array holding buffers for taking the FFT of overlap windows.
    std::shared_ptr<AudioBuffer<float>> overlapFftBuffer_;

    // Array holding buffers for taking the FFT of the current block.
    std::shared_ptr<AudioBuffer<float>> blockFftBuffer_;

    // Buffer to hold the Hamming window. This will be initialized in
    // prepareToPlay.
    std::shared_ptr<AudioBuffer<float>> analysisWindowFunction_;

    constexpr static const double pi_ = 3.141592653589793238462643383279502884197;

    int analysisWindowLength_;
    int resampledBufferLength_;
    int blockSize_;
    int analysisHopSize_;
    float shiftRatio_;

    float coarsePitch_;
    float finePitch_;
    float pitchShift_;
    float pitchShiftInv_;
    float actualRatio_;

    bool preparedToPlay_;


    std::shared_ptr<FFT> fft_;
    std::shared_ptr<FFT> ifft_;

    int fftSize_;

    // Vectors for storing phase information for each bin corresponding to a channel
    std::vector<std::vector<float>> prevAbsolutePhase_;
    std::vector<std::vector<float>> prevAdjustedPhase_;


    // Output buffers
    std::shared_ptr<AudioBuffer<float>> outputBuffer_;
    std::shared_ptr<AudioBuffer<float>> resampledOverlapBuffer_;
    std::shared_ptr<AudioBuffer<float>> resampledBlockBuffer_;
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
