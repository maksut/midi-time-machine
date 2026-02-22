#pragma once

#include <JuceHeader.h>
#include "State.h"

class ValueTreeLogger : public juce::ValueTree::Listener
{
public:
    ValueTreeLogger(State &state) : mState(state)
    {
#ifdef JUCE_DEBUG
        state.addListener(this);
#endif
    }

    ~ValueTreeLogger()
    {
#ifdef JUCE_DEBUG
        mState.removeListener(this);
#endif
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        // There can be a lot of playback time changes. 60Hz
        if (!mState.isPlaybackTimeSecChange(tree, property))
            juce::Logger::writeToLog(property.toString() + "=" + tree.getProperty(property).toString());
    }

private:
    State &mState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueTreeLogger)
};