#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "../Processor.h"
#include "../State.h"
#include "../Store.h"

class Toolbar : public juce::Component,
                public juce::Button::Listener,
                public juce::ValueTree::Listener
{
public:
    Toolbar(Processor &processor, Store &store) : processor(processor), state(processor.getState()), store(store)
    {
        state.addListener(this);

        playButton.setTooltip("Play the last saved midi file");
        exportButton.setTooltip("Export the last saved midi file");
        openButton.setTooltip("Open the directory of last saved midi file in system explorer");
        settingsButton.setTooltip("In which there is plugin settings");

        auto play = Resources::getIconDrawable(Icon::Play, juce::Colours::yellow, 0.1f);
        auto exportIcon = Resources::getIconDrawable(Icon::Export, juce::Colours::yellow, 0.1f);
        auto stop = Resources::getIconDrawable(Icon::Stop, juce::Colours::yellow, 0.1f);
        auto open = Resources::getIconDrawable(Icon::Folder, juce::Colours::yellow, 0.1f);
        auto settings = Resources::getIconDrawable(Icon::Settings, juce::Colours::yellow, 0.1f);

        playButton.setImages(
            play.get(), // normalImage
            nullptr,    // overImage
            nullptr,    // downImage
            nullptr,    // disabledImage
            stop.get()  // normalImageOn
        );

        exportButton.setImages(exportIcon.get());
        openButton.setImages(open.get());
        settingsButton.setImages(settings.get());

        bool isMidiFileAvailable = store.getLastSavedMidiFile().has_value();
        playButton.setEnabled(isMidiFileAvailable);
        exportButton.setEnabled(isMidiFileAvailable);

        playButton.addListener(this);
        exportButton.addListener(this);
        openButton.addListener(this);
        settingsButton.addListener(this);

        addAndMakeVisible(playButton);
        addAndMakeVisible(exportButton);
        addAndMakeVisible(midiFileDescription);
        addAndMakeVisible(openButton);
        addAndMakeVisible(settingsButton);
    }

    ~Toolbar() override
    {
        state.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (state.isMidiFileChange(tree, property))
        {
            bool isMidiFileAvailable = store.getLastSavedMidiFile().has_value();
            playButton.setEnabled(isMidiFileAvailable);
            exportButton.setEnabled(isMidiFileAvailable);
            midiFileDescription.setText(store.getLastSavedFileDescription(), juce::dontSendNotification);
        }
        else if (state.isPlaybackInProgressChange(tree, property))
        {
            playButton.setToggleState(state.isPlaybackInProgress(), juce::dontSendNotification);
        }
    }

    void resized() override
    {
        juce::FlexBox fb;

        fb.flexWrap = juce::FlexBox::Wrap::wrap;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.alignContent = juce::FlexBox::AlignContent::stretch;
        fb.alignItems = juce::FlexBox::AlignItems::stretch;

        auto stretchSelf = juce::FlexItem::AlignSelf::stretch;

        fb.items = {
            juce::FlexItem(playButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(exportButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(midiFileDescription).withMargin(5).withFlex(10.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(openButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(settingsButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),
        };

        fb.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &playButton)
        {
            if (playButton.getToggleState())
            {
                processor.stopPlayback();
            }
            else
            {
                auto midiFile = store.getLastSavedMidiFile();

                if (midiFile)
                    processor.startPlayback(*midiFile);
            }
        }
        else if (button == &openButton)
        {
            juce::URL(state.getRootDataDir()).launchInDefaultBrowser();
        }
        else if (button == &settingsButton)
        {
            state.toggleSettingsOpen();
        }
        else if (button == &exportButton)
        {
            onExportClick();
        }
    }

private:
    void onExportClick()
    {
        auto midiFile = store.getLastSavedMidiFile();

        if (!midiFile)
            return; // No midi file available. Nothing to do

        fileChooser.reset(
            new juce::FileChooser("Export last saved midi file...",
                                  juce::File(state.getLastExportDir()),
                                  "*.mid", true));

        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, midiFile](const juce::FileChooser &chooser)
            {
                auto result = chooser.getURLResult();

                if (!exportMidiFile(*midiFile, result))
                {
                    auto name = result.isLocalFile() ? result.getLocalFile().getFullPathName() : result.toString(true);
                    showWarning(juce::String("Failed to save file:\n") + name);
                }
            });
    }

    bool exportMidiFile(const juce::MidiFile &midiFile, const juce::URL &selectedFile)
    {
        if (selectedFile.isEmpty() || !selectedFile.isLocalFile())
            return true; // Nothing selected so nothing to do. This is not an error.

        juce::FileOutputStream stream(selectedFile.getLocalFile());

        if (!stream.openedOk())
            return false; // Couldn't write the file

        if (!midiFile.writeTo(stream))
            return false; // Couldn't write the file

        // All is fine
        state.setLastExportDir(stream.getFile().getParentDirectory().getFullPathName());

        return true;
    }

    void showWarning(const juce::String &message)
    {
        messageBox = juce::AlertWindow::showScopedAsync(
            juce::MessageBoxOptions()
                .withIconType(juce::MessageBoxIconType::WarningIcon)
                .withTitle("Export midi file...")
                .withMessage(message)
                .withButton("OK"),
            nullptr);
    }

private:
    juce::TooltipWindow tooltipWindow;
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton exportButton{"Export", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton openButton{"Open", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton settingsButton{"Settings", juce::DrawableButton::ImageOnButtonBackground};
    juce::Label midiFileDescription{"MidiFileDescription", "Nothing captured yet\nPlay something"};
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ScopedMessageBox messageBox;

    Processor &processor;
    State &state;
    Store &store;
};
