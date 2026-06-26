/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <JuceHeader.h>

//==============================================================================
GuitarSynthesisAudioProcessor::GuitarSynthesisAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

GuitarSynthesisAudioProcessor::~GuitarSynthesisAudioProcessor()
{
}

//==============================================================================
const juce::String GuitarSynthesisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GuitarSynthesisAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GuitarSynthesisAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GuitarSynthesisAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GuitarSynthesisAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GuitarSynthesisAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GuitarSynthesisAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GuitarSynthesisAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GuitarSynthesisAudioProcessor::getProgramName (int index)
{
    return {};
}

void GuitarSynthesisAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GuitarSynthesisAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());
	for (auto& synth : guitarSynthArray) {
		synth.prepare(spec);
	}
    limiter.prepare(spec);
    limiter.setThreshold(-2.0f); // dBFS
    limiter.setRelease(100.0f); // ms
}

void GuitarSynthesisAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GuitarSynthesisAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GuitarSynthesisAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Inject MIDI from the on-screen keyboard into the buffer
    keyboardState.processNextMidiBuffer(midiMessages,
        0,
        buffer.getNumSamples(),
        true); // true = inject + pass through
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            for (auto& guitar : guitarSynthArray)
            {
                if (!guitar.isActive())
                {
                    guitar.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
                    break;
                }
            }
        }
        else if (msg.isNoteOff())
        {
            for (auto& guitar : guitarSynthArray)
                if (guitar.currentNote() == msg.getNoteNumber())
                    guitar.noteOff();
        }
    }
    // mix all voices into the buffer
    juce::AudioBuffer<float> voiceBuffer(buffer.getNumChannels(),
        buffer.getNumSamples());
    for (auto& guitar : guitarSynthArray)
    {
        if (!guitar.isActive()) continue;

        voiceBuffer.clear();
        juce::dsp::AudioBlock<float>              block(voiceBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        guitar.process(context);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.addFrom(channel, 0, voiceBuffer, channel, 0,
                buffer.getNumSamples());

    }
    // apply limiter to the final mix
    juce::dsp::AudioBlock<float>              block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
}
    

//==============================================================================
bool GuitarSynthesisAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GuitarSynthesisAudioProcessor::createEditor()
{
    return new GuitarSynthesisAudioProcessorEditor (*this);
}

//==============================================================================
void GuitarSynthesisAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GuitarSynthesisAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GuitarSynthesisAudioProcessor();
}
