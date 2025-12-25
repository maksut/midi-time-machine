#pragma once

#include <JuceHeader.h>
#include "MessageTracker.h"

class Processor;
class State;

class Store : public juce::ValueTree::Listener, juce::Timer
{
public:
    Store(Processor &processor);
    virtual ~Store();

    void timerCallback() override;
    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override;
    bool prepareAndSaveLastMidi();

    typedef std::optional<std::reference_wrapper<const juce::MidiFile>> MaybeMidiFile;
    MaybeMidiFile getLastSavedMidiFile();
    juce::String getLastSavedFileDescription();
    juce::String getLastSavedFilePath();

private:
    void drainProcessorMidiQueue();
    bool saveTpqMidiFile(int noOfNoteOns, int durationMs);
    bool saveSmpteMidiFile(int noOfNoteOns, int durationMs);
    juce::File getRootDataDir();

    bool saveMidiFile(
        const juce::MidiFile &midiFile,
        int noOfNoteOns,
        int durationMs,
        const juce::String &filenamePostfix);

    static juce::String getDataString(const juce::MidiMessage &m);
    static juce::String getEventString(const juce::MidiMessage &m);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Processor &processor;
    State &state;
    juce::MidiMessageSequence midiSequence;
    MessageTracker messageTracker;
    juce::int64 lastQueueDrainedTimeMs = std::numeric_limits<juce::int64>::max();

    std::optional<juce::MidiFile> lastSavedFile = {};
    juce::String lastSavedFileDescription = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Store)
};
