#pragma once

#include <JuceHeader.h>
#include "Processor.h"
#include "Store.h"

class Editor : public juce::AudioProcessorEditor,
               public juce::ChangeListener,
               public juce::Button::Listener
{
public:
    Editor(Processor &processor, Store &store);
    ~Editor() override;

    void paint(juce::Graphics &) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster *source);
    void buttonClicked(juce::Button *button) override;

private:
    Processor &processor;
    Store &store;
    juce::TextButton saveButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
