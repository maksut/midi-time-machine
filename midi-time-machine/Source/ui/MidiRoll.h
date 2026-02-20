#pragma once

#include <JuceHeader.h>
#include "../Store.h"
#include "../State.h"

class MidiRoll : public juce::Component, public juce::ValueTree::Listener
{
public:
    MidiRoll(State &state, Store &store, int windowSizeSec = -1) : state(state), store(store), windowSizeSec(windowSizeSec)
    {
        state.addListener(this);
    }

    ~MidiRoll() override
    {
        state.removeListener(this);
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

        if (midiPath)
        {
            g.setColour(juce::Colours::black);
            juce::Rectangle<int> rect = getLocalBounds();

            if (windowSizeSec <= 0)
            {
                g.addTransform(juce::AffineTransform::scale(rect.getWidth(), rect.getHeight()));
            }
            else
            {
                float width = (rect.getWidth() / windowSizeSec) * midiDurationSec;
                g.addTransform(juce::AffineTransform::translation(4.0f, 0.0f)); // 4 sec scroll for testing
                g.addTransform(juce::AffineTransform::scale(width, rect.getHeight()));
            }

            g.fillPath(*midiPath);
        }
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
    }

    void load(juce::MidiFile &midiFileArg)
    {
        // Reset state
        unload();
        midiFile.emplace(juce::MidiFile(midiFileArg)); // Get a clone of it
        midiPath.emplace(juce::Path());

        //
        // Copy the midiFile, update matched pairs and convert ticks to seconds
        // Also calculate note range for the file.
        //

        noteRangeBegin = std::numeric_limits<int>().max();
        noteRangeEnd = std::numeric_limits<int>().min();

        for (int trackIndex = 0; trackIndex < midiFile->getNumTracks(); ++trackIndex)
        {
            const juce::MidiMessageSequence *sequence = midiFile->getTrack(trackIndex);

            for (int eventIndex = 0; eventIndex < sequence->getNumEvents(); ++eventIndex)
            {
                auto &message = sequence->getEventPointer(eventIndex)->message;

                if (!message.isNoteOn())
                    continue;

                noteRangeBegin = juce::jmin(noteRangeBegin, message.getNoteNumber());
                noteRangeEnd = juce::jmax(noteRangeEnd, message.getNoteNumber());
            }
        }

        midiFile->convertTimestampTicksToSeconds();

        // Calculate duration
        for (int trackIndex = 0; trackIndex < midiFile->getNumTracks(); ++trackIndex)
            midiDurationSec = juce::jmax(midiDurationSec, midiFile->getTrack(trackIndex)->getEndTime());

        // Draw it into the midiPath
        for (int trackIndex = 0; trackIndex < midiFile->getNumTracks(); ++trackIndex)
            drawTrack(*midiPath, midiFile->getTrack(trackIndex));

        repaint();
    }

    void unload()
    {
        midiFile.reset();
        midiPath.reset();
        noteRangeBegin = 0;
        noteRangeEnd = 127;
        midiDurationSec = 0;
        repaint();
    }

private:
    void drawTrack(juce::Path &midiPath, const juce::MidiMessageSequence *sequence)
    {
        double durationSec = sequence->getEndTime();
        int noteRange = juce::jmax(noteRangeEnd - noteRangeBegin + 1, 40);

        float margin = 1.0f / (noteRange + 4); // 2 line thickness margin
        float lineThickness = (1.0f - (2 * margin)) / noteRange;

        double lengthBySec = (1.0f - (2 * margin)) / durationSec;

        for (int i = 0; i < sequence->getNumEvents(); ++i)
        {
            auto event = sequence->getEventPointer(i);

            if (!event->message.isNoteOn())
                continue;

            auto *noteOffEvent = event->noteOffObject;

            if (noteOffEvent == nullptr)
                continue;

            double startTime = event->message.getTimeStamp();
            double endTime = noteOffEvent->message.getTimeStamp();
            double noteDuration = endTime - startTime;

            float x1 = (lengthBySec * startTime) + margin;
            float x2 = (lengthBySec * endTime) + margin;

            // Index of the note inside the range. Cropping the bottom.
            int noteIndex = event->message.getNoteNumber() - noteRangeBegin;

            // Flipping with (noteRange -) because 0,0 coord is top-right corner. Low notes needs to have higher y values.
            float y = (lineThickness * (noteRange - noteIndex)) + margin;

            midiPath.addLineSegment(juce::Line<float>(x1, y, x2, y), lineThickness);
        }
    }

protected:
    Store &store;
    State &state;

private:
    std::optional<juce::MidiFile> midiFile;
    std::optional<juce::Path> midiPath;
    int noteRangeBegin = 0;
    int noteRangeEnd = 127;
    double midiDurationSec = 0;
    int windowSizeSec = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiRoll)
};

class StartMarker : public juce::Component
{
public:
    void paint(juce::Graphics &g) override
    {
        float triangleSize = float(getWidth());
        float lineSize = 2.0f;
        float halfTriangleSize = triangleSize / 2.0f;

        g.setColour(juce::Colours::whitesmoke.withAlpha(0.5f));

        // Line
        g.fillRect(halfTriangleSize - (lineSize / 2.0), halfTriangleSize, lineSize, float(getHeight()) - halfTriangleSize);

        // Triangle head
        juce::Path triangle;
        triangle.addTriangle(0.0f, 0.0f, halfTriangleSize, halfTriangleSize, triangleSize, 0.0f);
        g.fillPath(triangle);
    }

    void mouseDown(const juce::MouseEvent &e) override
    {
        dragger.startDraggingComponent(this, e);
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        dragger.dragComponent(this, e, &constraints);

        if (onPositionChanged)
            onPositionChanged(getX() + (getWidth() / 2));
    }

    void setPosition(float position)
    {
    }

    void resized() override
    {
        constraints.setMinimumOnscreenAmounts(getHeight(), getWidth() / 2, getHeight(), getWidth() / 2);
    }

    std::function<void(int)> onPositionChanged;

private:
    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer constraints;
};

/**
 * MidiRoll with markers
 */
class MidiPreview : public MidiRoll
{
public:
    MidiPreview(State &state, Store &store) : MidiRoll(state, store)
    {
        addAndMakeVisible(startMarker, 0);

        startMarker.onPositionChanged = [this](int x)
        { startMarkerMoved(x); };
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        startMarker.setBounds(-8, 0, 16, getHeight());
    }

    void mouseDown(const juce::MouseEvent &e) override
    {
        startMarker.setCentrePosition(e.x, getHeight() / 2);
        startMarkerMoved(e.x);
        startMarker.mouseDown(e.getEventRelativeTo(&startMarker));
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        startMarker.mouseDrag(e.getEventRelativeTo(&startMarker));
    }

private:
    void startMarkerMoved(int xPos)
    {
        state.setStartMakerPosition((double)xPos / getWidth());
    }

private:
    StartMarker startMarker;
};
