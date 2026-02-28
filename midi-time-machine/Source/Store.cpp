#include "Store.h"
#include "Processor.h"
#include "State.h"

const int POLL_TIME_MILLIS = 1000;

Store::Store(Processor &processor) : mProcessor(processor), mState(processor.getState())
{
    // Start the timer
    startTimer(POLL_TIME_MILLIS);
}

bool Store::saveMidiFile(
    const juce::MidiFile &midiFile,
    int noOfNoteOns,
    int durationMs,
    const juce::String &filenamePostfix)
{
    juce::Time now = juce::Time::getCurrentTime();

    juce::String yearAndMonth;
    yearAndMonth
        << juce::String(now.getYear()).paddedLeft('0', 4) << "-"
        << juce::String(now.getMonth() + 1).paddedLeft('0', 2); // Month is 0-based

    juce::String date;
    date
        << yearAndMonth << "-"
        << juce::String(now.getDayOfMonth()).paddedLeft('0', 2);

    juce::String day(now.getWeekdayName(false));

    juce::String hours(now.getHours());
    juce::String mins(now.getMinutes());
    juce::String secs(now.getSeconds());

    juce::String seconds;
    seconds << juce::String(juce::roundToInt(durationMs / 1000));

    juce::String dateAndDay;
    dateAndDay << date << " " << day;

    juce::MemoryOutputStream filename;
    filename
        << dateAndDay << " "
        << hours.paddedLeft('0', 2) << "-" << mins.paddedLeft('0', 2) << "-" << secs.paddedLeft('0', 2) << " "
        << juce::String(noOfNoteOns) << " notes " << seconds << " seconds"
        << filenamePostfix << juce::String(".mid");

    juce::File parentDir = getRootDataDir().getChildFile(yearAndMonth).getChildFile(dateAndDay);

    if (!parentDir.exists())
    {
        if (parentDir.createDirectory().failed())
            return false;
    }

    if (!parentDir.isDirectory())
        return false;

    juce::File targetFile(parentDir.getChildFile(filename.toUTF8()));

    if (targetFile.exists())
        return false;

    juce::FileOutputStream stream(targetFile);

    if (!stream.openedOk())
        return false;

    // Finally write it
    if (!midiFile.writeTo(stream))
        return false;

    // File is saved. Update the selected midi file.
    mState.setSelectedMidiFile(targetFile.getFullPathName());

    return true;
}

/**
 * Saves the midiSequence into a file with "ticksPerQuarterNote" timestamps.
 *
 * So this converts the whole midi sequence into 120bmp, 4/4 tempo, 960 ticksPerQuarterNote.
 * And it inserts its own time signature and tempo meta events.
 * It also drops any existing time signature and tempo meta events.
 */
bool Store::saveTpqMidiFile(int noOfNoteOns, int durationMs)
{
    int beatsPerMinute = 120;
    int microsPerQuarterNote = int((60. / beatsPerMinute) * 1000000); // 500000 =>  0.5 seconds for 4/4 tempo and 120 bpm
    int ticksPerQuarterNote = 960;
    double ticksPerMilliSecond = ticksPerQuarterNote * (beatsPerMinute / 60.) / 1000.0; // for 4/4 tempo

    juce::MidiMessageSequence tpqMidiSequence;
    tpqMidiSequence.addEvent(juce::MidiMessage::tempoMetaEvent(microsPerQuarterNote), 0);
    tpqMidiSequence.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4), 0);

    for (int i = 0; i < mMidiSequence.getNumEvents(); ++i)
    {
        auto &message = mMidiSequence.getEventPointer(i)->message;
        auto timestampInMillis = message.getTimeStamp();
        auto ticks = timestampInMillis * ticksPerMilliSecond;

        if (!message.isTimeSignatureMetaEvent() && !message.isTempoMetaEvent())
            tpqMidiSequence.addEvent(message.withTimeStamp(ticks));
    }

    juce::MidiFile tpqMidiFile;
    tpqMidiFile.setTicksPerQuarterNote(ticksPerQuarterNote);
    tpqMidiFile.addTrack(tpqMidiSequence);

    return saveMidiFile(tpqMidiFile, noOfNoteOns, durationMs, "");
}

/**
 * Saves the midiSequence with SMPTE timestamps.
 * Unlike 'saveTpqMidiFile' it does not modify the messages.
 * However smpte support seems to be limited. Reaper cannot read them well.
 */
bool Store::saveSmpteMidiFile(int noOfNoteOns, int durationMs)
{
    juce::MidiFile smpteMidiFile;

    // framesPerSecond=25 and subframeResolution=40 gives us 24*50=1000 millisecond resolution
    // So the message timestamps must be absolute times in millis.
    smpteMidiFile.setSmpteTimeFormat(25, 40);
    smpteMidiFile.addTrack(mMidiSequence);

    return saveMidiFile(smpteMidiFile, noOfNoteOns, durationMs, "_smpte");
}

bool Store::prepareAndSaveLastMidi()
{
    // Before starting read every available midi message
    drainProcessorMidiQueue();

    if (mMidiSequence.getNumEvents() == 0)
        return true; // Success. Nothing to do.

    // Update the Note ON & OFF pairs
    mMidiSequence.updateMatchedPairs();

    // Get number of note on messages. And calculate the duration.
    int numNoteOns = mRecordingTracker.getNumberOfTotalNoteOns();
    int durationMs = int(mMidiSequence.getEndTime() - mMidiSequence.getStartTime());

    if (numNoteOns < mState.getMinNoOfNotes() || durationMs < mState.getMinDurationMs())
    {
        // Message too small. Dropping it.
        reset();

        return true;
    }

    // Timestamps are millisecond durations; since the Processor::processBlock started to run till the midi message arrived.
    mMidiSequence.addTimeToMessages(mState.getPredelayMs() - mMidiSequence.getStartTime());

    // Save the midi file
    bool filesSaved = false;
    if (mState.getMidiTimeFormat() == "TPQ")
        filesSaved = saveTpqMidiFile(numNoteOns, durationMs);
    else
        filesSaved = saveSmpteMidiFile(numNoteOns, durationMs);

    if (filesSaved)
        reset();

    return filesSaved;
}

juce::String Store::getEventString(const juce::MidiMessage &m)
{
    if (m.isNoteOn())
        return "Note on";
    if (m.isNoteOff())
        return "Note off";
    if (m.isProgramChange())
        return "Program change";
    if (m.isPitchWheel())
        return "Pitch wheel";
    if (m.isAftertouch())
        return "Aftertouch";
    if (m.isChannelPressure())
        return "Channel pressure";
    if (m.isAllNotesOff())
        return "All notes off";
    if (m.isAllSoundOff())
        return "All sound off";
    if (m.isMetaEvent())
        return "Meta event";

    if (m.isController())
    {
        const auto *name = juce::MidiMessage::getControllerName(m.getControllerNumber());
        return "Controller " + (name == nullptr ? juce::String(m.getControllerNumber()) : juce::String(name));
    }

    return juce::String::toHexString(m.getRawData(), m.getRawDataSize());
}

juce::String Store::getDataString(const juce::MidiMessage &m)
{
    if (m.isNoteOn())
        return juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + " Velocity " + juce::String(m.getVelocity());
    if (m.isNoteOff())
        return juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + " Velocity " + juce::String(m.getVelocity());
    if (m.isProgramChange())
        return juce::String(m.getProgramChangeNumber());
    if (m.isPitchWheel())
        return juce::String(m.getPitchWheelValue());
    if (m.isAftertouch())
        return juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + juce::String(m.getAfterTouchValue());
    if (m.isChannelPressure())
        return juce::String(m.getChannelPressureValue());
    if (m.isController())
        return juce::String(m.getControllerValue());

    return {};
}

void Store::drainProcessorMidiQueue()
{
    std::vector<WrappedMessage> messages = mProcessor.popMidiQueue();

    if (messages.size() == 0)
        return;

    bool recording = false;

    for (auto it = messages.begin(); it != messages.end(); ++it)
    {
        juce::MidiMessage &message = it->message;

        if (it->isPlayback)
        {
            mPlaybackTracker.track(message);
        }
        else
        {
            mLastNoteReceivedTimeMs = juce::Time::currentTimeMillis();
            mRecordingTracker.track(message);
            mMidiSequence.addEvent(message, 0);
            recording = true;
        }

        // Keyboard state is combined state of the playback and recording trackers
        if (message.isNoteOn())
            mKeyboardState.noteOn(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            mKeyboardState.noteOff(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    }

    if (recording)
        mState.setIsRecordingInProgress(true);
}

void Store::timerCallback()
{
    if (mMidiSequence.getNumEvents() == 0)
        return;

    int minSilenceMs = mState.getMinSilenceMs();

    if (mRecordingTracker.hasActiveNotes())
        minSilenceMs *= mState.getMinSilenceMultiplier();

    if ((juce::Time::currentTimeMillis() - mLastNoteReceivedTimeMs) < minSilenceMs)
        return;

    prepareAndSaveLastMidi();
}

juce::File Store::getRootDataDir()
{
    auto rootDir = juce::File(mState.getRootDataDir());

    if (!rootDir.exists())
        rootDir.createDirectory();

    return rootDir;
}

MessageTracker &Store::getRecordingTracker()
{
    return mRecordingTracker;
}

MessageTracker &Store::getPlaybackTracker()
{
    return mPlaybackTracker;
}

juce::MidiKeyboardState &Store::getKeyboardState()
{
    return mKeyboardState;
}

void Store::reset()
{
    mMidiSequence.clear();

    mRecordingTracker.reset();
    mPlaybackTracker.reset();

    mState.setIsRecordingInProgress(false);

    // Reset the keyboard state
    mKeyboardState.allNotesOff(0);
    mKeyboardState.reset();
}