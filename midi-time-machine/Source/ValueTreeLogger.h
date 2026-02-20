#pragma once

#include <JuceHeader.h>
#include "State.h"

class ValueTreeLogger : public juce::ValueTree::Listener
{
public:
    ValueTreeLogger(State &state) : state(state)
    {
        state.addListener(this);
    }

    ~ValueTreeLogger()
    {
        state.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        juce::Logger::writeToLog(property.toString() + "=" + tree.getProperty(property).toString());
    }

private:
    State &state;
};