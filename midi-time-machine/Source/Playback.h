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
    void stop();

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
        const juce::MidiFile &sourceMidiFile,
        int trackIndex,
        juce::MidiBuffer &destination,
        int numOfSamplesInBuffer,
        double bufferTimeMs);

private:
    juce::MidiFile midiFile;
    double playheadTime = -1;
    std::unique_ptr<int[]> nextMessageIndexes = nullptr;
};
