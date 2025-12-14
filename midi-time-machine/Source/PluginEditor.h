#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MTMAudioProcessorEditor : public juce::AudioProcessorEditor,
                                public juce::ChangeListener,
                                public juce::Button::Listener
{
public:
    MTMAudioProcessorEditor(MTMAudioProcessor &);
    ~MTMAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster *source);
    void buttonClicked(juce::Button *button) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MTMAudioProcessor &audioProcessor;
    juce::MidiMessageSequence midiSequence;
    juce::TextButton saveButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MTMAudioProcessorEditor)
};
