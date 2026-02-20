#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "../Processor.h"
#include "../State.h"
#include "../Store.h"
#include "MidiRoll.h"

class Toolbar : public juce::Component,
                public juce::Button::Listener,
                public juce::ValueTree::Listener
{
public:
    Toolbar(Processor &processor, Store &store, MidiRoll &midiToolbarPreview, MidiRoll &midiZoomedPreview)
        : processor(processor), state(processor.getState()), store(store), midiToolbarPreview(midiToolbarPreview), midiZoomedPreview(midiZoomedPreview)
    {
        state.addListener(this);

        playButton.setTooltip("Play the last saved midi file");
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

        openButton.setImages(open.get());
        settingsButton.setImages(settings.get());

        reset();

        playButton.addListener(this);
        openButton.addListener(this);
        settingsButton.addListener(this);

        addAndMakeVisible(playButton);
        addAndMakeVisible(openButton);
        addAndMakeVisible(settingsButton);
        addAndMakeVisible(midiToolbarPreview);
    }

    ~Toolbar() override
    {
        state.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (state.isSelectedMidiFileChange(tree, property))
            reset();
        else if (state.isPlaybackTimeSecChange(tree, property))
        {
            bool isPlaybackInProgress = state.getPlaybackTimeSec() >= 0;

            playButton.setToggleState(isPlaybackInProgress, juce::dontSendNotification);
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

            juce::FlexItem(midiToolbarPreview).withMargin(5).withFlex(10.0f, 0.0f).withAlignSelf(stretchSelf),

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
                std::optional<juce::File> file = getSelectedMidiFile();

                if (file)
                {
                    juce::FileInputStream fileStream(*file);
                    juce::MidiFile midiFile;

                    if (midiFile.readFrom(fileStream))
                        processor.startPlayback(midiFile);
                }
            }
        }
        else if (button == &openButton)
        {
            onOpenClick();
        }
        else if (button == &settingsButton)
        {
            state.toggleSettingsOpen();
        }
    }

private:
    std::optional<juce::File> getSelectedMidiFile()
    {
        juce::File file(state.getSelectedMidiFile().trim());

        if (file.existsAsFile())
            return file;
        else
            return {};
    }

    std::optional<juce::File> getParentDirOfSelectedMidiFile()
    {
        juce::File file(state.getSelectedMidiFile().trim());

        if (file.existsAsFile())
            return file.getParentDirectory();
        else
            return {};
    }

    void reset()
    {
        std::optional<juce::File> selectedFile = getSelectedMidiFile();

        if (selectedFile)
        {
            juce::FileInputStream fileStream(*selectedFile);
            juce::MidiFile midiFile;

            if (midiFile.readFrom(fileStream))
            {
                midiZoomedPreview.load(midiFile);
                midiToolbarPreview.load(midiFile);
            }
            else
            {
                showWarning(juce::String("Failed to load file:\n") + selectedFile->getFileName());
            }
        }

        playButton.setEnabled(selectedFile.has_value());
    }

    void onOpenClick()
    {
        std::optional<juce::File> parentDir = getParentDirOfSelectedMidiFile();
        juce::File currentDir = parentDir && parentDir->isDirectory() ? *parentDir : state.getRootDataDir();

        fileChooser.reset(new juce::FileChooser("Load a midi file...", currentDir, "*.mid", true));

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser &chooser)
            {
                auto selectedFile = chooser.getURLResult();

                if (selectedFile.isEmpty() || !selectedFile.isLocalFile())
                    return; // Nothing selected so nothing to do. This is not an error.

                juce::File file = selectedFile.getLocalFile();
                state.setSelectedMidiFile(file.getFullPathName());
            });
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
    juce::DrawableButton openButton{"Open", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton settingsButton{"Settings", juce::DrawableButton::ImageOnButtonBackground};
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ScopedMessageBox messageBox;

    Processor &processor;
    State &state;
    Store &store;
    MidiRoll &midiToolbarPreview;
    MidiRoll &midiZoomedPreview;
};
