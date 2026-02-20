#include "Playback.h"

double Playback::play(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer, double millisPerSample)
{
    if (!isReadyToPlay())
        return true; // The midi file is already played

    int numTracks = midiFile.getNumTracks();
    double bufferTimeSeconds = numOfSamplesInBuffer * millisPerSample / 1000.0f;
    bool allTracksPlayed = true;

    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
    {
        if (!playOneTrack(trackIndex, midiMessages, numOfSamplesInBuffer, bufferTimeSeconds))
            allTracksPlayed = false;
    }

    // Move the playhead next buffer window
    playheadTimeSeconds += bufferTimeSeconds;

    if (allTracksPlayed)
        reset(); // finished playing the midi file

    return playheadTimeSeconds;
}

bool Playback::playOneTrack(
    int trackIndex,
    juce::MidiBuffer &destination,
    int numOfSamplesInBuffer,
    double bufferTimeSeconds)
{
    auto sourceTrack = midiFile.getTrack(trackIndex);
    int numEvents = sourceTrack->getNumEvents();
    auto &nextMessageIndex = nextMessageIndexes[trackIndex];

    while (nextMessageIndex < numEvents)
    {
        auto &message = sourceTrack->getEventPointer(nextMessageIndex)->message;

        // During playback, ignore the initial silence
        double messageTimeDiff = message.getTimeStamp() - initialSilenceSeconds - playheadTimeSeconds;
        int messageSampleNumber = juce::roundToInt(messageTimeDiff / bufferTimeSeconds * numOfSamplesInBuffer);

        if (messageSampleNumber < numOfSamplesInBuffer)
        {
            // The next midi message is in this buffer window. Send it!
            // But filter out meta events like tempo, signature. Which wouldn't be accurate.
            if (!message.isMetaEvent())
            {
                message.setTimeStamp(messageSampleNumber); // not sure if this is needed
                destination.addEvent(message, messageSampleNumber);
            }

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

/**
 * Similar to MidiMessageSequence::getStartTime() but ignores meta events.
 * Returns -1 if there is no regular events in the sequence.
 */
double Playback::getTrackStart(const juce::MidiMessageSequence *track)
{
    int numEvents = track->getNumEvents();

    for (int messageIndex = 0; messageIndex < numEvents; ++messageIndex)
    {
        auto &message = track->getEventPointer(messageIndex)->message;

        if (!message.isMetaEvent()) // ignoring meta events
            return message.getTimeStamp();
    }

    return -1;
}

double Playback::getInitialSilenceSeconds(const juce::MidiFile &midiFile)
{
    int numTracks = midiFile.getNumTracks();

    if (numTracks == 0)
        return 0.0f;

    double silenceSeconds = std::numeric_limits<double>::max();

    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
    {
        double trackStartTime = getTrackStart(midiFile.getTrack(trackIndex));

        if (trackStartTime >= 0)
            silenceSeconds = juce::jmin(silenceSeconds, trackStartTime);
    }

    return silenceSeconds == std::numeric_limits<double>::max() ? 0 : silenceSeconds;
}

void Playback::start(const juce::MidiFile &sourceMidiFile)
{
    midiFile = juce::MidiFile(sourceMidiFile); // clones the midi file
    playheadTimeSeconds = 0;
    nextMessageIndexes = std::make_unique<int[]>(midiFile.getNumTracks());

    // Make sure the timestamps of the events are in seconds
    midiFile.convertTimestampTicksToSeconds();

    // Then calculate the initial silence
    initialSilenceSeconds = getInitialSilenceSeconds(midiFile);
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
    playheadTimeSeconds = -1;

    // Reset the note on and sustain state
    activeNotes.reset();
}

bool Playback::isReadyToPlay()
{
    return playheadTimeSeconds != -1;
}
