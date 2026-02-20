#pragma once

#include <JuceHeader.h>

class State
{
public:
    State()
    {
        // Set state properties
        state.setProperty(selectedMidiFileId, "", nullptr);
        state.setProperty(playbackTimeSecId, -1.0f, nullptr);
        state.setProperty(recordinInProgressId, false, nullptr);
        state.setProperty(settingsOpenId, false, nullptr);
        state.setProperty(startMarkerPositionId, 0.0f, nullptr);
        state.setProperty(playheadPositionId, 0.0f, nullptr);

        auto homeDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName();
        state.setProperty(lastExportDirId, homeDir, nullptr);

        // Set setting properties
        resetSettings();
        state.addChild(settings, -1, nullptr);
    }

    void resetSettings()
    {
        settings.setProperty(panelHeigthRatioId, 0.5, nullptr);
        settings.setProperty(minSilenceId, 4000, nullptr);
        settings.setProperty(minDurationId, 4000, nullptr);
        settings.setProperty(minNoOfNotesId, 5, nullptr);
        settings.setProperty(minSilenceMultiplierId, 5, nullptr);
        settings.setProperty(predelayId, 1000, nullptr);
        settings.setProperty(midiTimeFormatId, "TPQ", nullptr); // "TPQ" or "SMPTE" // Ticks Per Quarter note or absolute time

        juce::String defaultRootDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                                          .getChildFile("midi_time_machine")
                                          .getFullPathName();

        settings.setProperty(rootDataDirId, defaultRootDir, nullptr);
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

            if (auto s = state.getChildWithName(settingsId); s.isValid())
                settings = s;
            else
                state.addChild(settings, -1, nullptr);

            // But we reset some of the state props
            setIsRecordingInProgress(false);
        }
    }

    int getPredelayMs()
    {
        return settings.getProperty(predelayId);
    }

    void setPredelayMs(int preDelayMs)
    {
        settings.setProperty(predelayId, preDelayMs, nullptr);
    }

    double getPanelHeightRatio()
    {
        return settings.getProperty(panelHeigthRatioId);
    }

    void setPanelHeigthRatio(double panelHeigthRatio)
    {
        settings.setProperty(panelHeigthRatioId, panelHeigthRatio, nullptr);
    }

    bool isPanelHeightRatioChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return settings == tree && property == panelHeigthRatioId;
    }

    int getMinSilenceMs()
    {
        return settings.getProperty(minSilenceId);
    }

    void setMinSilenceMs(int minSilenceMs)
    {
        settings.setProperty(minSilenceId, minSilenceMs, nullptr);
    }

    int getMinDurationMs()
    {
        return settings.getProperty(minDurationId);
    }

    void setMinDurationMs(int minDurationMs)
    {
        settings.setProperty(minDurationId, minDurationMs, nullptr);
    }

    int getMinNoOfNotes()
    {
        return settings.getProperty(minNoOfNotesId);
    }

    void setMinNoOfNotes(int minNoOfNotes)
    {
        settings.setProperty(minNoOfNotesId, minNoOfNotes, nullptr);
    }

    int getMinSilenceMultiplier()
    {
        return settings.getProperty(minSilenceMultiplierId);
    }

    void setMinSilenceMultiplier(int minSilenceMultiplier)
    {
        settings.setProperty(minSilenceMultiplierId, minSilenceMultiplier, nullptr);
    }

    juce::String getRootDataDir()
    {
        return settings.getProperty(rootDataDirId);
    }

    void setRootDataDir(const juce::String &rootDataDir)
    {
        settings.setProperty(rootDataDirId, rootDataDir, nullptr);
    }

    juce::String getMidiTimeFormat()
    {
        return settings.getProperty(midiTimeFormatId);
    }

    void setMidiTimeFormat(const juce::String &midiTimeFormat)
    {
        settings.setProperty(midiTimeFormatId, midiTimeFormat, nullptr);
    }

    juce::String getSelectedMidiFile()
    {
        return state.getProperty(selectedMidiFileId);
    }

    void setSelectedMidiFile(const juce::String &midiFileName)
    {
        state.setProperty(selectedMidiFileId, midiFileName, nullptr);
    }

    bool isSelectedMidiFileChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return state == tree && property == selectedMidiFileId;
    }

    double getPlaybackTimeSec()
    {
        return state.getProperty(playbackTimeSecId);
    }

    void setPlaybackTimeSec(double playbackTimeSec)
    {
        state.setProperty(playbackTimeSecId, playbackTimeSec, nullptr);
    }

    bool isPlaybackTimeSecChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return state == tree && property == playbackTimeSecId;
    }

    bool isRecordingInProgress()
    {
        return state.getProperty(recordinInProgressId);
    }

    void setIsRecordingInProgress(bool recordingInProgress)
    {
        state.setProperty(recordinInProgressId, recordingInProgress, nullptr);
    }

    bool isRecordingInProgressChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return state == tree && property == recordinInProgressId;
    }

    bool isSettinsOpen()
    {
        return state.getProperty(settingsOpenId);
    }

    void setSettinsOpen(bool isSettingsOpenId)
    {
        state.setProperty(settingsOpenId, isSettingsOpenId, nullptr);
    }

    void toggleSettingsOpen()
    {
        setSettinsOpen(!isSettinsOpen());
    }

    bool isSettinsOpenChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return state == tree && property == settingsOpenId;
    }

    juce::String getLastExportDir()
    {
        return state.getProperty(lastExportDirId);
    }

    void setLastExportDir(const juce::String &lastExportDir)
    {
        state.setProperty(lastExportDirId, lastExportDir, nullptr);
    }

    double getStartMarkerPosition()
    {
        return state.getProperty(startMarkerPositionId);
    }

    void setStartMakerPosition(double startMakerPosition)
    {
        state.setProperty(startMarkerPositionId, startMakerPosition, nullptr);
    }

    double getPlayheadPosition()
    {
        return state.getProperty(playheadPositionId);
    }

    void setPlayheadPosition(double playheadPosition)
    {
        state.setProperty(playheadPositionId, playheadPosition, nullptr);
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
    static inline const juce::Identifier selectedMidiFileId{"selectedMidiFile"};
    static inline const juce::Identifier playbackTimeSecId{"playbackTimeSec"};
    static inline const juce::Identifier recordinInProgressId{"recordingInProgress"};
    static inline const juce::Identifier settingsOpenId{"settingsOpen"};
    static inline const juce::Identifier lastExportDirId{"lastExportDir"};
    static inline const juce::Identifier startMarkerPositionId{"startMarkerPosition"};
    static inline const juce::Identifier playheadPositionId{"playheadPosition"};

    // Settings properties - user visible
    static inline const juce::Identifier panelHeigthRatioId{"panelHeightRatio"};
    static inline const juce::Identifier minSilenceId{"minSilenceMs"};
    static inline const juce::Identifier minDurationId{"minDurationMs"};
    static inline const juce::Identifier minNoOfNotesId{"minNoOfNotes"};
    static inline const juce::Identifier minSilenceMultiplierId{"minSilenceMultiplier"};
    static inline const juce::Identifier predelayId{"predelayMs"};
    static inline const juce::Identifier midiTimeFormatId{"midiTimeFormatId"};
    static inline const juce::Identifier rootDataDirId{"rootDataDir"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(State)
};
