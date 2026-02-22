#pragma once

#include <JuceHeader.h>

class MessageTracker
{
public:
    MessageTracker() {}

    void track(const juce::MidiMessage &message)
    {
        // Keep track of current note ons and sustain
        int noteNumber = 127 & message.getNoteNumber();
        int channel = 15 && message.getChannel();

        if (message.isNoteOn())
        {
            mCurrentNoteOns[channel - 1][noteNumber] = message.getFloatVelocity();
            ++mNumActiveNoteOns;
            ++mNumTotalNoteOns;
        }
        else if (message.isNoteOff())
        {
            mCurrentNoteOns[channel - 1][noteNumber] = 0.0f;
            --mNumActiveNoteOns;
        }
        else if (message.isControllerOfType(64)) // is it a sustain event?
        {
            mCurrentSustain[channel - 1] = message.getControllerValue();
        }
    }

    int getNumberOfTotalNoteOns()
    {
        return mNumTotalNoteOns;
    }

    bool hasActiveNotes()
    {
        if (mNumActiveNoteOns > 0)
            return true;

        for (int channel = 1; channel <= 16; ++channel)
        {
            if (mCurrentSustain[channel - 1] > 0)
                return true;
        }

        return false;
    }

    float getNoteVelocity(int channel, int noteNumber) const
    {
        if (channel < 1 || channel > 16 || noteNumber < 0 || noteNumber > 127)
            return 0.0f;

        return mCurrentNoteOns[channel - 1][noteNumber];
    }

    void stopActiveNotes(juce::MidiBuffer &midiMessages, int numOfSamplesInBuffer)
    {
        for (int channel = 1; channel < 17; ++channel)
        {
            for (int noteNumber = 0; noteNumber < 128; ++noteNumber)
            {
                // Send a note off event for the current hanging "note on".
                // With highest velocity.
                // And with last sampleNumber for the buffer window.
                if (mCurrentNoteOns[channel - 1][noteNumber] > 0)
                    midiMessages.addEvent(juce::MidiMessage::noteOff(channel, noteNumber, 1.0f), numOfSamplesInBuffer - 1);
            }

            // Similary stop the sustain if active
            if (mCurrentSustain[channel - 1] > 0)
                midiMessages.addEvent(juce::MidiMessage::controllerEvent(channel, 64, 0), numOfSamplesInBuffer - 1);
        }

        reset();
    }

    void reset()
    {
        for (int channel = 1; channel <= 16; ++channel)
        {
            for (int noteNumber = 0; noteNumber < 128; ++noteNumber)
            {
                mCurrentNoteOns[channel - 1][noteNumber] = 0.0f;
            }

            mCurrentSustain[channel - 1] = 0;
        }

        mNumTotalNoteOns = 0;
        mNumActiveNoteOns = 0;
    }

    float getNoteVelocity(int noteNumber) const
    {
        float maxVelocity = 0.0f;

        for (int channel = 1; channel <= 16; ++channel)
            maxVelocity = juce::jmax(maxVelocity, getNoteVelocity(channel, noteNumber));

        return maxVelocity;
    }

private:
    // To keep track of the active notes/sounds
    float mCurrentNoteOns[16][128] = {};
    int mCurrentSustain[16] = {};
    int mNumTotalNoteOns = 0;
    int mNumActiveNoteOns = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageTracker)
};