/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GuitarSynthesisAudioProcessorEditor::GuitarSynthesisAudioProcessorEditor (GuitarSynthesisAudioProcessor& p)
    :AudioProcessorEditor(&p), audioProcessor(p),
keyboardComponent(p.keyboardState,
    juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible(keyboardComponent);

    // Start receiving MIDI from the keyboard UI
    keyboardComponent.setAvailableRange(21, 108); // A0 to C8
    keyboardComponent.setOctaveForMiddleC(3);

    setSize(800, 200);
}

GuitarSynthesisAudioProcessorEditor::~GuitarSynthesisAudioProcessorEditor()
{
}

//==============================================================================
void GuitarSynthesisAudioProcessorEditor::paint (juce::Graphics& g)
{
    // fill the whole window white
    g.fillAll(juce::Colours::white);
    g.drawFittedText("Guitar Synthesis", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void GuitarSynthesisAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    keyboardComponent.setBounds(getLocalBounds().reduced(8));
}
