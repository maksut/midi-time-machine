#pragma once

#include <JuceHeader.h>
#include "MessageTracker.h"

class Playback
{
public:
    /**
     * Puts zero-or-more midi messages from the playback midi sequence to the midiMessages buffer.
     * This is called in the audio thread!
     */
    bool play(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer, double millisPerSample);

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
    void start(const juce::MidiFile &sourceMidiFile);

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

    static double getTrackStart(const juce::MidiMessageSequence *track);
    static double getInitialSilenceSeconds(const juce::MidiFile &sourceMidiFile);

private:
    juce::MidiFile midiFile;
    double playheadTimeSeconds = -1;
    double initialSilenceSeconds = 0;
    std::unique_ptr<int[]> nextMessageIndexes = nullptr;
    MessageTracker activeNotes;
};
