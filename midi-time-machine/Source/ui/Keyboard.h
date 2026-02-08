#pragma once

#include <JuceHeader.h>
#include "VelocityLines.h"

class Keyboard : public juce::Component
{
public:
    Keyboard(MessageTracker &messageTracker) : midiKeyboard(messageTracker.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard),
                                               velocityLines(messageTracker, midiKeyboard)
    {
        midiKeyboard.setName("MIDI Keyboard");
        midiKeyboard.setLowestVisibleKey(21 /*A0*/);
        midiKeyboard.setAvailableRange(21 /*A0*/, 108 /*C8*/);

        addAndMakeVisible(midiKeyboard);
        addAndMakeVisible(velocityLines);
    }

    void resized() override
    {
        int keyboardHeight = 80;
        int velocityHeight = getHeight() - keyboardHeight;

        midiKeyboard.setBounds(0, getHeight() - keyboardHeight, getWidth(), keyboardHeight);
        velocityLines.setBounds(0, getHeight() - keyboardHeight - velocityHeight, getWidth(), velocityHeight);

        midiKeyboard.setKeyWidth(juce::jmax(10.0f, getWidth() / 52.0f /*noOfWhiteKeys*/));
    }

private:
    juce::MidiKeyboardComponent midiKeyboard;
    VelocityLines velocityLines;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
};
