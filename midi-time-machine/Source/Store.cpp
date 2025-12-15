#include "Store.h"

Store::Store(Processor &processor)
    : processor(processor)
{
    // Start listening the processor for changes
    processor.addChangeListener(this);
}

Store::~Store()
{
    // And stop listening the processor
    processor.removeChangeListener(this);
}

bool Store::saveMidiFile(const juce::MidiFile &midiFile, const juce::File &destination)
{
    juce::FileOutputStream stream(destination);

    if (stream.openedOk())
    {
        stream.setPosition(0);
        stream.truncate();
    }

    return midiFile.writeTo(stream);
}

bool Store::prepareAndSaveLastMidi()
{
    // Save first midi file in SMPT time format (with absolute message timestamps)
    juce::MidiFile smptMidiFile;

    // framesPerSecond=25 and subframeResolution=40 gives us 24*50=1000 millisecond resolution
    // So the message timestamps must be absolute times in millis
    smptMidiFile.setSmpteTimeFormat(25, 40);
    smptMidiFile.addTrack(midiSequence);

    bool filesSaved = saveMidiFile(smptMidiFile, juce::File("~/tmp/midi_file_absolute_times.mid"));

    // Save the second midi file in ticks per quarter time timestamps
    // This converts the whole midi sequence into 120bmp, 4/4 tempo, 960 ticksPerQuarterNote.
    // And it inserts its own time signature and tempo meta events.
    // It also drops any existing time signature and tempo meta events.

    // with ticks per quarter note
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

    juce::MidiFile newFile;
    newFile.setTicksPerQuarterNote(ticksPerQuarterNote);
    newFile.addTrack(tpqMidiSequence);

    filesSaved = filesSaved && saveMidiFile(newFile, juce::File("~/tmp/midi_file_ticks.mid"));

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

void Store::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    std::vector<juce::MidiMessage> messages = processor.popMidiQueue();

    for (auto it = messages.begin(); it != messages.end(); ++it)
    {
        std::cout << getEventString(*it) << " : " << getDataString(*it) << " : " << it->getTimeStamp() << std::endl;
        midiSequence.addEvent(*it, 0);
    }

    midiSequence.updateMatchedPairs();
}
