#pragma once

#include <JuceHeader.h>
#include "../State.h"

class RootDataDirInput : public juce::Component, public juce::Button::Listener
{
public:
    RootDataDirInput(State &state) : mState(state)
    {
        auto open = Resources::getIconDrawable(Icon::Folder, juce::Colours::yellow, 0.1f);
        mSelectDirButton.setTooltip("Select root directory");
        mSelectDirButton.setImages(open.get());
        mSelectDirButton.addListener(this);

        addAndMakeVisible(mRootDataDir);
        addAndMakeVisible(mSelectDirButton);
    }

    void resized() override
    {
        juce::FlexBox fb;

        auto stretchSelf = juce::FlexItem::AlignSelf::stretch;

        fb.items = {
            juce::FlexItem(mRootDataDir).withFlex(10.0f, 0.0f).withAlignSelf(stretchSelf),
            juce::FlexItem(mSelectDirButton).withFlex(2.0f, 0.0f).withAlignSelf(stretchSelf),
        };

        fb.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &mSelectDirButton)
        {
            bool useNativeVersion = true;
            mFileChooser.reset(new juce::FileChooser("Choose a directory to store MIDI files", mState.getRootDataDir(), "*", useNativeVersion));

            mFileChooser->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                [this](const juce::FileChooser &chooser)
                {
                    directorySelectedCallback(chooser);
                });
        }
    }

    juce::String getSelectedRootDataDir()
    {
        return mRootDataDir.getText();
    }

    void reloadSettings()
    {
        mRootDataDir.setText(mState.getRootDataDir());
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
            mRootDataDir.setText(newRootDataDir);

            mMessageBox = juce::AlertWindow::showScopedAsync(
                juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::InfoIcon)
                    .withTitle("Directory chosen")
                    .withMessage("The root data directory will be set to:\n" + newRootDataDir)
                    .withButton("OK"),
                nullptr);
        }
        else
        {
            mMessageBox = juce::AlertWindow::showScopedAsync(
                juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::InfoIcon)
                    .withTitle("The directory is invalid")
                    .withMessage("The directory is invalid:\n" + result.toString(true))
                    .withButton("OK"),
                nullptr);
        }
    }

private:
    State &mState;
    std::unique_ptr<juce::FileChooser> mFileChooser;
    juce::ScopedMessageBox mMessageBox;
    juce::DrawableButton mSelectDirButton{"Select", juce::DrawableButton::ImageOnButtonBackground};
    juce::TextEditor mRootDataDir{"RootDataDir"};
};

class Settings : public juce::Component, public juce::Button::Listener
{
public:
    Settings(State &state) : mState(state), mRootDataDir(state)
    {
        mPanelHeigthRatio.setRange(0, 1, 0.01);

        mMinSilence.setRange(1, 30, 0.2);
        mMinSilence.setTextValueSuffix(" sec");

        mMinDuration.setRange(1, 30, 0.2);
        mMinDuration.setTextValueSuffix(" sec");

        mMinNoOfNotes.setRange(1, 100, 1);
        mMinNoOfNotes.setTextValueSuffix(" notes");

        mMinSilenceMultipler.setRange(1, 10, 1);
        mMinSilenceMultipler.setTextValueSuffix("x");

        mPreDelay.setRange(0.0, 3000.0, 1.0);
        mPreDelay.setTextValueSuffix(" ms");

        mMidiTimeFormat.addItem("TPQ", 1);
        mMidiTimeFormat.addItem("SMPTE", 2);

        reloadSettings();

        mOkButton.addListener(this);
        mResetButton.addListener(this);

        addAndMakeVisible(mPanelHeigthRatio);
        addAndMakeVisible(mPanelHeigthRatioLabel);
        addAndMakeVisible(mMinSilence);
        addAndMakeVisible(mMinSilenceLabel);
        addAndMakeVisible(mMinDuration);
        addAndMakeVisible(mMinDurationLabel);
        addAndMakeVisible(mMinNoOfNotes);
        addAndMakeVisible(mMinNoOfNotesLabel);
        addAndMakeVisible(mMinSilenceMultipler);
        addAndMakeVisible(mMinSilenceMultiplerLabel);
        addAndMakeVisible(mPreDelay);
        addAndMakeVisible(mPreDelayLabel);
        addAndMakeVisible(mMidiTimeFormat);
        addAndMakeVisible(mMidiTimeFormatLabel);
        addAndMakeVisible(mRootDataDir);
        addAndMakeVisible(mRootDataDirLabel);
        addAndMakeVisible(mOkButton);
        addAndMakeVisible(mResetButton);
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
        using Margin = juce::GridItem::Margin;

        auto margin = Margin(4, 0, 4, 0);

        grid.templateRows = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {Track(Fr(1)), Track(Fr(1))};
        grid.items = {
            juce::GridItem(mPanelHeigthRatioLabel).withMargin(margin),
            juce::GridItem(mPanelHeigthRatio).withMargin(margin),

            juce::GridItem(mMinSilenceLabel).withMargin(margin),
            juce::GridItem(mMinSilence).withMargin(margin),

            juce::GridItem(mMinDurationLabel).withMargin(margin),
            juce::GridItem(mMinDuration).withMargin(margin),

            juce::GridItem(mMinNoOfNotesLabel).withMargin(margin),
            juce::GridItem(mMinNoOfNotes).withMargin(margin),

            juce::GridItem(mMinSilenceMultiplerLabel).withMargin(margin),
            juce::GridItem(mMinSilenceMultipler).withMargin(margin),

            juce::GridItem(mPreDelayLabel).withMargin(margin),
            juce::GridItem(mPreDelay).withMargin(margin),

            juce::GridItem(mMidiTimeFormatLabel).withMargin(margin),
            juce::GridItem(mMidiTimeFormat).withMargin(margin),

            juce::GridItem(mRootDataDirLabel).withMargin(margin),
            juce::GridItem(mRootDataDir).withMargin(margin),

            juce::GridItem(mResetButton).withMargin(Margin(5, 10, 5, 20)),
            juce::GridItem(mOkButton).withMargin(Margin(5, 20, 5, 10)),
        };

        grid.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &mOkButton)
        {
            mState.setPanelHeigthRatio(mPanelHeigthRatio.getValue());
            mState.setMinSilenceMs(int(mMinSilence.getValue() * 1000.0f));
            mState.setMinDurationMs(int(mMinDuration.getValue() * 1000.0f));
            mState.setMinNoOfNotes(int(mMinNoOfNotes.getValue()));
            mState.setMinSilenceMultiplier(int(mMinSilenceMultipler.getValue()));
            mState.setPredelayMs(int(mPreDelay.getValue()));
            mState.setMidiTimeFormat(mMidiTimeFormat.getSelectedId() == 1 ? "TPQ" : "SMPTE");
            mState.setRootDataDir(mRootDataDir.getSelectedRootDataDir());

            mState.setSettinsOpen(false);
        }
        else if (button == &mResetButton)
        {
            mState.resetSettings();
            reloadSettings();
        }
    }

    void reloadSettings()
    {
        mPanelHeigthRatio.setValue(mState.getPanelHeightRatio());
        mMinSilence.setValue(mState.getMinSilenceMs() / 1000.0f);
        mMinDuration.setValue(mState.getMinDurationMs() / 1000.0f);
        mMinNoOfNotes.setValue(mState.getMinNoOfNotes());
        mMinSilenceMultipler.setValue(mState.getMinSilenceMultiplier());
        mPreDelay.setValue(mState.getPredelayMs(), juce::NotificationType::dontSendNotification);
        mMidiTimeFormat.setSelectedId(mState.getMidiTimeFormat() == "TPQ" ? 1 : 2);
        mRootDataDir.reloadSettings();
    }

private:
    State &mState;

    juce::Slider mPanelHeigthRatio{"PanelHeigthRatio"};
    juce::Label mPanelHeigthRatioLabel{"PanelHeigthRatioLabel", "Panel heigth ratio:"};

    juce::Slider mMinSilence{"MinSilence"};
    juce::Label mMinSilenceLabel{"MinSilenceLabel", "Minimum silence:"};

    juce::Slider mMinDuration{"MinDuration"};
    juce::Label mMinDurationLabel{"MinDurationLabel", "Minimum duration:"};

    juce::Slider mMinNoOfNotes{"MinNoOfNotes"};
    juce::Label mMinNoOfNotesLabel{"MinNoOfNotes", "Minimum no of notes:"};

    juce::Slider mMinSilenceMultipler{"MinSilenceMultiplier"};
    juce::Label mMinSilenceMultiplerLabel{"MinSilenceMultiplierLabel", "Minimum silence multiplier:"};

    juce::Slider mPreDelay{"PreDelay"};
    juce::Label mPreDelayLabel{"PreDelayLabel", "Pre-delay:"};

    juce::ComboBox mMidiTimeFormat{"MidiTimeFormat"};
    juce::Label mMidiTimeFormatLabel{"MidiTimeFormatLabel", "MIDI file time format:"};

    RootDataDirInput mRootDataDir;
    juce::Label mRootDataDirLabel{"RootDataDirLabel", "Root data directory:"};

    juce::TextButton mOkButton{"OK"};
    juce::TextButton mResetButton{"Reset All"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Settings)
};