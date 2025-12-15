#pragma once

#include <JuceHeader.h>
#include "Processor.h"

class Store : public juce::ChangeListener
{
public:
    Store(Processor &processor);
    virtual ~Store();

    void changeListenerCallback(juce::ChangeBroadcaster *source);
    bool prepareAndSaveLastMidi();

private:
    static bool saveMidiFile(const juce::MidiFile &midiFile, const juce::File &destination);
    static juce::String getDataString(const juce::MidiMessage &m);
    static juce::String getEventString(const juce::MidiMessage &m);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Processor &processor;
    juce::MidiMessageSequence midiSequence;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Store)
};
