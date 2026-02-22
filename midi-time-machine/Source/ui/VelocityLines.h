#pragma once

#include <JuceHeader.h>
#include "../Store.h"

class VelocityLines : public juce::Component,
                      public juce::MidiKeyboardStateListener,
                      public juce::ChangeListener
{
public:
    VelocityLines(Store &store, juce::MidiKeyboardComponent &keyboardComponent)
        : mRecordingTracker(store.getRecordingTracker()),
          mPlaybackTracker(store.getPlaybackTracker()),
          mKeyboardState(store.getKeyboardState()),
          mKeyboardComponent(keyboardComponent)
    {
        mKeyboardState.addListener(this);
        keyboardComponent.addChangeListener(this);
    }

    ~VelocityLines() override
    {
        mKeyboardState.removeListener(this);
        mKeyboardComponent.removeChangeListener(this);
    }

    void paint(juce::Graphics &g) override
    {
        int rangeStart = mKeyboardComponent.getRangeStart();
        int rangeEnd = mKeyboardComponent.getRangeEnd();
        float keyWidth = mKeyboardComponent.getKeyWidth();
        float lineWidth = keyWidth * 0.50f;

        for (int n = rangeStart; n <= rangeEnd; ++n)
        {
            float velocity = juce::jmax(mRecordingTracker.getNoteVelocity(n), mPlaybackTracker.getNoteVelocity(n));

            if (velocity > 0.0f)
            {
                juce::Rectangle<float> rect = mKeyboardComponent.getRectangleForKey(n);

                // Convert keyboard relative coordinates to this component's coordinates
                juce::Point<float> pos = getLocalPoint(&mKeyboardComponent, rect.getCentre());

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
    MessageTracker &mRecordingTracker;
    MessageTracker &mPlaybackTracker;
    juce::MidiKeyboardState &mKeyboardState;
    juce::MidiKeyboardComponent &mKeyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelocityLines)
};
