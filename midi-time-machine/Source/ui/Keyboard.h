#pragma once

#include <JuceHeader.h>
#include "VelocityLines.h"
#include "RoundedComponent.h"

class Keyboard : public RoundedComponent
{
public:
    Keyboard(Store &store)
        : mMidiKeyboard(store.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard),
          mVelocityLines(store, mMidiKeyboard)
    {
        mBackgroundColour = juce::Colours::darkgrey;

        mMidiKeyboard.setName("MIDI Keyboard");
        mMidiKeyboard.setLowestVisibleKey(21 /*A0*/);
        mMidiKeyboard.setAvailableRange(21 /*A0*/, 108 /*C8*/);
        mMidiKeyboard.setOctaveForMiddleC(4);

        addAndMakeVisible(mMidiKeyboard);
        addAndMakeVisible(mVelocityLines);
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

        mMidiKeyboard.setBounds(0, getHeight() - keyboardHeight, getWidth(), keyboardHeight);
        mVelocityLines.setBounds(0, getHeight() - keyboardHeight - velocityHeight, getWidth(), velocityHeight);

        mMidiKeyboard.setKeyWidth(juce::jmax(10.0f, getWidth() / 52.0f /*noOfWhiteKeys*/));
    }

private:
    juce::MidiKeyboardComponent mMidiKeyboard;
    VelocityLines mVelocityLines;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
};
