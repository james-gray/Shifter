//
//  PhaseVocoder.cpp
//  Shifter
//
//  Created by Jordie Shier  on 2016-07-26.
//
//

#include "PhaseVocoder.hpp"

// Constructor
PhaseVocoder::PhaseVocoder(int size) : size_(size) {

    // Assuming that the block size is a power of 2 - may need to look into
    // this --
    double order = std::log2(size);
    
    // Initialize the forward and backward ffts
    fft_.reset(new FFT(order, false));
    ifft_.reset(new FFT(order, true));
    
    // Set the input buffer size to the same as the FFT size
    inputBuffer_.reset(new float[size]);
    
}


// Main body of the phase vocoder processing
void PhaseVocoder::process(AudioSampleBuffer& buffer, int channels) {

    for (int channel = 0; channel < channels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < size_; ++sample) {
            inputBuffer_[sample] = channelData[sample];
            
        }
        
    }
    
}