#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class NSOneEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit NSOneEditor (NSOneProcessor&);
    ~NSOneEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct KnobLookAndFeel : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                               float pos, float startAngle, float endAngle, juce::Slider&) override;
    };

    void timerCallback() override;

    NSOneProcessor& processor;
    KnobLookAndFeel lnf;
    juce::Slider knob;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    float shownReduction = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSOneEditor)
};
