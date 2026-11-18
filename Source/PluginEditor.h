#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class EasyDelayEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit EasyDelayEditor (EasyDelayProcessor&);
    ~EasyDelayEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateReadouts();
    void resetDelayLine();

    EasyDelayProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;

    struct LookAndFeel;
    std::unique_ptr<LookAndFeel> lookAndFeel;

    juce::Slider delaySlider, feedbackSlider, mixSlider;
    juce::Label delayLabel, feedbackLabel, mixLabel;

    // The live delay readout is drawn, not a Label, so the number can carry
    // the weight and the unit can stay quiet.
    struct Readout : juce::Component
    {
        void setDelayMs (float ms);
        void paint (juce::Graphics&) override;
        float delayMs = 0.0f;
    };

    Readout readout;
    juce::Label statusBadge;
    juce::TextButton resetButton { "Reset" };

    juce::AudioProcessorValueTreeState::SliderAttachment delayAttachment, feedbackAttachment, mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyDelayEditor)
};
