#pragma once
#include <JuceHeader.h>

#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#include "NullAudioDevice.h"

//==============================================================================
// Main window — no audio UI, MIDI-only settings dialog, processBlock ticks
// via the silent NullAudioDevice thread.
//==============================================================================
class MidiOnlyStandaloneWindow : public juce::StandaloneFilterWindow
{
public:
    MidiOnlyStandaloneWindow()
        : juce::StandaloneFilterWindow (
              JucePlugin_Name,
              juce::LookAndFeel::getDefaultLookAndFeel()
                  .findColour (juce::ResizableWindow::backgroundColourId),
              nullptr,
              true)
    {
        auto& dm = getDeviceManager();

        //--- 1. Register and activate the null audio device -------------------
        dm.addAudioDeviceType (std::make_unique<NullAudioDeviceType>());

        juce::AudioDeviceManager::AudioDeviceSetup setup;
        dm.getAudioDeviceSetup (setup);
        setup.outputDeviceName = "No Audio";
        setup.inputDeviceName  = "";
        setup.sampleRate       = 44100.0;
        setup.bufferSize       = 512;
        dm.setAudioDeviceSetup (setup, true);

        // setVisible (true);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOnlyStandaloneWindow)
};