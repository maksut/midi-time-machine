#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "../Store.h"
#include "Settings.h"
#include "Keyboard.h"
#include "MidiRoll.h"

const int RECORDING_BLINK_TIMER = 500;

class Content : public juce::Component,
                public juce::ValueTree::Listener,
                public juce::Timer
{
public:
    Content(State &state, Store &store, MidiRoll &midiRoll) : mState(state), mStore(store), mMidiRoll(midiRoll), mSettings(state), mKeyboard(store)
    {
        state.addListener(this);

        mMidiFileName.setFont(mMidiFileName.getFont().withHeight(18));
        mMidiFileName.setAlpha(0.5);
        resetMidiFileName();

        resetIsRecordingInProgress();

        addAndMakeVisible(midiRoll);
        addAndMakeVisible(mMidiFileName);

        addAndMakeVisible(mKeyboard);

        // Adding settings last so it will be on front
        addChildComponent(mSettings);
        mSettings.setVisible(state.isSettinsOpen());
    }

    ~Content() override
    {
        mState.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (mState.isSelectedMidiFileChange(tree, property))
        {
            resetMidiFileName();
            resized();
            repaint();
        }
        else if (mState.isSettinsOpenChange(tree, property))
        {
            bool isOpen = mState.isSettinsOpen();

            if (isOpen)
                mSettings.reloadSettings();

            mSettings.setVisible(isOpen);
        }
        else if (mState.isPanelHeightRatioChange(tree, property))
        {
            mPanelHeigthRatio = mState.getPanelHeightRatio();
            resized();
            repaint();
        }
        else if (mState.isRecordingInProgressChange(tree, property))
        {
            resetIsRecordingInProgress();
            repaint();
        }
    }

    void timerCallback() override
    {
        mDisplayRecordingDot = !mDisplayRecordingDot;
        repaint();
    }

    void paintOverChildren(juce::Graphics &g) override
    {
        if (mDisplayRecordingDot)
        {
            g.setColour(juce::Colours::red.withAlpha(0.6f));

            juce::Rectangle<int> bounds = getLocalBounds();
            float size = bounds.getWidth() / 40.0f;
            juce::Rectangle<float> rect(size, size, size, size);
            g.fillEllipse(rect);
        }
    }

    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colours::darkgrey);

        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(15.0f));
    }

    void resized() override
    {
        juce::Rectangle bounds = getLocalBounds();

        // Settings panel
        juce::Rectangle topLeft(bounds);

        topLeft.setWidth(400);
        topLeft.setHeight(275);
        topLeft.setX(bounds.getX() + bounds.getWidth() - topLeft.getWidth());

        if (topLeft.getWidth() < bounds.getWidth() && topLeft.getHeight() < bounds.getHeight())
            mSettings.setBounds(topLeft);
        else
            mSettings.setBounds(bounds);

        // Filename
        juce::TextLayout layout;
        juce::AttributedString attributedString;
        attributedString.append(mMidiFileName.getText(), mMidiFileName.getFont());
        layout.createLayout(attributedString, bounds.getWidth());

        int labelWidth = (int)layout.getWidth();
        int labelHeight = (int)layout.getHeight();

        mMidiFileName.setBounds((bounds.getWidth() - labelWidth) / 2,
                                4,
                                labelWidth,
                                labelHeight);

        // Midi preview
        int margin = 5;
        int panelWidth = bounds.getWidth() - (2 * margin);

        int previewHeight = (bounds.getHeight() - 60) * mPanelHeigthRatio; // 60 keyboard size

        juce::Rectangle<int> previewBounds(
            margin,
            margin,
            panelWidth,
            previewHeight - (2 * margin));

        mMidiRoll.setBounds(previewBounds);

        // Keyboard panel
        juce::Rectangle<int> keyboardBounds(
            margin,
            previewHeight,
            panelWidth,
            getHeight() - previewHeight - margin);

        mKeyboard.setBounds(keyboardBounds);
    }

private:
    void resetMidiFileName()
    {
        juce::File file(mState.getSelectedMidiFile());
        juce::String filename = file.existsAsFile() ? file.getFileNameWithoutExtension() : "";

        mMidiFileName.setText(filename, juce::dontSendNotification);
    }

    void resetIsRecordingInProgress()
    {
        if (mState.isRecordingInProgress())
        {
            startTimer(RECORDING_BLINK_TIMER);
            mDisplayRecordingDot = true;
        }
        else
        {
            stopTimer();
            mDisplayRecordingDot = false;
        }
    }

private:
    State &mState;
    Store &mStore;
    MidiRoll &mMidiRoll;
    Settings mSettings;
    Keyboard mKeyboard;

    float mPanelHeigthRatio = 0.5;
    bool mDisplayRecordingDot = false;

    juce::Label mMidiFileName{"MidiFileName", ""};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Content)
};
