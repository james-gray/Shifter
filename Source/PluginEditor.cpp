/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

//==============================================================================
ShifterAudioProcessorEditor::ShifterAudioProcessorEditor (ShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (300, 165);
    
    // Set the coarse pitch rotary
    coarsePitch.setSliderStyle(Slider::RotaryVerticalDrag);
    coarsePitch.setRange(-12.0, 12.0, 1.0);
    coarsePitch.setTextBoxStyle(Slider::NoTextBox, true, 90, 0);
    coarsePitch.setPopupDisplayEnabled(true, this);
    coarsePitch.setTextValueSuffix(" Semitones");
    coarsePitch.setColour(0x1001311, Colours::black);
    coarsePitch.setValue(0.0);
    
    // Set the fine pitch rotary
    finePitch.setSliderStyle(Slider::RotaryVerticalDrag);
    finePitch.setRange(-50.0, 50.0, 1.0);
    finePitch.setTextBoxStyle(Slider::NoTextBox, true, 90, 0);
    finePitch.setPopupDisplayEnabled(true, this);
    finePitch.setTextValueSuffix(" Cents");
    finePitch.setColour(0x1001311, Colours::black);
    finePitch.setValue(0.0);
    
    Image darthVader = ImageFileFormat::loadFrom(
        BinaryData::dv_helmet_png, static_cast<size_t>(BinaryData::dv_helmet_pngSize)
    );
    logo.setImage(darthVader);
    
    // Add the two rotaries to the GUI
    addAndMakeVisible(&coarsePitch);
    addAndMakeVisible(&finePitch);
    addAndMakeVisible(&logo);
}

ShifterAudioProcessorEditor::~ShifterAudioProcessorEditor()
{
}

//==============================================================================
void ShifterAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::darkgrey);

    g.setColour (Colours::black);
    g.setFont (15.0f);
}

void ShifterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    coarsePitch.setBounds(65, 40, 75, getHeight() - 25);
    finePitch.setBounds(170, 40, 50, getHeight() - 25);
    logo.setBounds(0, 0, 300, 60);
}
