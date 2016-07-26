/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ShifterAudioProcessor::ShifterAudioProcessor()
{
}

ShifterAudioProcessor::~ShifterAudioProcessor()
{
}

//==============================================================================
const String ShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShifterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShifterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double ShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShifterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShifterAudioProcessor::setCurrentProgram (int index)
{
}

const String ShifterAudioProcessor::getProgramName (int index)
{
    return String();
}

void ShifterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ShifterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShifterAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void ShifterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{

		const int totalNumInputChannels = getTotalNumInputChannels();
		const int totalNumOutputChannels = getTotalNumOutputChannels();
		const int numSamples = buffer.getNumSamples();

		// In case we have more outputs than inputs, this code clears any output
		// channels that didn't contain input data, (because these aren't
		// guaranteed to be empty - they may contain garbage).
		// This is here to avoid people getting screaming feedback
		// when they first compile a plugin, but obviously you don't need to keep
		// this code if your algorithm always overwrites all the output channels.
		for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
			buffer.clear(i, 0, buffer.getNumSamples());
		}
		//buffer.clear(,,)
		//buffer.applyGain(gain);

		// This is the place where you'd normally do the guts of your plugin's
		// audio processing...

		for (int channel = 0; channel < totalNumInputChannels; ++channel)
		{
			const float* in = buffer.getReadPointer(channel, 0); //there exists getReadPointer(channel,sampleIndex)
			float* out = buffer.getWritePointer(channel);

			FFT forwardFFT(log2(numSamples), false);//(log2(buffer.getNumSamples()),false);
			FFT backwardFFT(log2(numSamples), true);//(log2(buffer.getNumSamples()), true);

													//const int thisSize = buffer.getNumSamples();

			std::vector<Complex> input;
			std::vector<Complex> output;
			std::vector<Complex> result;

			//std::array<Complex, 1> input;
			//std::array<Complex, 1> output;
			//std::array<Complex, 1> result;

			//for (int k = 0; k<buffer.getNumSamples(); k+=numSamples) { 


			for (int m = 0; m < numSamples; ++m) { // forwardFFT.getSize(); ++m) {
				Complex temp;
				temp.r = in[m];
				temp.i = 0;

				input.push_back(temp);
				output.push_back(temp);
				result.push_back(temp);
				//input[m].r = in[m];
				//input[m].i = 0;

			}

			forwardFFT.perform(input.data(), output.data()); //fft result stored in output

															 //create the complex valued result, by setting the FFT to have zero phase
			for (int m = 0; m < numSamples; ++m) {// forwardFFT.getSize(); ++m) {
				output[m].r = output[m].r; //stays the same
				output[m].i = 0;           //set imaginary part to zero 	     
			}

			backwardFFT.perform(output.data(), result.data()); //time domain result stored in result

															   //put it in the correct location in the out
			for (int m = 0; m < numSamples; ++m) {// forwardFFT.getSize(); ++m) {
				out[m] = result[m].r / (float)numSamples;	//feed the time domain samples to the correct position      
															//use aproppriate writePointer in channelData (should write 1024 times)
			}

		}

}


//==============================================================================
bool ShifterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ShifterAudioProcessor::createEditor()
{
    return new ShifterAudioProcessorEditor (*this);
}

//==============================================================================
void ShifterAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ShifterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShifterAudioProcessor();
}
