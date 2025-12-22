#pragma once

#include <JuceHeader.h>

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
        double bufferTimeMs);

    /**
     * Called in audio thread
     */
    void reset();

    void resetNoteOnsAndSustain();

    static double getInitialSilenceMs(const juce::MidiFile &sourceMidiFile);

private:
    juce::MidiFile midiFile;
    double playheadTime = -1;
    double initialSilenceMs = 0;
    std::unique_ptr<int[]> nextMessageIndexes = nullptr;

    // To keep track of the active notes/sounds
    bool currentNoteOns[16][128] = {};
    int currentSustain[16] = {};
};
