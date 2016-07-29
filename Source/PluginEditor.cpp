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
    setSize (300, 165);
    
    // Create a new font to use for title
    Typeface::Ptr starFont = Typeface::createSystemTypefaceFor(BinaryData::Starjedi_ttf, BinaryData::Starjedi_ttfSize);
    starWarsFont = Font(starFont);
    
    // Set the coarse pitch rotary
    coarsePitch.setSliderStyle(Slider::RotaryVerticalDrag);
    coarsePitch.setRange(-12.0, 12.0, 1.0);
    coarsePitch.setTextBoxStyle(Slider::NoTextBox, true, 90, 0);
    coarsePitch.setPopupDisplayEnabled(true, this);
    coarsePitch.setTextValueSuffix(" Semitones");
    coarsePitch.setColour(0x1001311, Colours::black);
    coarsePitch.setValue(0.0);
    
    // Label for coarse pitch
    coarsePitchLabel.setFont(starWarsFont);
    coarsePitchLabel.setJustificationType(Justification(12));
    coarsePitchLabel.setText(String("coarse"), dontSendNotification);
    coarsePitchLabel.attachToComponent(&coarsePitch, false);
    
    // Set the fine pitch rotary
    finePitch.setSliderStyle(Slider::RotaryVerticalDrag);
    finePitch.setRange(-50.0, 50.0, 1.0);
    finePitch.setTextBoxStyle(Slider::NoTextBox, true, 90, 0);
    finePitch.setPopupDisplayEnabled(true, this);
    finePitch.setTextValueSuffix(" Cents");
    finePitch.setColour(0x1001311, Colours::black);
    finePitch.setValue(0.0);
    
    // Label for fine pitch
    finePitchLabel.setFont(starWarsFont);
    finePitchLabel.setJustificationType(Justification(12));
    finePitchLabel.setText(String("fine"), dontSendNotification);
    finePitchLabel.attachToComponent(&finePitch, false);
    
    // Setup the Darth Vader image
    Image vaderImage = ImageFileFormat::loadFrom(
        BinaryData::dv_helmet_png, static_cast<size_t>(BinaryData::dv_helmet_pngSize)
    );
    darthVader.setImage(vaderImage);
    darthVader.setAlpha(0.25);    
    
    // Add components to GUI
    addAndMakeVisible(&darthVader);
    addAndMakeVisible(&coarsePitch);
    addAndMakeVisible(&coarsePitchLabel);
    addAndMakeVisible(&finePitch);
    addAndMakeVisible(&finePitchLabel);
}

ShifterAudioProcessorEditor::~ShifterAudioProcessorEditor()
{
}

//==============================================================================
void ShifterAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::darkgrey);

    g.setColour (Colours::black);
    
    // Write title
    g.setFont(starWarsFont);
    g.setFont(60.0f);
    g.drawText(String("vaderizer"), 0, 0, 300, 300, Justification(12));
}

void ShifterAudioProcessorEditor::resized()
{
    // Position components on screen
    darthVader.setBounds(0, 10, 300, 150);
    coarsePitch.setBounds(50, 80, 75, getHeight() - 80);
    finePitch.setBounds(180, 80, 75, getHeight() - 80);
}
