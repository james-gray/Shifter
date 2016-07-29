/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ShifterAudioProcessor::ShifterAudioProcessor() :
    overlapWindowBuffer_(nullptr), overlapFftBuffer_(nullptr),
    blockFftBuffer_(nullptr), analysisWindowFunction_(nullptr),
    synthesisWindowFunction_(nullptr), fft_(nullptr),
    ifft_(nullptr), outputBuffer_(nullptr),
    resampledOverlapBuffer_(nullptr), resampledBlockBuffer_(nullptr)
{
}

ShifterAudioProcessor::~ShifterAudioProcessor()
{
    delete outputBuffer_;
    delete ifft_;
    delete fft_;
    delete analysisWindowFunction_;
    delete synthesisWindowFunction_;
    delete blockFftBuffer_;
    delete overlapFftBuffer_;
    delete overlapWindowBuffer_;
    delete resampledOverlapBuffer_;
    delete resampledBlockBuffer_;

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

    analysisWindowFunction_ = new AudioBuffer<float>(1, samplesPerBlock);
    synthesisWindowFunction_ = new AudioBuffer<float>(1, samplesPerBlock * 2);

    // Set up the FFT objects
    fft_ = new FFT(fftSize_, /* isInverse */ false);
    ifft_ = new FFT(fftSize_, /* isInverse */ true);

    // Allocate storage for FFT, overlap and window buffers.
    overlapWindowBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock);
    overlapFftBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2);
    blockFftBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2);

    // Allocate storage for output buffers.
    resampledOverlapBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2);
    resampledBlockBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2);

    outputBuffer_ = new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 4);

    // Initial pitch adjustment ratio
    analysisHopSize_ = samplesPerBlock / 2;
    pitchShift_ = pow(2.0, -12.0/12.0);
    actualRatio_ = round(pitchShift_ * analysisHopSize_) / analysisHopSize_;
    pitchShiftInv_ = 1/pitchShift_;

    //zero out
    outputBuffer_->clear();
    overlapWindowBuffer_->clear();
    overlapFftBuffer_->clear();
    blockFftBuffer_->clear();

    // Set up the Hamming window buffer
    analysisWindowLength_ = blockSize_ = samplesPerBlock;
    synthesisWindowLength_ = floor(pitchShiftInv_ * blockSize_);
    for (int i = 0; i < analysisWindowLength_; ++i) {
        analysisWindowFunction_->setSample(0, i, 0.54 - 0.46 * cos(2.0 * M_PI * (float) i / analysisWindowLength_));
    }
    for (int i = 0; i < synthesisWindowLength_; ++i) {
        synthesisWindowFunction_->setSample(0, i, 0.54 - 0.46 * cos(2.0 * M_PI * (float) i / synthesisWindowLength_));
    }

    // 2D Array for storing phase from previous block for each channel, initialize to 0
    for (int i = 0; i < totalNumInputChannels; ++i) {
        prevAbsolutePhase_.emplace_back(blockSize_, 0.0);
        prevAdjustedPhase_.emplace_back(blockSize_, 0.0);
    }
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
        float* outputBuffer = outputBuffer_->getWritePointer(channel);
        float* resampledOverlapBuffer = resampledOverlapBuffer_->getWritePointer(channel);
        float* resampledBlockBuffer = resampledBlockBuffer_->getWritePointer(channel);

        const float* analysisWindowFunction = analysisWindowFunction_->getReadPointer(0);
        const float* synthesisWindowFunction = synthesisWindowFunction_->getReadPointer(0);

        // Store the first half of this block in the second half of the overlap
        // buffer. This will ensure the overlap buffer is full.
        for (int i = 0; i < analysisHopSize_; ++i) {
            overlapBuffer[i + analysisHopSize_] = channelData[i];
        }

        // Apply the window function to the current block AND to the overlap
        // buffer, before staging these buffers' contents in their respective
        // FFTs.
        for (int i = 0; i < numSamples; ++i) {
            overlapFft[i] = overlapBuffer[i] * analysisWindowFunction[i];
            blockFft[i] = channelData[i] * analysisWindowFunction[i];
        }

        // Store the second half of the current block in the overlap buffer to
        // be processed in the next iteration of processBlock.
        for (int i = 0; i < analysisHopSize_; ++i) {
            overlapBuffer[i] = channelData[i + analysisHopSize_];
        }

        // Take the FFT of the overlap buffer AND the current block.
        fft_->performRealOnlyForwardTransform(overlapFft);
        fft_->performRealOnlyForwardTransform(blockFft);

        // **************************
        // * PHASE PROCESSING STAGE *
        // **************************
        adjustPhaseForPitchShift(overlapFft, channel);
        adjustPhaseForPitchShift(blockFft, channel);

        // ****************
        // * OUTPUT STAGE *
        // ****************
        ifft_->performRealOnlyInverseTransform(overlapFft);
        ifft_->performRealOnlyInverseTransform(blockFft);

        // Resample and write the phase-adjusted overlap and block data to their buffers
        resampleBuffer(overlapFft, resampledOverlapBuffer, synthesisWindowLength_);
        resampleBuffer(blockFft, resampledBlockBuffer, synthesisWindowLength_);

        // Write the resampled overlap and block data to the output buffer
        for (int i = 0, j = analysisHopSize_; i < synthesisWindowLength_; ++i, ++j) {
            outputBuffer[i] += resampledOverlapBuffer[i] * synthesisWindowFunction[i];
            outputBuffer[j] += resampledBlockBuffer[i] * synthesisWindowFunction[i];
        }

        // Clear the resampled buffers for this channel
        resampledOverlapBuffer_->clear(channel, 0, synthesisWindowLength_);
        resampledBlockBuffer_->clear(channel, 0, synthesisWindowLength_);

        // Output the samples for this block to the channel stream.
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] = outputBuffer[i];
        }
        
        // Save any output buffer samples that we have not yet output into the first
        // part of the output buffer, so they will be output on the next block after
        // additional output is processed and added to them.
        int samplesToOutput = synthesisWindowLength_ + analysisHopSize_ - numSamples;
        for (int i = 0; i < numSamples * 4; i++) {
            if (i < samplesToOutput) {
                outputBuffer[i] = outputBuffer[i + numSamples];
            } else {
                outputBuffer[i] = 0.0;
            }
        }

        // Clear the FFT buffers to remove FFT garbage
        overlapFftBuffer_->clear();
        blockFftBuffer_->clear();
    }
}


void ShifterAudioProcessor::adjustPhaseForPitchShift(float* fft, int channel) {
    for (int i = 0, fftIndex = 0; i < blockSize_; ++i, fftIndex = i * 2) {
        // Extract real and imaginary components from the fft
        float re = fft[fftIndex];
        float im = fft[fftIndex + 1];

        // Compute amplitude and phase
        float amplitude = sqrt((re * re) + (im * im));
        float phase = atan2(im, re);

        // Calculate the frequency of this bin
        float frequency = 2.0 * M_PI * static_cast<float>(i) * analysisHopSize_ / blockSize_;

        // Calculate the phase deviation for this hop
        float deviationPhase = frequency + princArg(phase - prevAbsolutePhase_[channel][i] - frequency);

        // Store the previous absolute and adjusted phases for the next hop
        prevAbsolutePhase_[channel][i] = phase;
        prevAdjustedPhase_[channel][i] = princArg(prevAdjustedPhase_[channel][i] +
            (deviationPhase * actualRatio_));

        // Convert back to real/imaginary form
        fft[fftIndex] = amplitude * cos(prevAdjustedPhase_[channel][i]);
        fft[fftIndex + 1] = amplitude * sin(prevAdjustedPhase_[channel][i]);
    }
}

void ShifterAudioProcessor::resampleBuffer(float* inBuffer, float* outBuffer, float outputLength) {
    for (int i = 0; i < outputLength; i++) {
        // Get the fractional index of the sample in question
        float sample = i * blockSize_ / outputLength;

        // Get the integer indices of the samples around our fractional sample
        int prevSample = floor(sample);
        int nextSample = prevSample + 1;

        // Calculate the fractional component
        float frac = sample - (float)prevSample;

        // Perform linear interpolation on the two integer indices
        outBuffer[i] = (inBuffer[prevSample] * (1.0 - frac));
        if (nextSample < blockSize_) {
            outBuffer[i] += inBuffer[nextSample] * frac;
        }
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
