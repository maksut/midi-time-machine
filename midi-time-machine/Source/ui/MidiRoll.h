#pragma once

#include <JuceHeader.h>
#include "../Store.h"
#include "../State.h"
#include "RoundedComponent.h"

class MidiRollBase : public RoundedComponent, public juce::ValueTree::Listener
{
public:
    MidiRollBase(State &state, Store &store) : mState(state), mStore(store)
    {
        state.addListener(this);
    }

    ~MidiRollBase() override
    {
        mState.removeListener(this);
    }

    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property) override
    {
        if (mState.isPlaybackTimeSecChange(tree, property))
        {
            mPlaybackTimeSec = mState.getPlaybackTimeSec();
            repaint();
        }
    }

    virtual void load(juce::MidiFile &midiFileArg)
    {
        // Reset state
        unload();
        mMidiFile.emplace(juce::MidiFile(midiFileArg)); // Get a clone of it
        mMidiPath.emplace(juce::Path());

        //
        // Copy the midiFile, update matched pairs and convert ticks to seconds
        // Also calculate note range for the file.
        //

        mNoteRangeBegin = std::numeric_limits<int>().max();
        mNoteRangeEnd = std::numeric_limits<int>().min();

        for (int trackIndex = 0; trackIndex < mMidiFile->getNumTracks(); ++trackIndex)
        {
            const juce::MidiMessageSequence *sequence = mMidiFile->getTrack(trackIndex);

            for (int eventIndex = 0; eventIndex < sequence->getNumEvents(); ++eventIndex)
            {
                auto &message = sequence->getEventPointer(eventIndex)->message;

                if (!message.isNoteOn())
                    continue;

                mNoteRangeBegin = juce::jmin(mNoteRangeBegin, message.getNoteNumber());
                mNoteRangeEnd = juce::jmax(mNoteRangeEnd, message.getNoteNumber());
            }
        }

        mMidiFile->convertTimestampTicksToSeconds();

        // Calculate duration
        for (int trackIndex = 0; trackIndex < mMidiFile->getNumTracks(); ++trackIndex)
            mMidiDurationSec = juce::jmax(mMidiDurationSec, mMidiFile->getTrack(trackIndex)->getEndTime());

        // Draw it into the midiPath
        for (int trackIndex = 0; trackIndex < mMidiFile->getNumTracks(); ++trackIndex)
            drawTrack(*mMidiPath, mMidiFile->getTrack(trackIndex));

        repaint();
    }

    void unload()
    {
        mMidiFile.reset();
        mMidiPath.reset();
        mNoteRangeBegin = 0;
        mNoteRangeEnd = 127;
        mMidiDurationSec = 0.0f;
        repaint();
        mPlaybackTimeSec = -1.0f;
    }

private:
    void drawTrack(juce::Path &midiPath, const juce::MidiMessageSequence *sequence)
    {
        double durationSec = sequence->getEndTime();
        int initialNoteRange = mNoteRangeEnd - mNoteRangeBegin + 1;
        int noteRange = juce::jmax(initialNoteRange, 20); // if the key range is too narrow than make it 20 lines
     
        float horizontalMargin = 1.0f / (noteRange + 4); // 2 line thickness horizontal margin
        float lineThickness = (1.0f - (2 * horizontalMargin)) / noteRange;

        double lengthBySec = 1.0f / durationSec;

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

            float x1 = (lengthBySec * startTime);
            float x2 = (lengthBySec * endTime);

            // Index of the note inside the range. Cropping the bottom.
            int noteIndex = event->message.getNoteNumber() - mNoteRangeBegin;

            // Flipping with (noteRange -) because 0,0 coord is top-right corner. Low notes needs to have higher y values.
            float pushToCenter = (noteRange - initialNoteRange) * lineThickness / 2.0f;
            float y = (lineThickness * (noteRange - noteIndex)) + horizontalMargin - pushToCenter;

            midiPath.addLineSegment(juce::Line<float>(x1, y, x2, y), lineThickness);
        }
    }

protected:
    State &mState;
    Store &mStore;

    std::optional<juce::Path> mMidiPath;
    double mMidiDurationSec = 0.0f;
    double mPlaybackTimeSec = -1.0f;

private:
    std::optional<juce::MidiFile> mMidiFile;
    int mNoteRangeBegin = 0;
    int mNoteRangeEnd = 127;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiRollBase)
};

class MidiRoll : public MidiRollBase
{
public:
    MidiRoll(State &state, Store &store, int windowSizeSec) : MidiRollBase(state, store), mWindowSizeSec(windowSizeSec)
    {
        mBackgroundColour = juce::Colours::darkgrey;
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRect(getLocalBounds());

        if (mMidiPath && mWindowSizeSec > 0)
        {
            g.setColour(juce::Colours::black);
            juce::Rectangle<int> rect = getLocalBounds();

            // Draw a slice of the midi path
            g.saveState();
            float visibleWidthPerSec = rect.getWidth() / mWindowSizeSec;
            float width = visibleWidthPerSec * mMidiDurationSec;
            float scroll = 0.0f;

            if (mStartMarkerMoving || mPlaybackTimeSec < 0)
                scroll = width * mStartMarkerPos;
            else if (mPlaybackTimeSec > 0)
                scroll = visibleWidthPerSec * mPlaybackTimeSec;

            if (scroll > 0.0f)
                g.addTransform(juce::AffineTransform::translation(-1.0f * scroll, 0.0f));

            g.addTransform(juce::AffineTransform::scale(width, rect.getHeight()));

            g.fillPath(*mMidiPath);
            g.restoreState();

            // Draw a playhead
            if (mStartMarkerMoving && mPlaybackTimeSec > 0)
            {
                g.setColour(juce::Colours::yellow.withAlpha(0.4f));

                float x = (mPlaybackTimeSec * visibleWidthPerSec) - scroll;
                float y = rect.getY();

                g.drawLine(x, y, x, y + rect.getHeight(), 2.0f);
            }
        }
    }

    void setStartMarkerState(double startMarkerPos, bool startMarkerMoving)
    {
        this->mStartMarkerPos = startMarkerPos;
        this->mStartMarkerMoving = startMarkerMoving;
        repaint();
    }

    void mouseDown(const juce::MouseEvent &) override
    {
        mIsDragging = false;
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        if (e.getDistanceFromDragStart() < 8 || mIsDragging)
            return;

        juce::File midiFile(mState.getSelectedMidiFile().trim());

        if (!midiFile.existsAsFile())
            return;

        mIsDragging = true;

        mTempMidiFile.reset(new juce::TemporaryFile(".mid"));

        if (!midiFile.copyFileTo(mTempMidiFile->getFile()))
        {
            mTempMidiFile.reset();

            return;
        }

        juce::StringArray filesToDrag;
        filesToDrag.add(mTempMidiFile->getFile().getFullPathName());

        juce::DragAndDropContainer::performExternalDragDropOfFiles(filesToDrag, true);
    }

    void mouseUp(const juce::MouseEvent &) override
    {
        mIsDragging = false;
    }

private:
    std::unique_ptr<juce::TemporaryFile> mTempMidiFile;
    bool mIsDragging = false;
    int mWindowSizeSec = -1.0f;
    double mStartMarkerPos = 0.0f;
    bool mStartMarkerMoving = false;

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
        mDragger.startDraggingComponent(this, e);

        if (onChange)
            onChange(getX() + (getWidth() / 2), true);
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        mDragger.dragComponent(this, e, &mConstraints);

        if (onChange)
            onChange(getX() + (getWidth() / 2), true);
    }

    void mouseUp(const juce::MouseEvent & /*e*/) override
    {
        if (onChange)
            onChange(getX() + (getWidth() / 2), false);
    }

    void resized() override
    {
        mConstraints.setMinimumOnscreenAmounts(getHeight(), getWidth() / 2, getHeight(), getWidth() / 2);
    }

    std::function<void(int posX, bool isMoving)> onChange;

private:
    juce::ComponentDragger mDragger;
    juce::ComponentBoundsConstrainer mConstraints;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartMarker)
};

/**
 * MidiRoll with markers
 */
class MidiPreview : public MidiRollBase
{
public:
    MidiPreview(State &state, Store &store, MidiRoll &midiRoll) : MidiRollBase(state, store), mMidiRoll(midiRoll)
    {
        addAndMakeVisible(mStartMarker, 0);

        mStartMarker.onChange = [this](int x, bool isMoving)
        { startMarkerMoved(x, isMoving); };
    }

    void load(juce::MidiFile &midiFileArg) override
    {
        MidiRollBase::load(midiFileArg);

        // Reset the start marker
        mState.setStartMarkerPosition(0.0f);
        resized();
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRect(getLocalBounds());

        if (mMidiPath)
        {
            // Draw the midi path
            g.saveState();
            g.setColour(juce::Colours::black);
            juce::Rectangle<int> rect = getLocalBounds();

            g.addTransform(juce::AffineTransform::scale(rect.getWidth(), rect.getHeight()));
            g.fillPath(*mMidiPath);
            g.restoreState();

            // Draw the play head
            g.setColour(juce::Colours::yellow.withAlpha(0.4f));

            if (mPlaybackTimeSec > 0)
            {
                float x = (mPlaybackTimeSec / mMidiDurationSec) * rect.getWidth();
                float y = rect.getY();
                g.drawLine(x, y, x, y + rect.getHeight(), 2.0f);
            }
        }
    }

    void resized() override
    {
        mStartMarker.setBounds(-8, 0, 16, getHeight());

        // Reset start marker position from the state
        double startMarkerPos = mState.getStartMarkerPosition();
        mStartMarker.setCentrePosition(startMarkerPos * getWidth(), getHeight() / 2);
    }

    void mouseDown(const juce::MouseEvent &e) override
    {
        mStartMarker.setCentrePosition(e.x, getHeight() / 2);
        mStartMarker.mouseDown(e.getEventRelativeTo(&mStartMarker));
    }

    void mouseUp(const juce::MouseEvent &e) override
    {
        mStartMarker.mouseUp(e.getEventRelativeTo(&mStartMarker));
    }

    void mouseDrag(const juce::MouseEvent &e) override
    {
        mStartMarker.mouseDrag(e.getEventRelativeTo(&mStartMarker));
    }

private:
    void startMarkerMoved(int posX, bool isMoving)
    {
        // Set start marker position of the state and the midi roll
        double startMarkerPos = (double)posX / getWidth();
        mState.setStartMarkerPosition(startMarkerPos);
        mMidiRoll.setStartMarkerState(startMarkerPos, isMoving);
    }

private:
    StartMarker mStartMarker;
    MidiRoll &mMidiRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiPreview)
};
