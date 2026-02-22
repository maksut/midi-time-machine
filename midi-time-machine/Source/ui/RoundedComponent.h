#pragma once

#include <JuceHeader.h>

class RoundedComponent : public juce::Component
{
public:
    RoundedComponent()
    {
        mBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    }

    void paintOverChildren(juce::Graphics &g) override
    {
        juce::Rectangle<float> bounds = getLocalBounds().toFloat();

        // Create a path that is the INVERSE of a rounded rectangle - four corner regions
        juce::Path corners;
        corners.addRectangle(bounds);              // full rect
        corners.addRoundedRectangle(bounds, 6.0f); // subtract the rounded rect
        corners.setUsingNonZeroWinding(false);     // makes it act as XOR / inverse

        g.setColour(mBackgroundColour);
        g.fillPath(corners);
    }

protected:
    juce::Colour mBackgroundColour;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoundedComponent)
};