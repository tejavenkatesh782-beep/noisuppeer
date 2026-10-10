#include "PluginEditor.h"

static const juce::Colour kAccent (0xffff7a29), kBg (0xff0c0e11), kPanel (0xff16191e), kLine (0xff2a2f37);

void NSOneEditor::KnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                     float pos, float start, float end, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (16.0f);
    const auto c = b.getCentre();
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const float ang = start + pos * (end - start);
    const juce::PathStrokeType stroke (10.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    juce::Path track;
    track.addCentredArc (c.x, c.y, r, r, 0.0f, start, end, true);
    g.setColour (kLine);
    g.strokePath (track, stroke);

    if (pos > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (c.x, c.y, r, r, 0.0f, start, ang, true);
        g.setColour (kAccent);
        g.strokePath (value, stroke);
    }

    const float br = r * 0.78f;
    g.setGradientFill (juce::ColourGradient (kPanel.brighter (0.25f), c.x, c.y - br,
                                             kBg, c.x, c.y + br, false));
    g.fillEllipse (c.x - br, c.y - br, br * 2.0f, br * 2.0f);
    g.setColour (kLine);
    g.drawEllipse (c.x - br, c.y - br, br * 2.0f, br * 2.0f, 2.0f);

    juce::Path p;
    p.addRoundedRectangle (-3.0f, -br * 0.88f, 6.0f, br * 0.42f, 3.0f);
    g.setColour (kAccent);
    g.fillPath (p, juce::AffineTransform::rotation (ang).translated (c.x, c.y));
}

NSOneEditor::NSOneEditor (NSOneProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      attachment (p.apvts, "amount", knob)
{
    knob.setLookAndFeel (&lnf);
    knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                              juce::MathConstants<float>::pi * 2.75f, true);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 110, 30);
    knob.setTextValueSuffix (" %");
    knob.setNumDecimalPlacesToDisplay (0);
    knob.setDoubleClickReturnValue (true, 60.0);
    knob.setMouseDragSensitivity (250);
    knob.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (knob);

    setSize (340, 430);
    startTimerHz (30);
}

NSOneEditor::~NSOneEditor() { knob.setLookAndFeel (nullptr); }

void NSOneEditor::timerCallback()
{
    const float target = processor.reductionDb.load();
    shownReduction += (target - shownReduction) * 0.3f;
    repaint (getLocalBounds().removeFromBottom (80));
}

void NSOneEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);
    g.setColour (kPanel);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (8.0f), 14.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (24.0f, juce::Font::bold)));
    g.drawText ("NS-ONE", 0, 22, getWidth(), 30, juce::Justification::centred);
    g.setColour (juce::Colours::grey);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("NOISE SUPPRESSOR", 0, 50, getWidth(), 16, juce::Justification::centred);

    g.setColour (juce::Colours::grey);
    g.drawText ("SUPPRESS", 0, 336, getWidth(), 14, juce::Justification::centred);

    auto meter = juce::Rectangle<float> (40.0f, (float) getHeight() - 52.0f, (float) getWidth() - 80.0f, 8.0f);
    g.setColour (kLine);
    g.fillRoundedRectangle (meter, 4.0f);
    const float frac = juce::jlimit (0.0f, 1.0f, -shownReduction / 40.0f);
    g.setColour (kAccent);
    g.fillRoundedRectangle (meter.withWidth (juce::jmax (8.0f, meter.getWidth() * frac)), 4.0f);
    g.setColour (juce::Colours::grey);
    g.drawText ("REDUCTION  " + juce::String (shownReduction, 1) + " dB",
                0, getHeight() - 38, getWidth(), 16, juce::Justification::centred);
}

void NSOneEditor::resized()
{
    knob.setBounds (60, 80, 220, 250);
}
