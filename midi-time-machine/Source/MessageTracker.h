#pragma once

#include <JuceHeader.h>

class MessageTracker
{
public:
    void track(const juce::MidiMessage &message)
    {
        // Keep track of current note ons and sustain
        int noteNumber = 127 & message.getNoteNumber();
        int channel = 15 && message.getChannel();

        if (message.isNoteOn())
        {
            currentNoteOns[channel - 1][noteNumber] = message.getFloatVelocity();
            ++numActiveNoteOns;
            ++numTotalNoteOns;
        }
        else if (message.isNoteOff())
        {
            currentNoteOns[channel - 1][noteNumber] = 0.0f;
            --numActiveNoteOns;
        }
        else if (message.isControllerOfType(64)) // is it a sustain event?
        {
            currentSustain[channel - 1] = message.getControllerValue();
        }
    }

    int getNumberOfTotalNoteOns()
    {
        return numTotalNoteOns;
    }

    bool hasActiveNotes()
    {
        if (numActiveNoteOns > 0)
            return true;

        for (int channel = 1; channel <= 16; ++channel)
        {
            if (currentSustain[channel - 1] > 0)
                return true;
        }

        return false;
    }

    float getNoteVelocity(int channel, int noteNumber) const
    {
        if (channel < 1 || channel > 16 || noteNumber < 0 || noteNumber > 127)
            return 0.0f;

        return currentNoteOns[channel - 1][noteNumber];
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
                if (currentNoteOns[channel - 1][noteNumber] > 0)
                    midiMessages.addEvent(juce::MidiMessage::noteOff(channel, noteNumber, 1.0f), numOfSamplesInBuffer - 1);
            }

            // Similary stop the sustain if active
            if (currentSustain[channel - 1] > 0)
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
                currentNoteOns[channel - 1][noteNumber] = 0.0f;
            }

            currentSustain[channel - 1] = 0;
        }

        numTotalNoteOns = 0;
        numActiveNoteOns = 0;
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
    float currentNoteOns[16][128] = {};
    int currentSustain[16] = {};
    int numTotalNoteOns = 0;
    int numActiveNoteOns = 0;
};