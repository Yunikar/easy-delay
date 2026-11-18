#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <memory>
#include <vector>


// Easy Delay 鈥?a musical delay that stays out of the host's delay compensation.
//
// How it differs from an ordinary delay plugin:
//   * It reports 0 samples of latency to the host, so Studio One (or any VST3
//     host) never shifts the track to compensate. The timeline stays put and
//     the sound simply arrives late.
//   * The delay line starts empty, so during the first N milliseconds the
//     plugin passes the dry signal through, then hands over to the delayed
//     signal. Nothing is muted and the track does not jump.
//   * Delay-time changes glide (one-pole smoothing), tape-style, instead of
//     jumping to a new read position.
class EasyDelayProcessor : public juce::AudioProcessor
{
public:
    EasyDelayProcessor();
    ~EasyDelayProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Easy Delay"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

    // Values the editor polls for its live readouts.
    std::atomic<float> currentDelayMs { 0.0f };   // smoothed, actually used delay
    std::atomic<bool> delayReady { false };       // false while dry passthrough
    std::atomic<double> hostSampleRate { 0.0 };

    static constexpr float maxDelayMs = 2000.0f;

private:
    // One-pole coefficient for the delay-time glide (~60 ms time constant).
    static float glideCoefficient (double sampleRate);

    juce::AudioBuffer<float> delayLine;
    int bufferLength = 0;
    int writePos = 0;
    int samplesWritten = 0;

    double sampleRate = 44100.0;
    float smoothedDelaySamples = 1.0f;
    float smoothedMix = 0.5f;
    float smoothedFeedback = 0.0f;

    float glideCoeff = 0.0f;
    float mixCoeff = 0.0f;
    float feedbackCoeff = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyDelayProcessor)
};

