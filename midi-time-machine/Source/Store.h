#pragma once

#include <JuceHeader.h>
#include "MessageTracker.h"

class Processor;
class State;

class Store : public juce::Timer
{
public:
    Store(Processor &processor);

    void timerCallback() override;
    bool prepareAndSaveLastMidi();

    juce::MidiKeyboardState &getKeyboardState();
    MessageTracker &getRecordingTracker();
    MessageTracker &getPlaybackTracker();
    void drainProcessorMidiQueue();

private:
    void reset();
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

    juce::MidiKeyboardState keyboardState;
    MessageTracker recordingTracker;
    MessageTracker playbackTracker;

    juce::int64 lastNoteReceivedTimeMs = std::numeric_limits<juce::int64>::max();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Store)
};
