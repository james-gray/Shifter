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
class ShifterAudioProcessorEditor  : public AudioProcessorEditor,
	private Slider::Listener
{
public:
    ShifterAudioProcessorEditor (ShifterAudioProcessor&);
    ~ShifterAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:

    ShifterAudioProcessor& processor;
    
    // GUI Components
    Slider coarsePitchSlider;
    Label coarsePitchLabel;
    Slider finePitchSlider;
    Label finePitchLabel;
    ImageComponent darthVader;
    
    // Custom font
    Font starWarsFont;

	void sliderValueChanged(Slider* slider) override;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShifterAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
