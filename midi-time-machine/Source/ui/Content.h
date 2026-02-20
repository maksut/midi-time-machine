#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "../Store.h"
#include "Settings.h"
#include "Keyboard.h"
#include "MidiRoll.h"

class Content : public juce::Component,
                public juce::ValueTree::Listener
{
public:
    Content(State &state, Store &store, MidiRoll &midiRoll) : state(state), store(store), settings(state), keyboard(store), midiRoll(midiRoll)
    {
        state.addListener(this);

        midiFileName.setFont(midiFileName.getFont().withHeight(18));
        midiFileName.setAlpha(0.5);
        resetMidiFileName();

        addAndMakeVisible(midiFileName);
        addAndMakeVisible(midiRoll);

        addAndMakeVisible(keyboard);

        // Adding settings last so it will be on front
        addChildComponent(settings);
        settings.setVisible(state.isSettinsOpen());
    }

    ~Content() override
    {
        state.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (state.isSelectedMidiFileChange(tree, property))
        {
            resetMidiFileName();
            resized();
            repaint();
        }
        else if (state.isSettinsOpenChange(tree, property))
        {
            bool isOpen = state.isSettinsOpen();

            if (isOpen)
                settings.reloadSettings();

            settings.setVisible(isOpen);
        }
        else if (state.isPanelHeightRatioChange(tree, property))
        {
            panelHeigthRatio = state.getPanelHeightRatio();
            resized();
            repaint();
        }
    }

    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colours::darkgrey);

        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(15.0f));

        /*
        auto bounds = getTopLevelComponent()->getLocalBounds();

        juce::String text;
        text << "Window size: " << bounds.getWidth() << "x" << bounds.getHeight();

        g.drawFittedText(text, getLocalBounds(), juce::Justification::centred, 1);
        */
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
            settings.setBounds(topLeft);
        else
            settings.setBounds(bounds);

        // Filename
        juce::TextLayout layout;
        juce::AttributedString attributedString;
        attributedString.append(midiFileName.getText(), midiFileName.getFont());
        layout.createLayout(attributedString, bounds.getWidth());

        int labelWidth = (int)layout.getWidth();
        int labelHeight = (int)layout.getHeight();

        midiFileName.setBounds((bounds.getWidth() - labelWidth) / 2,
                               4,
                               labelWidth,
                               labelHeight);

        // Midi preview
        int margin = 5;
        int panelWidth = bounds.getWidth() - (2 * margin);

        int previewHeight = (bounds.getHeight() - 60) * panelHeigthRatio; // 60 keyboard size

        juce::Rectangle<int> previewBounds(
            margin,
            margin,
            panelWidth,
            previewHeight - (2 * margin));

        midiRoll.setBounds(previewBounds);

        // Keyboard panel
        juce::Rectangle<int> keyboardBounds(
            margin,
            previewHeight,
            panelWidth,
            getHeight() - previewHeight - margin);

        keyboard.setBounds(keyboardBounds);
    }

private:
    void resetMidiFileName()
    {
        juce::File file(state.getSelectedMidiFile());
        juce::String filename = file.existsAsFile() ? file.getFileNameWithoutExtension() : "";

        midiFileName.setText(filename, juce::dontSendNotification);
    }

private:
    State &state;
    Store &store;
    MidiRoll &midiRoll;
    Settings settings;
    Keyboard keyboard;
    float panelHeigthRatio = 0.5;

    juce::Label midiFileName{"MidiFileName", ""};
};
