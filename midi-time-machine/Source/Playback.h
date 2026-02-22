#pragma once

#include <JuceHeader.h>
#include "MessageTracker.h"

class Playback
{
public:
    Playback();

    /**
     * Puts zero-or-more midi messages from the playback midi sequence to the midiMessages buffer.
     * This is called in the audio thread!
     *
     * @return playhead time in seconds. -1 if not playing.
     */
    double play(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer, double millisPerSample);

    /**
     * Called in audio thread
     */
    void stop(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer);

    /**
     * Called in audio thread
     */
    bool isReadyToPlay();

    /**
     * Should not be called in audio thread
     */
    void start(const juce::MidiFile &sourceMidiFile, double startPlayheadPos);

private:
    bool playOneTrack(
        int trackIndex,
        juce::MidiBuffer &destination,
        int numOfSamplesInBuffer,
        double bufferTimeSeconds);

    /**
     * Called in audio thread
     */
    void reset();

private:
    juce::MidiFile midiFile;
    double playheadTimeSeconds = -1;
    std::unique_ptr<int[]> nextMessageIndexes = nullptr;
    MessageTracker activeNotes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Playback)
};
