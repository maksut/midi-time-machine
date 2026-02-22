#pragma once

#include <JuceHeader.h>

class State
{
public:
    State()
    {
        // Set state properties
        mState.setProperty(sSelectedMidiFileId, "", nullptr);
        mState.setProperty(sPlaybackTimeSecId, -1.0f, nullptr);
        mState.setProperty(sRecordinInProgressId, false, nullptr);
        mState.setProperty(sSettingsOpenId, false, nullptr);
        mState.setProperty(sStartMarkerPositionId, 0.0f, nullptr);
        mState.setProperty(sPlayheadPositionId, 0.0f, nullptr);

        auto homeDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName();
        mState.setProperty(sLastExportDirId, homeDir, nullptr);

        // Set setting properties
        resetSettings();
        mState.addChild(mSettings, -1, nullptr);
    }

    void resetSettings()
    {
        mSettings.setProperty(sPanelHeigthRatioId, 0.5, nullptr);
        mSettings.setProperty(sMinSilenceId, 4000, nullptr);
        mSettings.setProperty(sMinDurationId, 4000, nullptr);
        mSettings.setProperty(sMinNoOfNotesId, 5, nullptr);
        mSettings.setProperty(sMinSilenceMultiplierId, 5, nullptr);
        mSettings.setProperty(sPredelayId, 1000, nullptr);
        mSettings.setProperty(sMidiTimeFormatId, "TPQ", nullptr); // "TPQ" or "SMPTE" // Ticks Per Quarter note or absolute time

        juce::String defaultRootDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                                          .getChildFile("midi_time_machine")
                                          .getFullPathName();

        mSettings.setProperty(sRootDataDirId, defaultRootDir, nullptr);
    }

    std::unique_ptr<juce::XmlElement> toXml() const
    {
        return mState.createXml();
    }

    void fromXml(std::unique_ptr<juce::XmlElement> &xmlState)
    {
        auto newState = juce::ValueTree::fromXml(*xmlState);

        if (newState.isValid() && newState.hasType(sStateId))
        {
            mState.copyPropertiesAndChildrenFrom(newState, nullptr);

            if (auto s = mState.getChildWithName(sSettingsId); s.isValid())
                mSettings = s;
            else
                mState.addChild(mSettings, -1, nullptr);

            // But we reset some of the state props
            setIsRecordingInProgress(false);
        }
    }

    int getPredelayMs()
    {
        return mSettings.getProperty(sPredelayId);
    }

    void setPredelayMs(int preDelayMs)
    {
        mSettings.setProperty(sPredelayId, preDelayMs, nullptr);
    }

    double getPanelHeightRatio()
    {
        return mSettings.getProperty(sPanelHeigthRatioId);
    }

    void setPanelHeigthRatio(double panelHeigthRatio)
    {
        mSettings.setProperty(sPanelHeigthRatioId, panelHeigthRatio, nullptr);
    }

    bool isPanelHeightRatioChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return mSettings == tree && property == sPanelHeigthRatioId;
    }

    int getMinSilenceMs()
    {
        return mSettings.getProperty(sMinSilenceId);
    }

    void setMinSilenceMs(int minSilenceMs)
    {
        mSettings.setProperty(sMinSilenceId, minSilenceMs, nullptr);
    }

    int getMinDurationMs()
    {
        return mSettings.getProperty(sMinDurationId);
    }

    void setMinDurationMs(int minDurationMs)
    {
        mSettings.setProperty(sMinDurationId, minDurationMs, nullptr);
    }

    int getMinNoOfNotes()
    {
        return mSettings.getProperty(sMinNoOfNotesId);
    }

    void setMinNoOfNotes(int minNoOfNotes)
    {
        mSettings.setProperty(sMinNoOfNotesId, minNoOfNotes, nullptr);
    }

    int getMinSilenceMultiplier()
    {
        return mSettings.getProperty(sMinSilenceMultiplierId);
    }

    void setMinSilenceMultiplier(int minSilenceMultiplier)
    {
        mSettings.setProperty(sMinSilenceMultiplierId, minSilenceMultiplier, nullptr);
    }

    juce::String getRootDataDir()
    {
        return mSettings.getProperty(sRootDataDirId);
    }

    void setRootDataDir(const juce::String &rootDataDir)
    {
        mSettings.setProperty(sRootDataDirId, rootDataDir, nullptr);
    }

    juce::String getMidiTimeFormat()
    {
        return mSettings.getProperty(sMidiTimeFormatId);
    }

    void setMidiTimeFormat(const juce::String &midiTimeFormat)
    {
        mSettings.setProperty(sMidiTimeFormatId, midiTimeFormat, nullptr);
    }

    juce::String getSelectedMidiFile()
    {
        return mState.getProperty(sSelectedMidiFileId);
    }

    void setSelectedMidiFile(const juce::String &midiFileName)
    {
        mState.setProperty(sSelectedMidiFileId, midiFileName, nullptr);
    }

    bool isSelectedMidiFileChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return mState == tree && property == sSelectedMidiFileId;
    }

    double getPlaybackTimeSec()
    {
        return mState.getProperty(sPlaybackTimeSecId);
    }

    void setPlaybackTimeSec(double playbackTimeSec)
    {
        mState.setProperty(sPlaybackTimeSecId, playbackTimeSec, nullptr);
    }

    bool isPlaybackTimeSecChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return mState == tree && property == sPlaybackTimeSecId;
    }

    bool isRecordingInProgress()
    {
        return mState.getProperty(sRecordinInProgressId);
    }

    void setIsRecordingInProgress(bool recordingInProgress)
    {
        mState.setProperty(sRecordinInProgressId, recordingInProgress, nullptr);
    }

    bool isRecordingInProgressChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return mState == tree && property == sRecordinInProgressId;
    }

    bool isSettinsOpen()
    {
        return mState.getProperty(sSettingsOpenId);
    }

    void setSettinsOpen(bool isSettingsOpenId)
    {
        mState.setProperty(sSettingsOpenId, isSettingsOpenId, nullptr);
    }

    void toggleSettingsOpen()
    {
        setSettinsOpen(!isSettinsOpen());
    }

    bool isSettinsOpenChange(juce::ValueTree &tree, const juce::Identifier &property)
    {
        return mState == tree && property == sSettingsOpenId;
    }

    juce::String getLastExportDir()
    {
        return mState.getProperty(sLastExportDirId);
    }

    void setLastExportDir(const juce::String &lastExportDir)
    {
        mState.setProperty(sLastExportDirId, lastExportDir, nullptr);
    }

    double getStartMarkerPosition()
    {
        return mState.getProperty(sStartMarkerPositionId);
    }

    void setStartMarkerPosition(double startMakerPosition)
    {
        mState.setProperty(sStartMarkerPositionId, startMakerPosition, nullptr);
    }

    double getPlayheadPosition()
    {
        return mState.getProperty(sPlayheadPositionId);
    }

    void setPlayheadPosition(double playheadPosition)
    {
        mState.setProperty(sPlayheadPositionId, playheadPosition, nullptr);
    }

    void addListener(juce::ValueTree::Listener *listener)
    {
        mState.addListener(listener);
    }

    void removeListener(juce::ValueTree::Listener *listener)
    {
        mState.removeListener(listener);
    }

private:
    juce::ValueTree mState{sStateId};
    juce::ValueTree mSettings{sSettingsId};

    // Parent and root node properties
    static inline const juce::Identifier sStateId{"MIDI_TIME_MACHINE_STATE"};
    static inline const juce::Identifier sSettingsId{"settings"};

    // State properties
    static inline const juce::Identifier sSelectedMidiFileId{"selectedMidiFile"};
    static inline const juce::Identifier sPlaybackTimeSecId{"playbackTimeSec"};
    static inline const juce::Identifier sRecordinInProgressId{"recordingInProgress"};
    static inline const juce::Identifier sSettingsOpenId{"settingsOpen"};
    static inline const juce::Identifier sLastExportDirId{"lastExportDir"};
    static inline const juce::Identifier sStartMarkerPositionId{"startMarkerPosition"};
    static inline const juce::Identifier sPlayheadPositionId{"playheadPosition"};

    // Settings properties - user visible
    static inline const juce::Identifier sPanelHeigthRatioId{"panelHeightRatio"};
    static inline const juce::Identifier sMinSilenceId{"minSilenceMs"};
    static inline const juce::Identifier sMinDurationId{"minDurationMs"};
    static inline const juce::Identifier sMinNoOfNotesId{"minNoOfNotes"};
    static inline const juce::Identifier sMinSilenceMultiplierId{"minSilenceMultiplier"};
    static inline const juce::Identifier sPredelayId{"predelayMs"};
    static inline const juce::Identifier sMidiTimeFormatId{"midiTimeFormatId"};
    static inline const juce::Identifier sRootDataDirId{"rootDataDir"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(State)
};
