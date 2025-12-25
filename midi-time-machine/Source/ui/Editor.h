#pragma once

#include <JuceHeader.h>
#include "../Processor.h"
#include "../Store.h"
#include "Toolbar.h"
#include "Content.h"

class Editor : public juce::AudioProcessorEditor, public juce::ValueTree::Listener
{
public:
    Editor(Processor &processor, Store &store);
    ~Editor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override;

private:
    Processor &processor;
    State &state;
    Store &store;
    juce::TextButton openDataFolderButton;

    Toolbar toolbar;
    Content content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
