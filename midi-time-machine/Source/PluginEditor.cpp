#include "PluginProcessor.h"
#include "PluginEditor.h"

MTMAudioProcessorEditor::MTMAudioProcessorEditor (MTMAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

MTMAudioProcessorEditor::~MTMAudioProcessorEditor()
{
}

void MTMAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World! Hebele", getLocalBounds(), juce::Justification::centred, 1);
}

void MTMAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

juce::String getEventString (const juce::MidiMessage& m)
{
    if (m.isNoteOn())           return "Note on";
    if (m.isNoteOff())          return "Note off";
    if (m.isProgramChange())    return "Program change";
    if (m.isPitchWheel())       return "Pitch wheel";
    if (m.isAftertouch())       return "Aftertouch";
    if (m.isChannelPressure())  return "Channel pressure";
    if (m.isAllNotesOff())      return "All notes off";
    if (m.isAllSoundOff())      return "All sound off";
    if (m.isMetaEvent())        return "Meta event";

    if (m.isController())
    {
        const auto* name = juce::MidiMessage::getControllerName(m.getControllerNumber());
        return "Controller " + (name == nullptr ? juce::String(m.getControllerNumber()) : juce::String(name));
    }

    return juce::String::toHexString (m.getRawData(), m.getRawDataSize());
}

static juce::String getDataString (const juce::MidiMessage& m)
{
    if (m.isNoteOn())           return juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) + " Velocity " + juce::String (m.getVelocity());
    if (m.isNoteOff())          return juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) + " Velocity " + juce::String (m.getVelocity());
    if (m.isProgramChange())    return juce::String (m.getProgramChangeNumber());
    if (m.isPitchWheel())       return juce::String (m.getPitchWheelValue());
    if (m.isAftertouch())       return juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + juce::String (m.getAfterTouchValue());
    if (m.isChannelPressure())  return juce::String (m.getChannelPressureValue());
    if (m.isController())       return juce::String (m.getControllerValue());

    return {};
}

void MTMAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    std::vector<juce::MidiMessage> messages = audioProcessor.popMidiQueue();
   
    for (auto it = messages.begin(); it != messages.end(); ++it)
        std::cout << getEventString(*it) << " : " << getDataString(*it) << " : " << it->getTimeStamp() << std::endl;
}
