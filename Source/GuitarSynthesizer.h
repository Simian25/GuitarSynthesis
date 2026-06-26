#pragma once
#include <JuceHeader.h>

class SimpleLowPassFilter
{
    public:
        void setSample(float sample) {
            previousSample = currentSample;
            currentSample = sample;
        }
        void reset() {
            currentSample = 0;
            previousSample = 0;
        }
        float returnFilteredValue() {
            return 0.5 * currentSample + 0.5 * previousSample;
        }
    private:
        float currentSample = 0;
        float previousSample = 0;
};

class WhiteNoiseGenerator
{
public:
    void setMaxSamples(int numSamples) { maxSamples = numSamples; }
    void reset() { samplesGenerated = 0; }
    bool isFinished()            const { return samplesGenerated >= maxSamples; }

    float nextSample()
    {
        if (isFinished()) return 0.0f;
        ++samplesGenerated;
        return random.nextFloat() * 2.0f - 1.0f;
    }

private:
    juce::Random random;
    int maxSamples = 0;
    int samplesGenerated = 0;
};


class GuitarSynthesizer : public juce::dsp::ProcessorBase
{
public:
    GuitarSynthesizer() = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void noteOn(int midiNote, float velocity);
    void noteOff();
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    int  currentNote() const { return midiNote; }
    bool isActive() const { return active; }


private:
    int  midiNote = -1;
    juce::Random           random;
    juce::dsp::Gain<float> gain;
    juce::dsp::DelayLine<float> delayLine;
    WhiteNoiseGenerator whitenoise;
    SimpleLowPassFilter lpFilter;
    float sampleRate = 48000.0;
    float numberOfWhiteNoiseSamples = 0;
    int numChannels = 2;
    bool  isprocessing = false;
    bool active = false;
    float velocity = 1.0f;
};