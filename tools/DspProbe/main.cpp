#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI init;

    EasyDelayProcessor p;
    juce::AudioProcessor::BusesLayout stereo;
    stereo.inputBuses.add (juce::AudioChannelSet::stereo());
    stereo.outputBuses.add (juce::AudioChannelSet::stereo());
    p.setBusesLayout (stereo);

    auto set = [&p] (const char* name, float v)
    {
        for (auto* param : p.getParameters())
            if (param->getName (64) == name)
            {
                std::cout << "set " << name << " -> " << v
                          << " now " << param->getText (param->getValue(), 64) << std::endl;
                param->setValueNotifyingHost (v);
                std::cout << "  after: " << param->getText (param->getValue(), 64) << std::endl;
            }
    };

    set ("Mix", 1.0f);
    set ("Delay", 0.6f);
    set ("Feedback", 0.0f);
    std::cout << "delay raw=" << p.apvts.getRawParameterValue ("delayMs")->load()
              << " mix raw=" << p.apvts.getRawParameterValue ("mix")->load() << std::endl;

    p.prepareToPlay (48000.0, 512);

    const int total = (int) (1.5 * 48000);
    const int silence = (int) (0.5 * 48000);
    juce::AudioBuffer<float> buffer (2, total);
    buffer.clear();
    buffer.setSample (0, silence, 1.0f);
    buffer.setSample (1, silence, 1.0f);

    juce::AudioBuffer<float> block (2, 512);
    juce::MidiBuffer midi;
    for (int pos = 0; pos < total; pos += 512)
    {
        const int n = juce::jmin (512, total - pos);
        for (int ch = 0; ch < 2; ++ch)
            block.copyFrom (ch, 0, buffer, ch, pos, n);
        p.processBlock (block, midi);
        for (int ch = 0; ch < 2; ++ch)
            buffer.copyFrom (ch, pos, block, ch, 0, n);
    }

    std::cout << "sample at impulse=" << buffer.getSample (0, silence)
              << " at +30850=" << buffer.getSample (0, silence + 30850) << std::endl;

    int peakAt = 0;
    float peak = 0.0f;
    for (int i = 0; i < total; ++i)
        if (std::abs (buffer.getSample (0, i)) > peak)
        {
            peak = std::abs (buffer.getSample (0, i));
            peakAt = i;
        }
    std::cout << "peak=" << peak << " at " << peakAt
              << " offset=" << (peakAt - silence) << std::endl;
    return 0;
}
