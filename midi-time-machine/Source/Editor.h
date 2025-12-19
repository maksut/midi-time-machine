#pragma once

#include <JuceHeader.h>
#include "Processor.h"
#include "Store.h"
#include "Toolbar.h"

class Editor : public juce::AudioProcessorEditor,
               public juce::ChangeListener
{
public:
    Editor(Processor &processor, Store &store);
    ~Editor() override;

    void paint(juce::Graphics &) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster *source);

private:
    Processor &processor;
    Store &store;
    juce::TextButton openDataFolderButton;
    Toolbar toolbar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
