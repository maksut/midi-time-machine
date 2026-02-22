#include "Playback.h"

Playback::Playback() {}

double Playback::play(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer, double millisPerSample)
{
    if (!isReadyToPlay())
        return true; // The midi file is already played

    int numTracks = mMidiFile.getNumTracks();
    double bufferTimeSeconds = numOfSamplesInBuffer * millisPerSample / 1000.0f;
    bool allTracksPlayed = true;

    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
    {
        if (!playOneTrack(trackIndex, midiMessages, numOfSamplesInBuffer, bufferTimeSeconds))
            allTracksPlayed = false;
    }

    // Move the playhead next buffer window
    mPlayheadTimeSeconds += bufferTimeSeconds;

    if (allTracksPlayed)
        reset(); // finished playing the midi file

    return mPlayheadTimeSeconds;
}

bool Playback::playOneTrack(
    int trackIndex,
    juce::MidiBuffer &destination,
    int numOfSamplesInBuffer,
    double bufferTimeSeconds)
{
    auto sourceTrack = mMidiFile.getTrack(trackIndex);
    int numEvents = sourceTrack->getNumEvents();
    auto &nextMessageIndex = mNextMessageIndexes[trackIndex];

    while (nextMessageIndex < numEvents)
    {
        auto &message = sourceTrack->getEventPointer(nextMessageIndex)->message;

        double messageTimeDiff = message.getTimeStamp() - mPlayheadTimeSeconds;
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
            mActiveNotes.track(message);
        }
        else
        {
            // It is not time for the next available midi message. Try it in next buffer window.
            break;
        }
    }

    return (nextMessageIndex >= numEvents); // Is this tracked played completely?
}

void Playback::start(const juce::MidiFile &sourceMidiFile, double startPlayheadPos)
{
    mMidiFile = juce::MidiFile(sourceMidiFile); // clones the midi file

    // Make sure the timestamps of the events are in seconds
    mMidiFile.convertTimestampTicksToSeconds();

    double midiDurationSec = 0.0f;

    // Calculate duration and set the playhead time seconds
    for (int trackIndex = 0; trackIndex < mMidiFile.getNumTracks(); ++trackIndex)
        midiDurationSec = juce::jmax(midiDurationSec, mMidiFile.getTrack(trackIndex)->getEndTime());

    mPlayheadTimeSeconds = midiDurationSec * startPlayheadPos;

    // Initialise nextMessage indexes by dropping messages before the playheadTimeSeconds
    mNextMessageIndexes = std::make_unique<int[]>(mMidiFile.getNumTracks());

    for (int trackIndex = 0; trackIndex < mMidiFile.getNumTracks(); ++trackIndex)
    {
        const juce::MidiMessageSequence *track = mMidiFile.getTrack(trackIndex);
        int numEvents = track->getNumEvents();

        for (int messageIndex = 0; messageIndex < numEvents; ++messageIndex)
        {
            juce::MidiMessage &message = track->getEventPointer(messageIndex)->message;

            if (message.getTimeStamp() >= mPlayheadTimeSeconds)
                break;
            else
                ++mNextMessageIndexes[trackIndex];
        }
    }
}

void Playback::stop(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer)
{
    mActiveNotes.stopActiveNotes(midiMessages, numOfSamplesInBuffer);

    reset();
}

void Playback::reset()
{
    // Reset the indexes to zeros
    std::fill_n(mNextMessageIndexes.get(), mMidiFile.getNumTracks(), 0);

    // Mark the playheadTime as "DONE"
    mPlayheadTimeSeconds = -1;

    // Reset the note on and sustain state
    mActiveNotes.reset();
}

bool Playback::isReadyToPlay()
{
    return mPlayheadTimeSeconds != -1;
}
