#pragma once

#include <JuceHeader.h>

enum class Icon
{
    Play,
    Pause,
    Stop,
    Open,
    Folder,
    Settings
};

namespace Resources
{
    juce::Path getIconPath(Icon icon);
    std::shared_ptr<juce::DrawablePath> getIconDrawable(Icon icon, juce::Colour strokeFill, float strokeThickness);
}