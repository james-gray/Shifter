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

class PhaseVocoder {
public:

    PhaseVocoder(int size) :
            fft_(std::make_unique<FFT>(size, false)), ifft_(std::make_unique<FFT>(size, true)) {};

    
    // Do in place phase vocoder 
    void operator() (float*);
    

private:
    
    // Smart pointers to forward and inverse FFTs
    std::unique_ptr<FFT> fft_;
    std::unique_ptr<FFT> ifft_;
};

#endif /* PhaseVocoder_hpp */
