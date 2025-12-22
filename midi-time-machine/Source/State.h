#pragma once

#include <JuceHeader.h>

class State
{
public:
    State()
    {
        // Set state properties
        state.setProperty(midiFileAvailableId, false, nullptr);
        state.setProperty(midiMessagesAvailableId, false, nullptr);

        // Set setting properties
        settings.setProperty(minSilenceId, 4000, nullptr);
        settings.setProperty(minSilenceMultiplierId, 5, nullptr);
        settings.setProperty(predelayId, 1000, nullptr);

        juce::String defaultRootDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                                          .getChildFile("midi_time_machine")
                                          .getFullPathName();

        settings.setProperty(rootDataDirId, defaultRootDir, nullptr);
        state.addChild(settings, -1, nullptr);
    }

    std::unique_ptr<juce::XmlElement> toXml() const
    {
        return state.createXml();
    }

    void fromXml(std::unique_ptr<juce::XmlElement> &xmlState)
    {
        auto newState = juce::ValueTree::fromXml(*xmlState);

        if (newState.isValid() && newState.hasType(stateId))
        {
            state.copyPropertiesAndChildrenFrom(newState, nullptr);

            // But we reset some of the state props
            setMidiFileAvailable(false);
            setMidiMessagesAvailable(false);
            setPlaybackInProgress(false);
        }
    }

    int getPredelayMs()
    {
        return settings.getProperty(predelayId);
    }

    int getMinSilenceMs()
    {
        return settings.getProperty(minSilenceId);
    }

    int getMinSilenceMultiplier()
    {
        return settings.getProperty(minSilenceMultiplierId);
    }

    juce::String getRootDataDir()
    {

        return settings.getProperty(rootDataDirId);
    }

    bool isMidiFileAvailable()
    {
        return state.getProperty(midiFileAvailableId);
    }

    void setMidiFileAvailable(bool isMidiFileAvailable)
    {
        state.setProperty(midiFileAvailableId, isMidiFileAvailable, nullptr);
    }

    bool isMidiFileAvailableChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return this->state == tree && property == midiFileAvailableId;
    }

    bool isMidiMessagesAvailable()
    {
        return state.getProperty(midiMessagesAvailableId);
    }

    void setMidiMessagesAvailable(bool isMidiMessagesAvailable)
    {
        state.setProperty(midiMessagesAvailableId, isMidiMessagesAvailable, nullptr);
    }

    bool isMidiMessagesAvailableChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return this->state == tree && property == midiMessagesAvailableId;
    }

    bool isPlaybackInProgress()
    {
        return state.getProperty(playbackInProgressId);
    }

    void setPlaybackInProgress(bool isPlaybackInProgress)
    {
        state.setProperty(playbackInProgressId, isPlaybackInProgress, nullptr);
    }

    bool isPlaybackInProgressChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return this->state == tree && property == playbackInProgressId;
    }

    void addListener(juce::ValueTree::Listener *listener)
    {
        state.addListener(listener);
    }

    void removeListener(juce::ValueTree::Listener *listener)
    {
        state.removeListener(listener);
    }

private:
    juce::ValueTree state{stateId};
    juce::ValueTree settings{settingsId};

    // Parent and root node properties
    static inline const juce::Identifier stateId{"MIDI_TIME_MACHINE_STATE"};
    static inline const juce::Identifier settingsId{"settings"};

    // State properties
    static inline const juce::Identifier midiFileAvailableId{"midiFileAvailable"};
    static inline const juce::Identifier midiMessagesAvailableId{"midiMessagesAvailable"};
    static inline const juce::Identifier playbackInProgressId{"playbackInProgress"};

    // Settings properties - user visible
    static inline const juce::Identifier minSilenceId{"minSilenceMs"};
    static inline const juce::Identifier minSilenceMultiplierId{"minSilenceMultiplier"};
    static inline const juce::Identifier predelayId{"predelayMs"};
    static inline const juce::Identifier rootDataDirId{"rootDataDir"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(State)
};
