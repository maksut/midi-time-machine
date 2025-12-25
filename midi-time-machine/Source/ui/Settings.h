#pragma once

#include <JuceHeader.h>
#include "../State.h"

class RootDataDirInput : public juce::Component, public juce::Button::Listener
{
public:
    RootDataDirInput(State &state) : state(state)
    {
        auto open = Resources::getIconDrawable(Icon::Folder, juce::Colours::yellow, 0.1f);
        selectDirButton.setTooltip("Select root directory");
        selectDirButton.setImages(open.get());
        selectDirButton.addListener(this);

        addAndMakeVisible(rootDataDir);
        addAndMakeVisible(selectDirButton);
    }

    void resized() override
    {
        juce::FlexBox fb;

        auto stretchSelf = juce::FlexItem::AlignSelf::stretch;

        fb.items = {
            juce::FlexItem(rootDataDir).withFlex(10.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(selectDirButton).withFlex(2.0f, 0.0f).withAlignSelf(stretchSelf),
        };

        fb.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &selectDirButton)
        {
            bool useNativeVersion = true;
            fileChooser.reset(new juce::FileChooser("Choose a directory to store MIDI files", state.getRootDataDir(), "*", useNativeVersion));

            fileChooser->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                [this](const juce::FileChooser &chooser)
                {
                    directorySelectedCallback(chooser);
                });
        }
    }

    juce::String getSelectedRootDataDir()
    {
        return rootDataDir.getText();
    }

    void reloadSettings()
    {
        rootDataDir.setText(state.getRootDataDir());
    }

private:
    void directorySelectedCallback(const juce::FileChooser &chooser)
    {
        auto result = chooser.getURLResult();
        juce::String newRootDataDir;

        if (result.isLocalFile())
        {
            juce::File localFile = result.getLocalFile();

            if (localFile.isDirectory())
                newRootDataDir = localFile.getFullPathName();
        }

        if (!newRootDataDir.isEmpty())
        {
            rootDataDir.setText(newRootDataDir);

            messageBox = juce::AlertWindow::showScopedAsync(
                juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::InfoIcon)
                    .withTitle("Directory chosen")
                    .withMessage("The root data directory will be set to:\n" + newRootDataDir)
                    .withButton("OK"),
                nullptr);
        }
        else
        {
            messageBox = juce::AlertWindow::showScopedAsync(
                juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::InfoIcon)
                    .withTitle("The directory is invalid")
                    .withMessage("The directory is invalid:\n" + result.toString(true))
                    .withButton("OK"),
                nullptr);
        }
    }

private:
    State &state;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ScopedMessageBox messageBox;
    juce::DrawableButton selectDirButton{"Select", juce::DrawableButton::ImageOnButtonBackground};
    juce::TextEditor rootDataDir{"RootDataDir"};
};

class Settings : public juce::Component, public juce::Button::Listener
{
public:
    Settings(State &state) : state(state), rootDataDir(state)
    {
        minSilence.setRange(1, 30, 0.2);
        minSilence.setTextValueSuffix(" sec");

        minSilenceMultipler.setRange(1, 10, 1);
        minSilenceMultipler.setTextValueSuffix("x");

        preDelay.setRange(0.0, 3000.0, 1.0);
        preDelay.setTextValueSuffix(" ms");

        midiTimeFormat.addItem("TPQ", 1);
        midiTimeFormat.addItem("SMPTE", 2);

        reloadSettings();

        okButton.addListener(this);
        resetButton.addListener(this);

        addAndMakeVisible(minSilence);
        addAndMakeVisible(minSilenceLabel);
        addAndMakeVisible(minSilenceMultipler);
        addAndMakeVisible(minSilenceMultiplerLabel);
        addAndMakeVisible(preDelay);
        addAndMakeVisible(preDelayLabel);
        addAndMakeVisible(midiTimeFormat);
        addAndMakeVisible(midiTimeFormatLabel);
        addAndMakeVisible(rootDataDir);
        addAndMakeVisible(rootDataDirLabel);
        addAndMakeVisible(okButton);
        addAndMakeVisible(resetButton);
    }

    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colours::grey);
    }

    void resized() override
    {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;
        using Margin = juce::GridItem::Margin;

        auto margin = Margin(10, 0, 10, 0);

        grid.templateRows = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {Track(Fr(1)), Track(Fr(1))};
        grid.items = {
            juce::GridItem(minSilenceLabel).withMargin(margin),
            juce::GridItem(minSilence).withMargin(margin),
            juce::GridItem(minSilenceMultiplerLabel).withMargin(margin),
            juce::GridItem(minSilenceMultipler).withMargin(margin),
            juce::GridItem(preDelayLabel).withMargin(margin),
            juce::GridItem(preDelay).withMargin(margin),
            juce::GridItem(midiTimeFormatLabel).withMargin(margin),
            juce::GridItem(midiTimeFormat).withMargin(margin),
            juce::GridItem(rootDataDirLabel).withMargin(margin),
            juce::GridItem(rootDataDir).withMargin(margin),
            juce::GridItem(resetButton).withMargin(Margin(5, 10, 5, 20)),
            juce::GridItem(okButton).withMargin(Margin(5, 20, 5, 10)),
        };

        grid.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &okButton)
        {
            state.setMinSilenceMs(int(minSilence.getValue() * 1000.0f));
            state.setMinSilenceMultiplier(int(minSilenceMultipler.getValue()));
            state.setPredelayMs(int(preDelay.getValue()));
            state.setMidiTimeFormat(midiTimeFormat.getSelectedId() == 1 ? "TPQ" : "SMPTE");
            state.setRootDataDir(rootDataDir.getSelectedRootDataDir());

            state.setSettinsOpen(false);
        }
        else if (button == &resetButton)
        {
            state.resetSettings();
            reloadSettings();
        }
    }

    void reloadSettings()
    {
        minSilence.setValue(state.getMinSilenceMs() / 1000.0f);
        minSilenceMultipler.setValue(state.getMinSilenceMultiplier());
        preDelay.setValue(state.getPredelayMs(), juce::NotificationType::dontSendNotification);
        midiTimeFormat.setSelectedId(state.getMidiTimeFormat() == "TPQ" ? 1 : 2);
        rootDataDir.reloadSettings();
    }

private:
    State &state;

    juce::Slider minSilence{"MinSilence"};
    juce::Label minSilenceLabel{"MinSilenceLabel", "Minimum silence:"};

    juce::Slider minSilenceMultipler{"MinSilenceMultiplier"};
    juce::Label minSilenceMultiplerLabel{"MinSilenceMultiplierLabel", "Minimum silence multiplier:"};

    juce::Slider preDelay{"PreDelay"};
    juce::Label preDelayLabel{"PreDelayLabel", "Pre-delay:"};

    juce::ComboBox midiTimeFormat{"MidiTimeFormat"};
    juce::Label midiTimeFormatLabel{"MidiTimeFormatLabel", "MIDI file time format:"};

    RootDataDirInput rootDataDir;
    juce::Label rootDataDirLabel{"RootDataDirLabel", "Root data directory:"};

    juce::TextButton okButton{"OK"};
    juce::TextButton resetButton{"Reset All"};
};