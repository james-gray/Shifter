/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

// Custom slider class that recieves a AudioParameterFloat object and communicates
// directly with that object when any parameters change
class VaderizerAudioProcessorEditor::ParameterSlider : public Slider, public Timer
{
public:
    // Constructor, takes in a parameter object
    ParameterSlider(AudioParameterFloat& p) :
        Slider(), param(p)
    {
        setRange(param.range.start, param.range.end, param.range.interval);
        updateSliderPos();
        startTimer(50);
    }
 
    // Value changed on UI
    void valueChanged() override
    {
        float newValue = param.range.convertTo0to1((float)Slider::getValue());
        param.setValueNotifyingHost(newValue);
    }
    
    // Update UI from internal values
    void updateSliderPos()
    {
        const float newValue = param.get();
        
        if (newValue != (float) Slider::getValue() && ! isMouseButtonDown())
            Slider::setValue (newValue);
    }

    // Periodically update the UI from internal values
    void timerCallback() override
    {
        updateSliderPos();
    }
    
    AudioParameterFloat& param;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
};

//==============================================================================
VaderizerAudioProcessorEditor::VaderizerAudioProcessorEditor (VaderizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Create a new font to use for title
    Typeface::Ptr starFont = Typeface::createSystemTypefaceFor(BinaryData::Starjedi_ttf, BinaryData::Starjedi_ttfSize);
    starWarsFont = Font(starFont);
    
    // Setup the Darth Vader image
    addAndMakeVisible(&darthVader);
    Image vaderImage = ImageFileFormat::loadFrom(
        BinaryData::dv_helmet_png, static_cast<size_t>(BinaryData::dv_helmet_pngSize)
    );
    darthVader.setImage(vaderImage);
    darthVader.setAlpha(0.25);
    
    // Set the coarse pitch rotary
    coarsePitchSlider = new ParameterSlider(*processor.coarsePitch_);
    addAndMakeVisible(coarsePitchSlider);
    coarsePitchSlider->setSliderStyle(Slider::RotaryVerticalDrag);
    coarsePitchSlider->setTextBoxStyle(Slider::NoTextBox, true, 120, 0);
    coarsePitchSlider->setPopupDisplayEnabled(true, this);
    coarsePitchSlider->setTextValueSuffix(" Semitones");
    coarsePitchSlider->setColour(0x1001311, Colours::black);

    // Label for coarse pitch
    addAndMakeVisible(&coarsePitchLabel);
    coarsePitchLabel.setFont(starWarsFont);
    coarsePitchLabel.setJustificationType(Justification(12));
    coarsePitchLabel.setText(String("vaderization"), dontSendNotification);
    coarsePitchLabel.attachToComponent(coarsePitchSlider, false);

    // Set the fine pitch rotary
    finePitchSlider = new ParameterSlider(*processor.finePitch_);
    addAndMakeVisible(finePitchSlider);
    finePitchSlider->setSliderStyle(Slider::RotaryVerticalDrag);
    finePitchSlider->setTextBoxStyle(Slider::NoTextBox, true, 120, 0);
    finePitchSlider->setPopupDisplayEnabled(true, this);
    finePitchSlider->setTextValueSuffix(" Cents");
    finePitchSlider->setColour(0x1001311, Colours::black);

    // Label for fine pitch
    addAndMakeVisible(&finePitchLabel);
    finePitchLabel.setFont(starWarsFont);
    finePitchLabel.setJustificationType(Justification(12));
    finePitchLabel.setText(String("sith factor"), dontSendNotification);
    finePitchLabel.attachToComponent(finePitchSlider, false);
    
    setSize (300, 165);

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
    coarsePitchSlider->setBounds(50, 80, 75, getHeight() - 80);
    finePitchSlider->setBounds(180, 80, 75, getHeight() - 80);
    darthVader.setBounds(0, 10, 300, 150);
}