#include "Playback.h"

bool Playback::play(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer, double millisPerSample)
{
    if (!isReadyToPlay())
        return true; // The midi file is already played

    int numTracks = midiFile.getNumTracks();
    double bufferTimeMs = numOfSamplesInBuffer * millisPerSample;
    bool allTracksPlayed = true;

    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
    {
        if (!playOneTrack(trackIndex, midiMessages, numOfSamplesInBuffer, bufferTimeMs))
            allTracksPlayed = false;
    }

    // Move the playhead next buffer window
    playheadTime += bufferTimeMs;

    if (allTracksPlayed)
        reset(); // finished playing the midi file

    return allTracksPlayed;
}

bool Playback::playOneTrack(
    int trackIndex,
    juce::MidiBuffer &destination,
    int numOfSamplesInBuffer,
    double bufferTimeMs)
{
    auto sourceTrack = midiFile.getTrack(trackIndex);
    int numEvents = sourceTrack->getNumEvents();
    auto &nextMessageIndex = nextMessageIndexes[trackIndex];

    while (nextMessageIndex < numEvents)
    {
        auto &message = sourceTrack->getEventPointer(nextMessageIndex)->message;

        // During playback, ignore the initial silence
        double messageTimeDiff = message.getTimeStamp() - initialSilenceMs - playheadTime;
        int messageSampleNumber = juce::roundToInt(messageTimeDiff / bufferTimeMs * numOfSamplesInBuffer);

        if (messageSampleNumber < numOfSamplesInBuffer)
        {
            // The next midi message is in this buffer window. Send it!
            message.setTimeStamp(messageSampleNumber); // not sure if this is needed
            destination.addEvent(message, messageSampleNumber);

            ++nextMessageIndex;

            // Keep track of current note ons and sustain
            activeNotes.track(message);
        }
        else
        {
            // It is not time for the next available midi message. Try it in next buffer window.
            break;
        }
    }

    return (nextMessageIndex >= numEvents); // Is this tracked played completely?
}

double Playback::getInitialSilenceMs(const juce::MidiFile &midiFile)
{
    int numTracks = midiFile.getNumTracks();

    if (numTracks == 0)
        return 0.0f;

    double silenceMs = std::numeric_limits<double>::max();

    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
    {
        auto track = midiFile.getTrack(trackIndex);
        silenceMs = juce::jmin(silenceMs, track->getStartTime());
    }

    return silenceMs;
}

void Playback::start(const juce::MidiFile &sourceMidiFile)
{
    midiFile = juce::MidiFile(sourceMidiFile); // clones the midi file
    initialSilenceMs = getInitialSilenceMs(sourceMidiFile);
    playheadTime = 0;
    nextMessageIndexes = std::make_unique<int[]>(midiFile.getNumTracks());
}

void Playback::stop(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer)
{
    activeNotes.stopActiveNotes(midiMessages, numOfSamplesInBuffer);

    reset();
}

void Playback::reset()
{
    // Reset the indexes to zeros
    std::fill_n(nextMessageIndexes.get(), midiFile.getNumTracks(), 0);

    // Mark the playheadTime as "DONE"
    playheadTime = -1;

    // Reset the note on and sustain state
    activeNotes.reset();
}

bool Playback::isReadyToPlay()
{
    return playheadTime != -1;
}
