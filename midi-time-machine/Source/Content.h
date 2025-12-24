#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "Store.h"

class Content : public juce::Component,
                public juce::ValueTree::Listener
{
public:
    Content(State &state, Store &store) : state(state), store(store)
    {
        addAndMakeVisible(testButton);
        state.addListener(this);
    }

    ~Content() override
    {
        state.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
    }

    void paint(juce::Graphics &g)
    {
        g.fillAll(juce::Colours::grey);

        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(15.0f));

        auto bounds = getTopLevelComponent()->getLocalBounds();

        juce::String text;
        text << "Window size: " << bounds.getWidth() << "x" << bounds.getHeight();

        g.drawFittedText(text, getLocalBounds(), juce::Justification::centred, 1);
    }

private:
    State &state;
    Store &store;

    juce::DrawableButton testButton{"Test", juce::DrawableButton::ImageOnButtonBackground};
};
