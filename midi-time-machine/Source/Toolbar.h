#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "Processor.h"

class Toolbar : public juce::Component,
                public juce::Button::Listener,
                public juce::ValueTree::Listener
{
public:
    Toolbar(Processor &processor, Store &store) : processor(processor), state(processor.getState()), store(store)
    {
        state.addListener(this);

        playButton.setTooltip("Play the last saved midi file");
        openButton.setTooltip("Open the directory of last saved midi file in system explorer");
        settingsButton.setTooltip("In which there is plugin settings");

        auto play = Resources::getIconDrawable(Icon::Play, juce::Colours::yellow, 0.1f);
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

        playButton.setEnabled(state.isMidiFileAvailable());
        playButton.addListener(this);
        openButton.addListener(this);
        settingsButton.addListener(this);

        addAndMakeVisible(playButton);
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
        if (state.isMidiFileAvailableChange(tree, property))
        {
            playButton.setEnabled(state.isMidiFileAvailable());
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
    }

private:
    juce::TooltipWindow tooltipWindow;
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton openButton{"Open", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton settingsButton{"Settings", juce::DrawableButton::ImageOnButtonBackground};
    juce::Label midiFileDescription{"MidiFileDescription", "Nothing captured yet\nPlay something"};

    Processor &processor;
    State &state;
    Store &store;
};
