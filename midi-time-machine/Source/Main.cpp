#include <JuceHeader.h>
#include "MidiOnlyStandaloneWindow.h"

class StandaloneApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return JucePlugin_Name; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override          { return false; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MidiOnlyStandaloneWindow>();
        mainWindow->setVisible(true);
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    std::unique_ptr<MidiOnlyStandaloneWindow> mainWindow;
};

// This macro replaces JUCE's default standalone main()
START_JUCE_APPLICATION(StandaloneApp)