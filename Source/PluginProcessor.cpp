#include "PluginProcessor.h"
#include "PluginEditor.h"

NSOneProcessor::NSOneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    amountParam = apvts.getRawParameterValue ("amount");
}

juce::AudioProcessorValueTreeState::ParameterLayout NSOneProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "amount", 1 }, "Suppress",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 60.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));
    return layout;
}

void NSOneProcessor::prepareToPlay (double sampleRate, int)
{
    for (auto& s : suppressors)
    {
        s.prepare (sampleRate);
        s.setAmount (*amountParam * 0.01f);
        s.reset();
    }
    setLatencySamples (nsone::Suppressor::latency);
}

bool NSOneProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == l.getMainInputChannelSet();
}

void NSOneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numIn = getTotalNumInputChannels(), numOut = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, numSamples);
    if (numIn == 0) return;

    const float amount = *amountParam * 0.01f;
    const int numCh = juce::jmin (numIn, 2);
    float red = 0.0f;

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto& sup = suppressors[ch];
        sup.setAmount (amount);
        auto* d = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            d[i] = sup.process (d[i]);
        red += sup.getReductionDb();
    }
    reductionDb.store (red / (float) numCh);
}

juce::AudioProcessorEditor* NSOneProcessor::createEditor() { return new NSOneEditor (*this); }

void NSOneProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void NSOneProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new NSOneProcessor(); }
