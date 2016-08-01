/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include <memory>


//==============================================================================
/**
*/
class VaderizerAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    VaderizerAudioProcessorEditor (VaderizerAudioProcessor&);
    ~VaderizerAudioProcessorEditor();

    //==============================================================================

    void paint (Graphics&) override;
    void resized() override;

private:
    class ParameterSlider;

    VaderizerAudioProcessor& processor;

    // GUI Components
    ScopedPointer<ParameterSlider> coarsePitchSlider, finePitchSlider;
    Label coarsePitchLabel, finePitchLabel;
    ImageComponent darthVader;

    // Custom font
    Font starWarsFont;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaderizerAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
