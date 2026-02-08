#pragma once

#include <JuceHeader.h>
#include "../Store.h"

class VelocityLines : public juce::Component,
                      public juce::MidiKeyboardStateListener,
                      public juce::ChangeListener
{
public:
    VelocityLines(Store &store, juce::MidiKeyboardComponent &keyboardComponent)
        : recordingTracker(store.getRecordingTracker()),
          playbackTracker(store.getPlaybackTracker()),
          keyboardState(store.getKeyboardState()),
          keyboardComponent(keyboardComponent)
    {
        keyboardState.addListener(this);
        keyboardComponent.addChangeListener(this);
    }

    ~VelocityLines() override
    {
        keyboardState.removeListener(this);
        keyboardComponent.removeChangeListener(this);
    }

    void paint(juce::Graphics &g) override
    {
        int rangeStart = keyboardComponent.getRangeStart();
        int rangeEnd = keyboardComponent.getRangeEnd();
        float keyWidth = keyboardComponent.getKeyWidth();
        float lineWidth = keyWidth * 0.50f;

        for (int n = rangeStart; n <= rangeEnd; ++n)
        {
            float velocity = juce::jmax(recordingTracker.getNoteVelocity(n), playbackTracker.getNoteVelocity(n));

            if (velocity > 0.0f)
            {
                juce::Rectangle<float> rect = keyboardComponent.getRectangleForKey(n);

                // Convert keyboard relative coordinates to this component's coordinates
                juce::Point<float> pos = getLocalPoint(&keyboardComponent, rect.getCentre());

                float lineMaxHeight = (float)getHeight();
                float lineHeight = velocity * lineMaxHeight;

                g.setColour(juce::Colours::orange.withAlpha(0.8f));
                g.drawLine(pos.getX(), (float)getHeight(), pos.getX(), (float)getHeight() - lineHeight, lineWidth);
            }
        }
    }

    // MidiKeyboardStateListener
    void handleNoteOn(juce::MidiKeyboardState *, int, int, float) override { repaint(); }
    void handleNoteOff(juce::MidiKeyboardState *, int, int, float) override { repaint(); }

    // ChangeListener (for keyboard scrolling)
    void changeListenerCallback(juce::ChangeBroadcaster *) override { repaint(); }

private:
    MessageTracker &recordingTracker;
    MessageTracker &playbackTracker;
    juce::MidiKeyboardState &keyboardState;
    juce::MidiKeyboardComponent &keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelocityLines)
};
