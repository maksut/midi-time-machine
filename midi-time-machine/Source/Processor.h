#pragma once

#include "MidiQueue.h"
#include <JuceHeader.h>

class State;
class Store;
class Playback;
class ValueTreeLogger;

class Processor : public juce::AudioProcessor, public juce::ChangeBroadcaster, public juce::ChangeListener, public juce::Timer
{
  public:
    Processor();
    ~Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    // To suppress -Woverloaded-virtual
    void processBlock(juce::AudioBuffer<double> &, juce::MidiBuffer &) override
    {
        jassertfalse;
    }

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    std::vector<WrappedMessage> popMidiQueue();

    void startPlayback(const juce::MidiFile &sequence);
    void stopPlayback();

    State &getState();

  private:
    void flushAndReset();
    bool isHostPlaying();

    MidiQueue mQueue;
    std::unique_ptr<State> mState;
    std::unique_ptr<ValueTreeLogger> mValueTreeLogger;
    std::unique_ptr<Store> mStore;
    juce::int64 mSampleCount = 0;

    juce::CriticalSection mPlaybackLock;
    std::unique_ptr<Playback> mPlaybackRequest;
    std::unique_ptr<Playback> mCurrentlyPlaying;
    juce::Atomic<double> mPlaybackTimeSec = -1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
