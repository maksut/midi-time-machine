#pragma once

#include <JuceHeader.h>
#include "../Processor.h"
#include "../Store.h"
#include "Toolbar.h"
#include "Content.h"
#include "MidiRoll.h"

class Editor : public juce::AudioProcessorEditor, public juce::ValueTree::Listener
{
public:
    Editor(Processor &processor, Store &store);
    ~Editor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override;

private:
    State &mState;
    Store &mStore;
    juce::TextButton mOpenDataFolderButton;

    MidiRoll mMidiRoll;
    MidiPreview mMidiPreview;
    Toolbar mToolbar;
    Content mContent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
