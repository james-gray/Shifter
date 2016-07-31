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
VaderizerAudioProcessorEditor::VaderizerAudioProcessorEditor (VaderizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (300, 165);

    // Create a new font to use for title
    Typeface::Ptr starFont = Typeface::createSystemTypefaceFor(BinaryData::Starjedi_ttf, BinaryData::Starjedi_ttfSize);
    starWarsFont = Font(starFont);

    // Set the coarse pitch rotary
    coarsePitchSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    coarsePitchSlider.setRange(0, 12.0, 1.0);
    coarsePitchSlider.setTextBoxStyle(Slider::NoTextBox, true, 120, 0);
    coarsePitchSlider.setPopupDisplayEnabled(true, this);
    coarsePitchSlider.setTextValueSuffix(" Semitones");
    coarsePitchSlider.setColour(0x1001311, Colours::black);
    coarsePitchSlider.setValue(0.0);

    // Label for coarse pitch
    coarsePitchLabel.setFont(starWarsFont);
    coarsePitchLabel.setJustificationType(Justification(12));
    coarsePitchLabel.setText(String("vaderization"), dontSendNotification);
    coarsePitchLabel.attachToComponent(&coarsePitchSlider, false);

    // Set the fine pitch rotary
    finePitchSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    finePitchSlider.setRange(-50.0, 50.0, 1.0);
    finePitchSlider.setTextBoxStyle(Slider::NoTextBox, true, 120, 0);
    finePitchSlider.setPopupDisplayEnabled(true, this);
    finePitchSlider.setTextValueSuffix(" Cents");
    finePitchSlider.setColour(0x1001311, Colours::black);
    finePitchSlider.setValue(0.0);

    // Label for fine pitch
    finePitchLabel.setFont(starWarsFont);
    finePitchLabel.setJustificationType(Justification(12));
    finePitchLabel.setText(String("sith factor"), dontSendNotification);
    finePitchLabel.attachToComponent(&finePitchSlider, false);

    // Setup the Darth Vader image
    Image vaderImage = ImageFileFormat::loadFrom(
        BinaryData::dv_helmet_png, static_cast<size_t>(BinaryData::dv_helmet_pngSize)
    );
    darthVader.setImage(vaderImage);
    darthVader.setAlpha(0.25);

    // Add components to GUI
    addAndMakeVisible(&darthVader);
	coarsePitchSlider.addListener(this);

    addAndMakeVisible(&coarsePitchSlider);
    addAndMakeVisible(&coarsePitchLabel);

	finePitchSlider.addListener(this);
    addAndMakeVisible(&finePitchSlider);
    addAndMakeVisible(&finePitchLabel);
    
    // Setup callback timer
    startTimer(50);
}

VaderizerAudioProcessorEditor::~VaderizerAudioProcessorEditor()
{
}

//==============================================================================
void VaderizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::darkgrey);

    g.setColour (Colours::black);

    // Write title
    g.setFont(starWarsFont);
    g.setFont(60.0f);
    g.drawText(String("vaderizer"), 0, 0, 300, 300, Justification(12));
}

void VaderizerAudioProcessorEditor::resized()
{
    // Position components on screen
    darthVader.setBounds(0, 10, 300, 150);
    coarsePitchSlider.setBounds(50, 80, 75, getHeight() - 80);
    finePitchSlider.setBounds(180, 80, 75, getHeight() - 80);
}

void VaderizerAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
	if (slider == &coarsePitchSlider) {
        processor.setParameterNotifyingHost(VaderizerAudioProcessor::coarsePitchParam,
                                            static_cast<float>(-coarsePitchSlider.getValue()));
	}
	else if (slider == &finePitchSlider) {
        processor.setParameterNotifyingHost(VaderizerAudioProcessor::finePitchParam,
                                            static_cast<float>(finePitchSlider.getValue() / 100.0f));
	}
}

void VaderizerAudioProcessorEditor::timerCallback()
{
    coarsePitchSlider.setValue(-processor.getParameter(VaderizerAudioProcessor::coarsePitchParam));
    finePitchSlider.setValue(processor.getParameter(VaderizerAudioProcessor::finePitchParam) * 100.0f);
}
