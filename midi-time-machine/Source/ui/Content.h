#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "../Store.h"
#include "Settings.h"

class Content : public juce::Component,
                public juce::ValueTree::Listener
{
public:
    Content(State &stateArg, Store &store) : state(stateArg), store(store), settings(stateArg)
    {
        addChildComponent(settings);
        settings.setVisible(state.isSettinsOpen());

        state.addListener(this);
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

        auto bounds = getTopLevelComponent()->getLocalBounds();

        juce::String text;
        text << "Window size: " << bounds.getWidth() << "x" << bounds.getHeight();

        g.drawFittedText(text, getLocalBounds(), juce::Justification::centred, 1);
    }

    void resized() override
    {
        juce::Rectangle bounds = getLocalBounds();
        juce::Rectangle topLeft(bounds);

        topLeft.setWidth(400);
        topLeft.setHeight(250);
        topLeft.setX(bounds.getX() + bounds.getWidth() - topLeft.getWidth());

        if (topLeft.getWidth() < bounds.getWidth() && topLeft.getHeight() < bounds.getHeight())
            settings.setBounds(topLeft);
        else
            settings.setBounds(bounds);
    }

private:
    State &state;
    Store &store;
    Settings settings;
};
