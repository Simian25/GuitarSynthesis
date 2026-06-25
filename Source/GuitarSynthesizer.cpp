#include "GuitarSynthesizer.h"


//GuitarSynthesizer
void GuitarSynthesizer::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    gain.prepare(spec);
    gain.setGainLinear(0.0f);
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(sampleRate*10);
}

void GuitarSynthesizer::noteOn(int midiNote, float velocity)
{   
    if (!isprocessing){
        numberOfWhiteNoiseSamples = (sampleRate / juce::MidiMessage::getMidiNoteInHertz(midiNote));
        delayLine.setDelay(numberOfWhiteNoiseSamples);
        gain.setGainLinear(velocity * 0.8f);
        whitenoise.reset();
        whitenoise.setMaxSamples(numberOfWhiteNoiseSamples);
        active = true;
    }
    else {
        juce::ignoreUnused(midiNote);
    }

    
}

void GuitarSynthesizer::noteOff()
{
    gain.setGainLinear(0.0f);
    active = false;
    delayLine.reset();
    lpFilter.reset();
}
void GuitarSynthesizer::reset() {
    gain.reset();
    delayLine.reset();
    active = false;
}
void GuitarSynthesizer::process(const juce::dsp::ProcessContextReplacing<float>& context)
{

    
    if (!active) return;
    auto& block = context.getOutputBlock();
    for (int sample = 0; sample < (int)block.getNumSamples(); ++sample)
    {   

        float noise = whitenoise.nextSample(); // generate once
        auto nextOutputSample = delayLine.popSample(1);
        lpFilter.setSample(nextOutputSample);
        auto nextInputSample = noise + lpFilter.returnFilteredValue();
        delayLine.pushSample(1, nextInputSample);
        for (int channel = 0; channel < (int)block.getNumChannels(); ++channel) {
            block.getChannelPointer(channel)[sample] = nextOutputSample; // write to all channels
        }
            
    }

    gain.process(context);
}