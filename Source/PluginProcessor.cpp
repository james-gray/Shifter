/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ShifterAudioProcessor::ShifterAudioProcessor() : fft_(nullptr), ifft_(nullptr)
{
}

ShifterAudioProcessor::~ShifterAudioProcessor()
{
    delete fft_;
    delete ifft_;
    delete2DPhaseArray(prevAdjustedPhase_);
    delete2DPhaseArray(prevAbsolutePhase_);
}

//==============================================================================
const String ShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShifterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShifterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double ShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShifterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShifterAudioProcessor::setCurrentProgram (int index)
{
}

const String ShifterAudioProcessor::getProgramName (int index)
{
    return String();
}

void ShifterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int totalNumInputChannels = getTotalNumInputChannels();

    // Set the FFT size
    fftSize_ = std::log2(samplesPerBlock);

    // Set up the FFT objects
    fft_ = new FFT(fftSize_, /* isInverse */ false);
    ifft_ = new FFT(fftSize_, /* isInverse */ true);

    // Allocate storage for FFT, overlap and window buffers.
    overlapWindowBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock);
    overlapFftBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2);
    blockFftBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2);
    
    windowFunction_ = new AudioBuffer<float>(1, samplesPerBlock);

    // Initial pitch adjustment ratio
    analysisHopSize_ = samplesPerBlock / 2;
    shiftRatio_ = 1.0;
    
    //zero out
    overlapWindowBuffer_->clear();
    overlapFftBuffer_->clear();
    blockFftBuffer_->clear();
    
    // Set up the Hamming window buffer
    windowLength_ = samplesPerBlock;
    for (int i = 0; i < windowLength_; ++i) {
        windowFunction_->setSample(0, i, 0.54 - 0.46 * cos(2.0 * M_PI * (double) i / windowLength_));
    }
    
    // 2D Array for storing phase from previous block for each channel,
    // initialize to 0
    // TODO: Create deconstructors for these
    initialize2DPhaseArray(prevAbsolutePhase_);
    initialize2DPhaseArray(prevAdjustedPhase_);
}

void ShifterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShifterAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void ShifterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear (i, 0, numSamples);
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {

        // ***************
        // * INPUT STAGE *
        // ***************

        // Raw samples from the current block.
        float* channelData = buffer.getWritePointer(channel);

        // Raw pointers to the overlap buffer and FFT buffers for the current
        // channel.
        float* overlapBuffer = overlapWindowBuffer_->getWritePointer(channel);
        float* overlapFft = overlapFftBuffer_->getWritePointer(channel);
        float* blockFft = blockFftBuffer_->getWritePointer(channel);
        const float* windowFunction = windowFunction_->getReadPointer(0);

        // Store the first half of this block in the second half of the overlap
        // buffer. This will ensure the overlap buffer is full.
        for (int i = 0; i < numSamples / 2; ++i) {
            overlapBuffer[i + (numSamples / 2)] = channelData[i];
        }

        // Apply the window function to the current block AND to the overlap
        // buffer, before staging these buffers' contents in their respective
        // FFTs.
        for (int i = 0; i < windowLength_; ++i) {
            overlapFft[i] = overlapBuffer[i] * windowFunction[i];
            blockFft[i] = channelData[i] * windowFunction[i];
        }

        // Take the FFT of the overlap buffer AND the current block.
        fft_->performRealOnlyForwardTransform(overlapFft);
        fft_->performRealOnlyForwardTransform(blockFft);

        // Store the second half of the current block in the overlap buffer to
        // be processed in the next iteration of processBlock.
        for (int i = 0; i < numSamples / 2; ++i) {
            overlapBuffer[i] = channelData[i + (numSamples / 2)];
        }

        // **************************
        // * PHASE PROCESSING STAGE *
        // **************************
        adjustPhaseForPitchShift(overlapFft, channel);
        adjustPhaseForPitchShift(blockFft, channel);

        // ****************
        // * OUTPUT STAGE *
        // ****************
        // TODO
    }
}

void ShifterAudioProcessor::initialize2DPhaseArray(float**& array) {
    const int totalNumInputChannels = getTotalNumInputChannels();

    array = new float*[totalNumInputChannels];
    for (int i = 0; i < totalNumInputChannels; ++i) {
        array[i] = new float[windowLength_];
        for (int j = 0; j < windowLength_; j++) {
            array[i][j] = 0;
        }
    }
}

void ShifterAudioProcessor::delete2DPhaseArray(float**& array) {
    for(int i = 0; i < getTotalNumInputChannels(); ++i) {
        delete [] array[i];
    }
    delete [] array;
}

void ShifterAudioProcessor::adjustPhaseForPitchShift(float* fft, int channel) {
    for (int i = 0; i < windowLength_ * 2; i += 2) {
        float re = fft[i];
        float im = fft[i + 1];
        float amplitude = sqrt((re * re) + (im * im));
        float phase = atan2(im, re);

        // Calculate the frequency of this bin
        float frequency = 2.0 * M_PI * (static_cast<float>(i) / windowLength_);
        float normFrequency = (frequency * analysisHopSize_);

        // Calculate the phase deviation for this hop
        float deviationPhase = (normFrequency + princArg(phase - prevAbsolutePhase_[channel][i] - normFrequency));

        // Store the previous absolute and adjusted phases for the next hop
        prevAbsolutePhase_[channel][i] = phase;
        prevAdjustedPhase_[channel][i] = princArg(prevAdjustedPhase_[channel][i] +
            deviationPhase * shiftRatio_ * analysisHopSize_);

        // Convert back to real/imaginary form
        fft[i] = amplitude * cos(prevAdjustedPhase_[channel][i]);
        fft[i + 1] = amplitude * sin(prevAdjustedPhase_[channel][i]);
    }
}

// Principal argument
float ShifterAudioProcessor::princArg(float phase)
{
    if (phase >= 0) {
        return std::fmod(phase + M_PI, 2 * M_PI) - M_PI;
    } else {
        return std::fmod(phase + M_PI, -2 * M_PI) + M_PI;
    }
}

//==============================================================================
bool ShifterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ShifterAudioProcessor::createEditor()
{
    return new ShifterAudioProcessorEditor (*this);
}

//==============================================================================
void ShifterAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ShifterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShifterAudioProcessor();
}
