#pragma once

#include "../Processor.h"
#include "../Store.h"
#include "Content.h"
#include "MidiRoll.h"
#include "Toolbar.h"
#include <JuceHeader.h>

class Editor : public juce::AudioProcessorEditor,
               public juce::KeyListener
{
  public:
    Editor(Processor &processor, Store &store);
    ~Editor();

    void paint(juce::Graphics &) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent) override;

    // To supress -Woverloaded-virtual
    bool keyPressed(const juce::KeyPress &) override
    {
        return false;
    }

    void addToDesktop(int windowStyleFlags, void *nativeWindowToAttachTo = nullptr) override;

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
