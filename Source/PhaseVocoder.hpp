//
//  PhaseVocoder.hpp
//  Shifter
//
//  Created by Jordie Shier  on 2016-07-26.
//
//

#ifndef PhaseVocoder_hpp
#define PhaseVocoder_hpp

#include <stdio.h>
#include <memory>
#include "../JuceLibraryCode/JuceHeader.h"


// Phase Vocoder Processing Class
class PhaseVocoder {
public:
    
    // Constructor
    PhaseVocoder(int);
    
    // Do processing on an input buffer
    void process(AudioSampleBuffer&, int);
    
private:
    
    int size_;
    
    // Smart pointers to hold FFT classes and input buffer
    std::unique_ptr<FFT> fft_;
    std::unique_ptr<FFT> ifft_;
    std::unique_ptr<float[]> inputBuffer_;
};

#endif /* PhaseVocoder_hpp */
