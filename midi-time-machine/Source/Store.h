#pragma once

#include <JuceHeader.h>
#include "Processor.h"

class Store : public juce::ChangeListener, juce::Timer
{
public:
    Store(Processor &processor);
    virtual ~Store();

    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    bool prepareAndSaveLastMidi();

private:
    void drainProcessorMidiQueue();
    bool saveTpqMidiFile(int noOfNoteOns, int durationMs);
    bool saveSmpteMidiFile(int noOfNoteOns, int durationMs);

    static bool saveMidiFile(const juce::MidiFile &midiFile, int noOfNoteOns, int durationMs, const juce::String &filenamePostfix);
    static juce::String getDataString(const juce::MidiMessage &m);
    static juce::String getEventString(const juce::MidiMessage &m);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Processor &processor;
    juce::MidiMessageSequence midiSequence;
    juce::int64 lastQueueDrainedTimeMs = std::numeric_limits<juce::int64>::max();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Store)
};
