#pragma once

#include <JuceHeader.h>
#include "../Store.h"
#include "../State.h"

class MidiRollBase : public juce::Component, public juce::ValueTree::Listener
{
public:
    MidiRollBase(State &state, Store &store) : state(state), store(store)
    {
        state.addListener(this);
    }

    ~MidiRollBase() override
    {
        state.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (state.isPlaybackTimeSecChange(tree, property))
        {
            playbackTimeSec = state.getPlaybackTimeSec();
            repaint();
        }
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
        midiDurationSec = 0.0f;
        repaint();
        playbackTimeSec = -1.0f;
    }

private:
    void drawTrack(juce::Path &midiPath, const juce::MidiMessageSequence *sequence)
    {
        double durationSec = sequence->getEndTime();
        int noteRange = juce::jmax(noteRangeEnd - noteRangeBegin + 1, 40);

        float margin = 0.0f; // 1.0f / (noteRange + 4); // 2 line thickness margin
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

    std::optional<juce::Path> midiPath;
    double midiDurationSec = 0.0f;
    double playbackTimeSec = -1.0f;

private:
    std::optional<juce::MidiFile> midiFile;
    int noteRangeBegin = 0;
    int noteRangeEnd = 127;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiRollBase)
};

class MidiRoll : public MidiRollBase
{
public:
    MidiRoll(State &state, Store &store, int windowSizeSec) : MidiRollBase(state, store), windowSizeSec(windowSizeSec)
    {
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

        if (midiPath && windowSizeSec > 0)
        {
            g.setColour(juce::Colours::black);
            juce::Rectangle<int> rect = getLocalBounds();

            // Draw a slice of the midi path
            g.saveState();
            float visibleWidthPerSec = rect.getWidth() / windowSizeSec;
            float width = visibleWidthPerSec * midiDurationSec;
            float scroll = 0.0f;

            if (startMarkerMoving || playbackTimeSec < 0)
                scroll = width * startMarkerPos;
            else if (playbackTimeSec > 0)
                scroll = visibleWidthPerSec * playbackTimeSec;

            if (scroll > 0.0f)
                g.addTransform(juce::AffineTransform::translation(-1.0f * scroll, 0.0f));

            g.addTransform(juce::AffineTransform::scale(width, rect.getHeight()));

            g.fillPath(*midiPath);
            g.restoreState();

            // Draw a playhead
            if (startMarkerMoving && playbackTimeSec > 0)
            {
                g.setColour(juce::Colours::yellow.withAlpha(0.4f));

                float x = (playbackTimeSec * visibleWidthPerSec) - scroll;
                float y = rect.getY();

                g.drawLine(x, y, x, y + rect.getHeight(), 2.0f);
            }
        }
    }

    void setStartMarkerState(double startMarkerPos, bool startMarkerMoving)
    {
        this->startMarkerPos = startMarkerPos;
        this->startMarkerMoving = startMarkerMoving;
        repaint();
    }

private:
    int windowSizeSec = -1.0f;
    double startMarkerPos = 0.0f;
    bool startMarkerMoving = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiRoll)
};

class StartMarker : public juce::Component
{
public:
    StartMarker() {}

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

        if (onChange)
            onChange(getX() + (getWidth() / 2), true);
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        dragger.dragComponent(this, e, &constraints);

        if (onChange)
            onChange(getX() + (getWidth() / 2), true);
    }

    void mouseUp(const juce::MouseEvent &e) override
    {
        if (onChange)
            onChange(getX() + (getWidth() / 2), false);
    }

    void resized() override
    {
        constraints.setMinimumOnscreenAmounts(getHeight(), getWidth() / 2, getHeight(), getWidth() / 2);
    }

    std::function<void(int posX, bool isMoving)> onChange;

private:
    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer constraints;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartMarker)
};

/**
 * MidiRoll with markers
 */
class MidiPreview : public MidiRollBase
{
public:
    MidiPreview(State &state, Store &store, MidiRoll &midiRoll) : MidiRollBase(state, store), midiRoll(midiRoll)
    {
        addAndMakeVisible(startMarker, 0);

        startMarker.onChange = [this](int x, bool isMoving)
        { startMarkerMoved(x, isMoving); };
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

        if (midiPath)
        {
            // Draw the midi path
            g.saveState();
            g.setColour(juce::Colours::black);
            juce::Rectangle<int> rect = getLocalBounds();

            g.addTransform(juce::AffineTransform::scale(rect.getWidth(), rect.getHeight()));
            g.fillPath(*midiPath);
            g.restoreState();

            // Draw the play head
            g.setColour(juce::Colours::yellow.withAlpha(0.4f));

            if (playbackTimeSec > 0)
            {
                float x = (playbackTimeSec / midiDurationSec) * rect.getWidth();
                float y = rect.getY();
                g.drawLine(x, y, x, y + rect.getHeight(), 2.0f);
            }
        }
    }

    void resized() override
    {
        startMarker.setBounds(-8, 0, 16, getHeight());

        // Reset start marker position from the state
        double startMarkerPos = state.getStartMarkerPosition();
        startMarker.setCentrePosition(startMarkerPos * getWidth(), getHeight() / 2);
    }

    void mouseDown(const juce::MouseEvent &e) override
    {
        startMarker.setCentrePosition(e.x, getHeight() / 2);
        startMarker.mouseDown(e.getEventRelativeTo(&startMarker));
    }

    void mouseUp(const juce::MouseEvent &e) override
    {
        startMarker.mouseUp(e.getEventRelativeTo(&startMarker));
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        startMarker.mouseDrag(e.getEventRelativeTo(&startMarker));
    }

private:
    void startMarkerMoved(int posX, bool isMoving)
    {
        // Set start marker position of the state and the midi roll
        double startMarkerPos = (double)posX / getWidth();
        state.setStartMarkerPosition(startMarkerPos);
        midiRoll.setStartMarkerState(startMarkerPos, isMoving);
    }

private:
    StartMarker startMarker;
    MidiRoll &midiRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiPreview)
};
