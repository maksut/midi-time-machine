#pragma once

#include <JuceHeader.h>
#include "Resources.h"
#include "Store.h"

class Toolbar : public juce::Component, public juce::Button::Listener
{
public:
    Toolbar(Processor &processor, Store &store) : processor(processor), store(store)
    {
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

        playButton.setClickingTogglesState(true);
        playButton.setEnabled(store.getLastSavedMidiFile().has_value());
        playButton.addListener(this);
        openButton.addListener(this);
        settingsButton.addListener(this);

        addAndMakeVisible(playButton);
        addAndMakeVisible(openButton);
        addAndMakeVisible(settingsButton);
    }

    void resized() override
    {
        juce::FlexBox fb;
        fb.flexWrap = juce::FlexBox::Wrap::wrap;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.alignContent = juce::FlexBox::AlignContent::center;

        for (auto *button : getChildren())
            fb.items.add(juce::FlexItem(*button).withMargin(5).withMinWidth(50.0f).withMinHeight(50.0f));

        fb.performLayout(getLocalBounds());
    }

    void buttonClicked(juce::Button *button) override
    {
        if (button == &playButton)
        {
            if (playButton.getToggleState())
                std::cout << "Play is on" << std::endl;
            else
                std::cout << "Play is off" << std::endl;
        }
        else if (button == &openButton)
        {
            juce::URL(store.getRootDataDir()).launchInDefaultBrowser();
        }
    }

private:
    juce::TooltipWindow tooltipWindow;
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton openButton{"Open", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton settingsButton{"Settings", juce::DrawableButton::ImageOnButtonBackground};

    Processor &processor;
    Store &store;
};
