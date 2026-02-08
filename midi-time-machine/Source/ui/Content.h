#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "../Store.h"
#include "Settings.h"
#include "Keyboard.h"

class Content : public juce::Component,
                public juce::ValueTree::Listener
{
public:
    Content(State &state, Store &store) : state(state), store(store), settings(state), keyboard(store)
    {
        state.addListener(this);

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
        if (state.isSettinsOpenChange(tree, property))
        {
            bool isOpen = state.isSettinsOpen();

            if (isOpen)
                settings.reloadSettings();

            settings.setVisible(isOpen);
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

        // Keyboard panel
        int keyboardMargin = 10;
        keyboard.setBounds(bounds.withSizeKeepingCentre(bounds.getWidth() - keyboardMargin, bounds.getHeight() - keyboardMargin));
    }

private:
    State &state;
    Store &store;
    Settings settings;
    Keyboard keyboard;
};
