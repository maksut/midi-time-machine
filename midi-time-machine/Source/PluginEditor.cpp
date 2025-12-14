#include "PluginProcessor.h"
#include "PluginEditor.h"

MTMAudioProcessorEditor::MTMAudioProcessorEditor(MTMAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    saveButton.setButtonText("Save");
    saveButton.addListener(this);
    addAndMakeVisible(saveButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);

    // Start listening the processor for changes
    audioProcessor.addChangeListener(this);
}

MTMAudioProcessorEditor::~MTMAudioProcessorEditor()
{
    // And stop listening the processor
    audioProcessor.removeChangeListener(this);
}

void MTMAudioProcessorEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    g.drawFittedText("Hello World! Hebele", getLocalBounds(), juce::Justification::centred, 1);
}

void MTMAudioProcessorEditor::resized()
{
    auto windowBounds = getLocalBounds();
    saveButton.setBounds(windowBounds.withSizeKeepingCentre(100, 30).translated(0, 50));
}

bool saveMidiFile(const juce::MidiFile &midiFile, const juce::File &destination)
{
    juce::FileOutputStream stream(destination);

    if (stream.openedOk())
    {
        stream.setPosition(0);
        stream.truncate();
    }

    return midiFile.writeTo(stream);
}

void MTMAudioProcessorEditor::buttonClicked(juce::Button *button)
{
    if (button == &saveButton)
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

        juce::String msg = filesSaved ? "Midi files are saved." : "Midi file is failed to save!";
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::NoIcon, "Midi file save", msg);
    }
}

juce::String getEventString(const juce::MidiMessage &m)
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

static juce::String getDataString(const juce::MidiMessage &m)
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

void MTMAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    std::vector<juce::MidiMessage> messages = audioProcessor.popMidiQueue();

    for (auto it = messages.begin(); it != messages.end(); ++it)
    {
        std::cout << getEventString(*it) << " : " << getDataString(*it) << " : " << it->getTimeStamp() << std::endl;
        midiSequence.addEvent(*it, 0);
    }

    midiSequence.updateMatchedPairs();
}
