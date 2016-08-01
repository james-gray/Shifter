/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VaderizerAudioProcessor::VaderizerAudioProcessor() :
    coarsePitch_(nullptr), finePitch_(nullptr), preparedToPlay_(false)
{
    // Register the coarse pitch parameter
    addParameter(coarsePitch_ = new AudioParameterFloat(
        "coarsePitchParam",
        "Vaderization",
        NormalisableRange<float>(0, 12, 1),
        0
    ));
    
    // Register the fine pitch parameter
    addParameter(finePitch_ = new AudioParameterFloat(
        "finePitchParam",
        "Sith Factor",
        NormalisableRange<float>(-50, 50, 1),
        0
    ));
}

VaderizerAudioProcessor::~VaderizerAudioProcessor()
{
}

//==============================================================================
const String VaderizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VaderizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VaderizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double VaderizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VaderizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VaderizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VaderizerAudioProcessor::setCurrentProgram (int index)
{
}

const String VaderizerAudioProcessor::getProgramName (int index)
{
    return String();
}

void VaderizerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================

void VaderizerAudioProcessor::setParameter (int index, float newValue)
{
    // If one of the pitch parameters was changed then update pitch shift
    String paramId = getParameterID(index);
    if (paramId == "coarsePitchParam" || paramId == "finePitchParam")
    {
        updatePitchShift();
        resetPreviousPhaseData();
    }
    
    AudioProcessor::setParameter(index, newValue);
}

float VaderizerAudioProcessor::getActualCoarsePitch() {return -coarsePitch_->get();}

float VaderizerAudioProcessor::getActualFinePitch() {return finePitch_->get() / 100.0f;}

//=============================================================================

void VaderizerAudioProcessor::updatePitchShift() {
    pitchShift_ = pow(2.0, (getActualCoarsePitch() + getActualFinePitch()) / 12.0);
    actualRatio_ = round(pitchShift_ * analysisHopSize_) / analysisHopSize_;
    pitchShiftInv_ = 1/pitchShift_;
    resampledBufferLength_ = floor(pitchShiftInv_ * blockSize_);
}

void VaderizerAudioProcessor::resetPreviousPhaseData() {
    for (int i = 0; i<blockSize_; i++) {
        for (int channel=0; channel<getTotalNumInputChannels(); channel++) {
            prevAdjustedPhase_[channel][i] = 0.0;
            prevAbsolutePhase_[channel][i] = 0.0;
        }
    }
}

//==============================================================================
void VaderizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    preparedToPlay_ = false;
    //const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumInputChannels = 2;

    // Set the FFT size
    fftSize_ = std::log2(samplesPerBlock);

    analysisWindowFunction_.reset(new AudioBuffer<float>(1, samplesPerBlock));

    // Set up the FFT objects
    fft_.reset(new FFT(fftSize_, /* isInverse */ false));
    ifft_.reset(new FFT(fftSize_, /* isInverse */ true));

    // Allocate storage for FFT, overlap and window buffers.
    overlapWindowBuffer_.reset(new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock));
    overlapFftBuffer_.reset(new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2));
    blockFftBuffer_.reset(new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 2));

    // Allocate storage for output buffers.
    resampledOverlapBuffer_.reset(new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 3));
    resampledBlockBuffer_.reset(new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 3));

    outputBuffer_.reset(new AudioBuffer<float>(totalNumInputChannels, samplesPerBlock * 5));

    // Initial pitch adjustment ratio
    analysisHopSize_ = samplesPerBlock / 2;

    //zero out
    outputBuffer_->clear();
    overlapWindowBuffer_->clear();
    overlapFftBuffer_->clear();
    blockFftBuffer_->clear();
    resampledBlockBuffer_->clear();
    resampledOverlapBuffer_->clear();

    // Set up the Hann window buffer
    analysisWindowLength_ = blockSize_ = samplesPerBlock;

    for (int i = 0; i < analysisWindowLength_; ++i) {
        analysisWindowFunction_->setSample(0, i, 0.5 * (1.0 - cos(2.0 * pi_ * (float) i / analysisWindowLength_)));
    }

    // 2D Array for storing phase from previous block for each channel, initialize to 0
    for (int i = 0; i < totalNumInputChannels; ++i) {
        prevAbsolutePhase_.emplace_back(blockSize_, 0.0);
        prevAdjustedPhase_.emplace_back(blockSize_, 0.0);
    }

    // Update the pitch shift values
    updatePitchShift();

    preparedToPlay_ = true;
}

void VaderizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    preparedToPlay_ = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VaderizerAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
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

void VaderizerAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    if (!preparedToPlay_) {
        buffer.clear();
        return;
    }

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
        resampleAndWindowBuffer(overlapFft, resampledOverlapBuffer, resampledBufferLength_);
        resampleAndWindowBuffer(blockFft, resampledBlockBuffer, resampledBufferLength_);

        // Write the resampled overlap and block data to the output buffer
        for (int i = 0, j = analysisHopSize_; i < resampledBufferLength_; ++i, ++j) {
            outputBuffer[i] += resampledOverlapBuffer[i];
            outputBuffer[j] += resampledBlockBuffer[i];
        }

        // Clear the resampled buffers for this channel
        resampledOverlapBuffer_->clear(channel, 0, resampledOverlapBuffer_->getNumSamples());
        resampledBlockBuffer_->clear(channel, 0, resampledBlockBuffer_->getNumSamples());

        // Output the samples for this block to the channel stream.
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] = outputBuffer[i];
        }

        // Save any output buffer samples that we have not yet output into the first
        // part of the output buffer, so they will be output on the next block after
        // additional output is processed and added to them.
        int samplesToOutput = resampledBufferLength_ + analysisHopSize_ - numSamples;
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


void VaderizerAudioProcessor::adjustPhaseForPitchShift(float* fft, int channel) {
    for (int i = 0, fftIndex = 0; i < blockSize_; ++i, fftIndex = i * 2) {
        // Extract real and imaginary components from the fft
        float re = fft[fftIndex];
        float im = fft[fftIndex + 1];

        // Compute amplitude and phase
        float amplitude = sqrt((re * re) + (im * im));
        float phase = atan2(im, re);

        // Calculate the frequency of this bin
        float frequency = 2.0 * pi_ * static_cast<float>(i) * analysisHopSize_ / blockSize_;

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

void VaderizerAudioProcessor::resampleAndWindowBuffer(float* inBuffer, float* outBuffer, float outputLength) {
    for (int i = 0; i < outputLength; i++) {
        // Get the fractional index of the sample in question
        float sample = (i * blockSize_) / outputLength;

        // Get the integer indices of the samples around our fractional sample
        int prevSample = floor(sample);
        int nextSample = (prevSample + 1) % blockSize_;

        // Calculate the fractional component
        float frac = sample - (float)prevSample;

        // Perform linear interpolation on the two integer indices
        outBuffer[i] = (inBuffer[prevSample] * (1.0 - frac)) + (inBuffer[nextSample] * frac);

        // Window the buffer if any transposition is being applied.
        // If we aren't transposing at all it seems to be much higher quality
        // without windowing.
        if (!(static_cast<int>(getActualCoarsePitch()) == 0 &&
                static_cast<int>(getActualFinePitch()) == 0)) {

            outBuffer[i] *= 0.5 * (1.0 - cos(2.0 * pi_ * (float) i / outputLength));
        }
    }
}

// Principal argument
float VaderizerAudioProcessor::princArg(float phase)
{
    if (phase >= 0) {
        return std::fmod(phase + pi_, 2 * pi_) - pi_;
    } else {
        return std::fmod(phase + pi_, -2 * pi_) + pi_;
    }
}

//==============================================================================
bool VaderizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* VaderizerAudioProcessor::createEditor()
{
    return new VaderizerAudioProcessorEditor (*this);
}

//==============================================================================
void VaderizerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml ("VADERIZER_SETTINGS");
    
    xml.setAttribute("coarsePitch", coarsePitch_->get());
    xml.setAttribute("finePitch", finePitch_->get());
    
    // Store the values of all our parameters, using their param ID as the XML attribute
    for (int i = 0; i < getNumParameters(); ++i)
        if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
            xml.setAttribute (p->paramID, p->getValue());
    
    // Save XML as binary
    copyXmlToBinary (xml, destData);
}

void VaderizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Load parameters from saved xml state
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName ("VADERIZER_SETTINGS"))
        {
            xmlState->getIntAttribute("coarsePitch", *coarsePitch_);
            xmlState->getIntAttribute("finePitch", *finePitch_);

            // Reload parameters
            for (int i = 0; i < getNumParameters(); ++i)
                if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
                    p->setValueNotifyingHost ((float) xmlState->getDoubleAttribute (p->paramID, p->getValue()));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VaderizerAudioProcessor();
}
