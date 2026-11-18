#include "PluginEditor.h"

namespace
{
    // Charcoal panel, one accent, no gradients. Nothing here should look like
    // it was generated from an "audio plugin UI" prompt.
    const juce::Colour bg          { 0xff17181b };
    const juce::Colour panelLine   { 0xff2b2d33 };
    const juce::Colour textPrimary { 0xffe8e8ea };
    const juce::Colour textMuted   { 0xff85878e };
    const juce::Colour accent      { 0xffe8a33c };
    const juce::Colour dryDot      { 0xff6f737c };

    constexpr int preferredWidth = 520;
    constexpr int preferredHeight = 280;
}

//==============================================================================
struct EasyDelayEditor::LookAndFeel : public juce::LookAndFeel_V4
{
    LookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, textMuted);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        setColour (juce::TextButton::textColourOffId, textMuted);
        setColour (juce::TextButton::textColourOnId, textPrimary);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto arcRadius = radius - 2.0f;
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto lineWidth = juce::jmax (1.5f, radius * 0.032f);

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (panelLine);
        g.strokePath (track, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        if (sliderPos > 0.0001f)
        {
            juce::Path filled;
            filled.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                  rotaryStartAngle, angle, true);
            g.setColour (accent);
            g.strokePath (filled, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        // Knob face: flat, with one hairline so the value is readable.
        g.setColour (juce::Colour (0xff232529));
        g.fillEllipse (juce::Rectangle<float> (radius * 1.12f, radius * 1.12f).withCentre (centre));

        juce::Path pointer;
        const auto pointerLength = radius * 0.52f;
        const auto pointerThickness = juce::jmax (1.5f, radius * 0.05f);
        pointer.addRoundedRectangle (-pointerThickness * 0.5f, -radius * 0.62f,
                                     pointerThickness, pointerLength,
                                     pointerThickness * 0.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
        g.setColour (textPrimary);
        g.fillPath (pointer);
    }

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool, bool) override
    {
        // Text-only reset: nothing to draw.
    }
};

//==============================================================================
EasyDelayEditor::EasyDelayEditor (EasyDelayProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      apvts (p.apvts),
      delayAttachment (apvts, "delayMs", delaySlider),
      feedbackAttachment (apvts, "feedback", feedbackSlider),
      mixAttachment (apvts, "mix", mixSlider)
{
    lookAndFeel = std::make_unique<LookAndFeel>();

    const auto setupKnob = [this] (juce::Slider& s, juce::Label& label, const juce::String& name)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                               juce::MathConstants<float>::pi * 2.75f, true);
        s.setLookAndFeel (lookAndFeel.get());
        addAndMakeVisible (s);

        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (juce::FontOptions (11.0f)).withExtraKerningFactor (0.10f));
        label.setColour (juce::Label::textColourId, textMuted);
        addAndMakeVisible (label);
    };

    setupKnob (delaySlider, delayLabel, "DELAY");
    setupKnob (feedbackSlider, feedbackLabel, "FEEDBACK");
    setupKnob (mixSlider, mixLabel, "MIX");

    addAndMakeVisible (readout);

    statusBadge.setJustificationType (juce::Justification::centredRight);
    statusBadge.setInterceptsMouseClicks (false, false);
    statusBadge.setFont (juce::Font (juce::FontOptions (11.0f)).withExtraKerningFactor (0.12f));
    addAndMakeVisible (statusBadge);

    resetButton.setLookAndFeel (lookAndFeel.get());
    resetButton.setTooltip ("Clear the delay line and start buffering again");
    resetButton.onClick = [this] { resetDelayLine(); };
    addAndMakeVisible (resetButton);

    setResizable (true, true);
    setResizeLimits (420, 250, 1040, 600);
    setSize (preferredWidth, preferredHeight);

    updateReadouts();
    startTimerHz (30);
}

//==============================================================================
void EasyDelayEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    // A single hairline under the header; the number is the hero, not chrome.
    g.setColour (panelLine);
    g.fillRect (juce::Rectangle<float> (24.0f, 62.0f, (float) getWidth() - 48.0f, 1.0f));

    g.setColour (textPrimary);
    g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)).withExtraKerningFactor (0.06f));
    g.drawText ("EASY DELAY", 24, 16, getWidth() - 200, 22, juce::Justification::centredLeft);

    g.setColour (textMuted);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("zero PDC - delayed listening", 24, 38, getWidth() - 200, 16,
                juce::Justification::centredLeft);
}

void EasyDelayEditor::resized()
{
    auto area = getLocalBounds().reduced (24);
    area.removeFromBottom (juce::jmax (0, area.getHeight() - 222));

    auto header = area.removeFromTop (38);
    resetButton.setBounds (header.removeFromRight (48).withSizeKeepingCentre (48, 22));
    area.removeFromTop (8);   // hairline rule at y = 62

    auto valueRow = area.removeFromTop (46);
    readout.setBounds (valueRow.removeFromLeft (juce::jmax (170, area.getWidth() - 160)).reduced (2, 0));
    statusBadge.setBounds (valueRow);

    area.removeFromTop (8);

    const int knobWidth = area.getWidth() / 3;
    const int knobHeight = juce::jlimit (68,
                                         juce::jmin (area.getWidth() / 3 - 12, area.getHeight() - 22),
                                         juce::jmin (area.getWidth() / 3 - 12, area.getHeight() - 22));
    const int rowTop = area.getY() + juce::jmax (0, (area.getHeight() - knobHeight - 20) / 2);

    const auto placeKnob = [] (juce::Slider& slider, juce::Label& label,
                               juce::Rectangle<int> knobArea, juce::Rectangle<int> labelArea)
    {
        slider.setBounds (knobArea);
        label.setBounds (labelArea);
    };

    for (int i = 0; i < 3; ++i)
    {
        const juce::Rectangle<int> cell (area.getX() + i * knobWidth, rowTop, knobWidth, knobHeight + 20);
        auto& slider = i == 0 ? delaySlider : (i == 1 ? feedbackSlider : mixSlider);
        auto& label  = i == 0 ? delayLabel  : (i == 1 ? feedbackLabel  : mixLabel);
        placeKnob (slider, label, cell.withTrimmedBottom (20), cell.withTrimmedTop (knobHeight));
    }
}

//==============================================================================
void EasyDelayEditor::timerCallback()
{
    updateReadouts();
}

void EasyDelayEditor::updateReadouts()
{
    readout.setDelayMs (processor.currentDelayMs.load());

    const bool ready = processor.delayReady.load();
    statusBadge.setText (ready ? "DELAYED" : "DRY - BUFFERING", juce::dontSendNotification);
    statusBadge.setColour (juce::Label::textColourId, ready ? accent : dryDot);
}

void EasyDelayEditor::resetDelayLine()
{
    processor.prepareToPlay (processor.hostSampleRate.load() > 0.0 ? processor.hostSampleRate.load() : 44100.0, 512);
    updateReadouts();
}
//==============================================================================
void EasyDelayEditor::Readout::setDelayMs (float ms)
{
    if (! juce::approximatelyEqual (ms, delayMs))
    {
        delayMs = ms;
        repaint();
    }
}

void EasyDelayEditor::Readout::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto number = delayMs < 10.0f ? juce::String (delayMs, 2)
                                        : juce::String (juce::roundToInt (delayMs));

    const auto numberFont = juce::Font (juce::FontOptions (34.0f)).withExtraKerningFactor (-0.01f);
    const auto unitFont = juce::Font (juce::FontOptions (12.0f)).withExtraKerningFactor (0.10f);

    juce::GlyphArrangement arrangement;
    arrangement.addLineOfText (numberFont, number, 0.0f, 0.0f);
    const auto numberWidth = arrangement.getBoundingBox (0, -1, true).getWidth();

    g.setColour (textPrimary);
    arrangement.draw (g, juce::AffineTransform::translation (bounds.getX() + 1.0f,
                                                             bounds.getCentreY() + numberFont.getAscent() * 0.5f - 2.0f));

    g.setColour (textMuted);
    g.setFont (unitFont);
    g.drawText ("ms", juce::Rectangle<float> (bounds.getX() + numberWidth + 8.0f,
                                              bounds.getY(), 40.0f, bounds.getHeight()),
                juce::Justification::centredLeft, false);
}



