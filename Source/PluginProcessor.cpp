#include "PluginProcessor.h"
#include "PluginEditor.h"


namespace
{
    constexpr float minDelayMs = 0.0f;
    constexpr float maxDelayMs = EasyDelayProcessor::maxDelayMs;

    // Four seconds of storage covers the 2 s range plus glide overshoot, and
    // leaves room to grow if the range ever does.
    constexpr double delayLineSeconds = 4.0;

    constexpr float glideTimeConstantSeconds = 0.060f;   // tape-style glide
    constexpr float mixTimeConstantSeconds = 0.020f;
    constexpr float feedbackTimeConstantSeconds = 0.020f;

    // With 100 % feedback the write path is scaled down so the line decays
    // instead of slamming into the clamp.
    constexpr float feedbackWriteGain = 0.5f;

    float onePoleCoefficient (float timeConstantSeconds, double sampleRate) noexcept
    {
        return 1.0f - std::exp (-1.0f / (timeConstantSeconds * (float) sampleRate));
    }
}

EasyDelayProcessor::EasyDelayProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Deliberately zero. The whole point of this plugin is to be heard later
    // without the host applying plug-in delay compensation to the track.
    setLatencySamples (0);
}

juce::AudioProcessorValueTreeState::ParameterLayout EasyDelayProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delayMs", 1 }, "Delay", juce::NormalisableRange<float> (minDelayMs, maxDelayMs, 0.1f, 0.45f), 250.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms").withStringFromValueFunction (
            [] (float v, int) { return v < 10.0f ? juce::String (v, 2) : juce::String (juce::roundToInt (v)); })));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "feedback", 1 }, "Feedback", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 25.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Mix", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 35.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    return layout;
}

float EasyDelayProcessor::glideCoefficient (double sr)
{
    return onePoleCoefficient (glideTimeConstantSeconds, sr);
}

void EasyDelayProcessor::prepareToPlay (double newSampleRate, int)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    bufferLength = juce::jmax (8, (int) std::ceil (delayLineSeconds * sampleRate));
    delayLine.setSize (juce::jmax (1, getTotalNumInputChannels()), bufferLength);
    delayLine.clear();

    writePos = 0;
    samplesWritten = 0;

    const float delayParam = apvts.getRawParameterValue ("delayMs")->load();
    smoothedDelaySamples = juce::jmax (1.0f, delayParam * 0.001f * (float) sampleRate);
    smoothedMix = *apvts.getRawParameterValue ("mix") * 0.01f;
    smoothedFeedback = *apvts.getRawParameterValue ("feedback") * 0.01f;

    glideCoeff = glideCoefficient (sampleRate);
    mixCoeff = onePoleCoefficient (mixTimeConstantSeconds, sampleRate);
    feedbackCoeff = onePoleCoefficient (feedbackTimeConstantSeconds, sampleRate);

    hostSampleRate.store (sampleRate);
    currentDelayMs.store (delayParam);
    delayReady.store (false);

    setLatencySamples (0);
}

void EasyDelayProcessor::releaseResources()
{
}

bool EasyDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();

    if (mainIn != mainOut)
        return false;

    return mainOut == juce::AudioChannelSet::stereo()
        || mainOut == juce::AudioChannelSet::mono();
}

double EasyDelayProcessor::getTailLengthSeconds() const
{
    return juce::jmin (10.0, 2.0 * (double) maxDelayMs / 1000.0 + 2.0);
}

void EasyDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numSamples == 0)
        return;

    // Defensive: the host may hand us a layout we did not see in prepareToPlay.
    if (delayLine.getNumChannels() < numChannels || bufferLength <= 0)
    {
        prepareToPlay (sampleRate, numSamples);
        if (delayLine.getNumChannels() < numChannels)
            return;
    }

    const float targetDelaySamples = juce::jlimit (
        1.0f,
        (float) (bufferLength - 2),
        *apvts.getRawParameterValue ("delayMs") * 0.001f * (float) sampleRate);

    const float targetMix = juce::jlimit (0.0f, 1.0f, *apvts.getRawParameterValue ("mix") * 0.01f);
    const float targetFeedback = juce::jlimit (0.0f, 1.0f, *apvts.getRawParameterValue ("feedback") * 0.01f);

    auto* line = delayLine.getArrayOfWritePointers();

    for (int i = 0; i < numSamples; ++i)
    {
        // Tape-style glide: the read head slides to the new delay time.
        smoothedDelaySamples += (targetDelaySamples - smoothedDelaySamples) * glideCoeff;
        if (std::abs (targetDelaySamples - smoothedDelaySamples) < 0.01f)
            smoothedDelaySamples = targetDelaySamples;

        const float delaySamples = juce::jlimit (1.0f, (float) (bufferLength - 2), smoothedDelaySamples);

        smoothedMix += (targetMix - smoothedMix) * mixCoeff;
        smoothedFeedback += (targetFeedback - smoothedFeedback) * feedbackCoeff;

        // Fractional read head, guaranteed to sit behind the write head.
        float readPos = (float) writePos - delaySamples;
        if (readPos < 0.0f)
            readPos += (float) bufferLength;

        const int ri0 = (int) readPos;
        const int ri1 = (ri0 + 1) % bufferLength;
        const float frac = readPos - (float) ri0;

        // Dry until the line has actually been filled to the requested depth
        // (only relevant right after the plugin starts, or right after a big
        // jump to a longer delay time). Nothing is muted.
        const bool ready = (float) samplesWritten > delaySamples + 1.0f;
        const float mix = ready ? smoothedMix : 0.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            // The buffer holds the incoming audio; the delay line only holds
            // what has already played (plus the feedback tail).
            const float dry = buffer.getSample (ch, i);
            const float wet = line[ch][ri0] + (line[ch][ri1] - line[ch][ri0]) * frac;

            buffer.setSample (ch, i, dry + (wet - dry) * mix);

            // Clamped feedback so 100 % decays instead of pinning to +/-1.
            line[ch][writePos] = juce::jlimit (-1.0f, 1.0f,
                                               dry + wet * smoothedFeedback * feedbackWriteGain);
        }

        if (++writePos >= bufferLength)
            writePos = 0;

        if (samplesWritten < bufferLength)
            ++samplesWritten;
    }


    currentDelayMs.store (targetDelaySamples / (float) sampleRate * 1000.0f);
    delayReady.store ((float) samplesWritten > targetDelaySamples + 1.0f);
}

juce::AudioProcessorEditor* EasyDelayProcessor::createEditor()
{
    return new EasyDelayEditor (*this);
}

void EasyDelayProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void EasyDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EasyDelayProcessor();
}













