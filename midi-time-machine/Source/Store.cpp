#include "Store.h"

const int POLL_TIME_MILLIS = 1000;

Store::Store(Processor &processor) : processor(processor), state(processor.getState())
{
    // Start listening the state for changes
    state.addListener(this);

    // Start the timer
    startTimer(POLL_TIME_MILLIS);
}

Store::~Store()
{
    // And stop listening the state
    state.removeListener(this);
}

bool Store::saveMidiFile(
    const juce::File &rootDataDir,
    const juce::MidiFile &midiFile,
    int noOfNoteOns, int durationMs,
    const juce::String &filenamePostfix)
{
    juce::Time now = juce::Time::getCurrentTime();

    juce::String yearAndMonth;
    yearAndMonth
        << juce::String(now.getYear()).paddedLeft('0', 4) << "-"
        << juce::String(now.getMonth() + 1).paddedLeft('0', 2); // Month is 0-based

    juce::MemoryOutputStream filename;
    filename
        << yearAndMonth << "-"
        << juce::String(now.getDayOfMonth()).paddedLeft('0', 2) << "T"
        << juce::String(now.getHours()).paddedLeft('0', 2) << ":"
        << juce::String(now.getMinutes()).paddedLeft('0', 2) << ":"
        << juce::String(now.getSeconds()).paddedLeft('0', 2) << " "

        << juce::String(now.getWeekdayName(false)) << " "
        << juce::String(noOfNoteOns) << " notes "
        << juce::String(juce::roundToInt(durationMs / 1000)) << " seconds"
        << filenamePostfix
        << juce::String(".mid");

    juce::File parentDir = rootDataDir.getChildFile(yearAndMonth);

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
    return midiFile.writeTo(stream);
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
    int microsPerQuarterNote = (60. / beatsPerMinute) * 1000000; // 500000 =>  0.5 seconds for 4/4 tempo and 120 bpm
    int ticksPerQuarterNote = 960;
    double ticksPerMilliSecond = ticksPerQuarterNote * (beatsPerMinute / 60.) / 1000.0; // for 4/4 tempo

    juce::MidiMessageSequence tpqMidiSequence;
    tpqMidiSequence.addEvent(juce::MidiMessage::tempoMetaEvent(microsPerQuarterNote), 0);
    tpqMidiSequence.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4), 0);

    for (int i = 0; i < midiSequence.getNumEvents(); ++i)
    {
        auto &message = midiSequence.getEventPointer(i)->message;
        auto timestampInMillis = message.getTimeStamp();
        auto ticks = timestampInMillis * ticksPerMilliSecond;

        if (!message.isTimeSignatureMetaEvent() && !message.isTempoMetaEvent())
            tpqMidiSequence.addEvent(message.withTimeStamp(ticks));
    }

    juce::MidiFile tpqMidiFile;
    tpqMidiFile.setTicksPerQuarterNote(ticksPerQuarterNote);
    tpqMidiFile.addTrack(tpqMidiSequence);

    return saveMidiFile(getRootDataDir(), tpqMidiFile, noOfNoteOns, durationMs, "");
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
    smpteMidiFile.addTrack(midiSequence);

    bool fileSaved = saveMidiFile(getRootDataDir(), smpteMidiFile, noOfNoteOns, durationMs, "_smpte");

    if (fileSaved)
    {
        lastSavedSmpteFile = smpteMidiFile;
        state.setMidiFileAvailable(true);
    }

    return fileSaved;
}

bool Store::prepareAndSaveLastMidi()
{
    // Before starting read every available midi message
    drainProcessorMidiQueue();

    if (midiSequence.getNumEvents() == 0)
        return true; // Success. Nothing to do.

    // Update the Note ON & OFF pairs
    midiSequence.updateMatchedPairs();

    // Timestamps are millisecond durations; since the Processor::processBlock started to run till the midi message arrived.
    midiSequence.addTimeToMessages(state.getPredelayMs() - midiSequence.getStartTime());

    // Count note on messages. And calculate the duration.
    int noOfNoteOns = 0;
    int durationMs = midiSequence.getEndTime() - midiSequence.getStartTime();

    for (int i = 0; i < midiSequence.getNumEvents(); ++i)
    {
        if (midiSequence.getEventPointer(i)->message.isNoteOn())
            ++noOfNoteOns;
    }

    // Save two midi files; one with ticks per quarter note timestamps and another with SMPTE timestamps.
    bool filesSaved = saveTpqMidiFile(noOfNoteOns, durationMs) && saveSmpteMidiFile(noOfNoteOns, durationMs);

    if (filesSaved)
        midiSequence.clear();

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

void Store::valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property)
{
    if (state.isMidiMessagesAvailableChange(tree, property) && state.isMidiMessagesAvailable())
    {
        state.setMidiMessagesAvailable(false);
        drainProcessorMidiQueue();
    }
}

void Store::drainProcessorMidiQueue()
{
    std::vector<juce::MidiMessage> messages = processor.popMidiQueue();

    if (messages.size() == 0)
        return;

    lastQueueDrainedTimeMs = juce::Time::currentTimeMillis();

    for (auto it = messages.begin(); it != messages.end(); ++it)
        midiSequence.addEvent(*it, 0);
}

void Store::timerCallback()
{
    if (midiSequence.getNumEvents() == 0)
        return;

    if ((juce::Time::currentTimeMillis() - lastQueueDrainedTimeMs) < state.getMinSilenceMs())
        return;

    prepareAndSaveLastMidi();
}

Store::MaybeMidiFile Store::getLastSavedMidiFile()
{
    return lastSavedSmpteFile;
}

juce::File Store::getRootDataDir()
{
    auto rootDir = juce::File(state.getRootDataDir());

    if (!rootDir.exists())
        rootDir.createDirectory();

    return rootDir;
}