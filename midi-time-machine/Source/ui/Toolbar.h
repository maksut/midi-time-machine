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
    Toolbar(Processor &processor, Store &store, MidiPreview &midiPreview, MidiRoll &midiRoll)
        : mProcessor(processor), mState(processor.getState()), mStore(store), mMidiPreview(midiPreview), mMidiRoll(midiRoll)
    {
        mState.addListener(this);

        mPlayButton.setTooltip("Play the last saved midi file");
        mOpenButton.setTooltip("Open the directory of last saved midi file in system explorer");
        mSettingsButton.setTooltip("In which there is plugin settings");

        auto play = Resources::getIconDrawable(Icon::Play, juce::Colours::yellow, 0.1f);
        auto exportIcon = Resources::getIconDrawable(Icon::Export, juce::Colours::yellow, 0.1f);
        auto stop = Resources::getIconDrawable(Icon::Stop, juce::Colours::yellow, 0.1f);
        auto open = Resources::getIconDrawable(Icon::Folder, juce::Colours::yellow, 0.1f);
        auto settings = Resources::getIconDrawable(Icon::Settings, juce::Colours::yellow, 0.1f);

        mPlayButton.setImages(
            play.get(), // normalImage
            nullptr,    // overImage
            nullptr,    // downImage
            nullptr,    // disabledImage
            stop.get()  // normalImageOn
        );

        mOpenButton.setImages(open.get());
        mSettingsButton.setImages(settings.get());

        reset();

        mPlayButton.addListener(this);
        mOpenButton.addListener(this);
        mSettingsButton.addListener(this);

        addAndMakeVisible(mPlayButton);
        addAndMakeVisible(mOpenButton);
        addAndMakeVisible(mSettingsButton);
        addAndMakeVisible(midiPreview);
    }

    ~Toolbar() override
    {
        mState.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (mState.isSelectedMidiFileChange(tree, property))
            reset();
        else if (mState.isPlaybackTimeSecChange(tree, property))
        {
            bool isPlaybackInProgress = mState.getPlaybackTimeSec() >= 0;

            mPlayButton.setToggleState(isPlaybackInProgress, juce::dontSendNotification);
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
            juce::FlexItem(mPlayButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),

            juce::FlexItem(mMidiPreview).withMargin(5).withFlex(10.0f, 0.0f).withAlignSelf(stretchSelf),

            juce::FlexItem(mOpenButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(mSettingsButton).withMargin(5).withFlex(1.0f, 0.0f).withAlignSelf(stretchSelf),
        };

        fb.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &mPlayButton)
        {
            if (mPlayButton.getToggleState())
            {
                mProcessor.stopPlayback();
            }
            else
            {
                std::optional<juce::File> file = getSelectedMidiFile();

                if (file)
                {
                    juce::FileInputStream fileStream(*file);
                    juce::MidiFile midiFile;

                    if (midiFile.readFrom(fileStream))
                        mProcessor.startPlayback(midiFile);
                }
            }
        }
        else if (button == &mOpenButton)
        {
            onOpenClick();
        }
        else if (button == &mSettingsButton)
        {
            mState.toggleSettingsOpen();
        }
    }

private:
    std::optional<juce::File> getSelectedMidiFile()
    {
        juce::File file(mState.getSelectedMidiFile().trim());

        if (file.existsAsFile())
            return file;
        else
            return {};
    }

    std::optional<juce::File> getParentDirOfSelectedMidiFile()
    {
        juce::File file(mState.getSelectedMidiFile().trim());

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
                mMidiRoll.load(midiFile);
                mMidiPreview.load(midiFile);
            }
            else
            {
                showWarning("Loading midi file...", juce::String("Failed to load file:\n") + selectedFile->getFileName());
            }
        }

        mPlayButton.setEnabled(selectedFile.has_value());
    }

    void onOpenClick()
    {
        std::optional<juce::File> parentDir = getParentDirOfSelectedMidiFile();
        juce::File currentDir = parentDir && parentDir->isDirectory() ? *parentDir : mState.getRootDataDir();

        mFileChooser.reset(new juce::FileChooser("Load a midi file...", currentDir, "*.mid", true));

        mFileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser &chooser)
            {
                auto selectedFile = chooser.getURLResult();

                if (selectedFile.isEmpty() || !selectedFile.isLocalFile())
                    return; // Nothing selected so nothing to do. This is not an error.

                juce::File file = selectedFile.getLocalFile();
                mState.setSelectedMidiFile(file.getFullPathName());
            });
    }

    void showWarning(const juce::String &title, const juce::String &message)
    {
        mMessageBox = juce::AlertWindow::showScopedAsync(
            juce::MessageBoxOptions()
                .withIconType(juce::MessageBoxIconType::WarningIcon)
                .withTitle(title)
                .withMessage(message)
                .withButton("OK"),
            nullptr);
    }

private:
    juce::TooltipWindow mTooltipWindow;
    juce::DrawableButton mPlayButton{"Play", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton mOpenButton{"Open", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton mSettingsButton{"Settings", juce::DrawableButton::ImageOnButtonBackground};
    std::unique_ptr<juce::FileChooser> mFileChooser;
    juce::ScopedMessageBox mMessageBox;

    Processor &mProcessor;
    State &mState;
    Store &mStore;
    MidiPreview &mMidiPreview;
    MidiRoll &mMidiRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Toolbar)
};
