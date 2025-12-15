#include "Editor.h"

Editor::Editor(Processor &processor, Store &midiStore)
    : AudioProcessorEditor(&processor), processor(processor), store(midiStore)
{
    saveButton.setButtonText("Save");
    saveButton.addListener(this);
    addAndMakeVisible(saveButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);

    // Start listening the processor for changes
    processor.addChangeListener(this);
}

Editor::~Editor()
{
    // And stop listening the processor
    processor.removeChangeListener(this);
}

void Editor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    g.drawFittedText("Hello World! Hebele", getLocalBounds(), juce::Justification::centred, 1);
}

void Editor::resized()
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

void Editor::buttonClicked(juce::Button *button)
{
    if (button == &saveButton)
    {
        juce::String msg = store.prepareAndSaveLastMidi() ? "Midi files are saved." : "Midi file is failed to save!";

        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::NoIcon, "Midi file save", "Save button clicked");
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

void Editor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
}
