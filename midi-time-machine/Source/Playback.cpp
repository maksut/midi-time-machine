#include "Playback.h"

void Playback::play(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer, double millisPerSample)
{
    if (!isReadyToPlay())
        return; // The midi file is already played

    int numTracks = midiFile.getNumTracks();
    double bufferTimeMs = numOfSamplesInBuffer * millisPerSample;
    bool allTracksPlayed = true;

    for (int i = 0; i < numTracks; ++i)
    {
        if (!playOneTrack(midiFile, i, midiMessages, numOfSamplesInBuffer, bufferTimeMs))
            allTracksPlayed = false;
    }

    // Move the playhead next buffer window
    playheadTime += bufferTimeMs;

    if (allTracksPlayed)
        stop(); // finished playing this 'source' sequence
}

bool Playback::playOneTrack(
    const juce::MidiFile &sourceMidiFile,
    int trackIndex,
    juce::MidiBuffer &destination,
    int numOfSamplesInBuffer,
    double bufferTimeMs)
{
    auto &source = *sourceMidiFile.getTrack(trackIndex);
    int numEvents = source.getNumEvents();
    auto &nextMessageIndex = nextMessageIndexes[trackIndex];

    while (nextMessageIndex < numEvents)
    {
        auto &message = source.getEventPointer(nextMessageIndex)->message;

        double messageTimeDiff = message.getTimeStamp() - playheadTime;
        int messageSampleNumber = juce::roundToInt(messageTimeDiff / bufferTimeMs * numOfSamplesInBuffer);

        if (messageSampleNumber < numOfSamplesInBuffer)
        {
            // The next midi message is in this buffer window. Send it!
            message.setTimeStamp(messageSampleNumber); // not sure if this is needed
            destination.addEvent(message, messageSampleNumber);

            ++nextMessageIndex;
        }
        else
        {
            // It is not time for the next available midi message. Try it in next buffer window.
            break;
        }
    }

    return (nextMessageIndex >= numEvents); // Is this tracked played completely?
}

void Playback::start(const juce::MidiFile &sourceMidiFile)
{
    midiFile = juce::MidiFile(sourceMidiFile); // clones the midi file
    playheadTime = 0;
    nextMessageIndexes = std::make_unique<int[]>(midiFile.getNumTracks());
}

void Playback::stop()
{
    // Reset the indexes to zeros
    std::fill_n(nextMessageIndexes.get(), midiFile.getNumTracks(), 0);

    // Mark the playheadTime as "DONE"
    playheadTime = -1;
}

bool Playback::isReadyToPlay()
{
    return playheadTime != -1;
}
