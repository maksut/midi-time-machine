#pragma once

#include <JuceHeader.h>
#include "VelocityLines.h"
#include "RoundedComponent.h"

class Keyboard : public RoundedComponent
{
public:
    Keyboard(Store &store)
        : midiKeyboard(store.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard),
          velocityLines(store, midiKeyboard)
    {
        backgroundColour = juce::Colours::darkgrey;

        midiKeyboard.setName("MIDI Keyboard");
        midiKeyboard.setLowestVisibleKey(21 /*A0*/);
        midiKeyboard.setAvailableRange(21 /*A0*/, 108 /*C8*/);
        midiKeyboard.setOctaveForMiddleC(4);

        addAndMakeVisible(midiKeyboard);
        addAndMakeVisible(velocityLines);
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRect(getLocalBounds());
    }

    void resized() override
    {
        int keyboardHeight = 60;
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
