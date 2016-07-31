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


//==============================================================================
/**
*/
class VaderizerAudioProcessorEditor  : public AudioProcessorEditor,
	public Slider::Listener, public Timer
{
public:
    VaderizerAudioProcessorEditor (VaderizerAudioProcessor&);
    ~VaderizerAudioProcessorEditor();

    //==============================================================================
    // Update GUI parameters on timer callback (make sure UI reflects internal state)
    void timerCallback() override;

    void paint (Graphics&) override;
    void resized() override;

private:

    VaderizerAudioProcessor& processor;

    // GUI Components
    Slider coarsePitchSlider;
    Label coarsePitchLabel;
    Slider finePitchSlider;
    Label finePitchLabel;
    ImageComponent darthVader;

    // Custom font
    Font starWarsFont;

    // Update internal parameters on GUI change
	void sliderValueChanged(Slider* slider) override;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaderizerAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
