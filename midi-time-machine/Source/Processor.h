#pragma once

#include <JuceHeader.h>
#include "MidiQueue.h"

class State;
class Store;
class Playback;

class Processor : public juce::AudioProcessor, public juce::ChangeBroadcaster, public juce::ChangeListener
{
public:
    Processor();
    ~Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

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

    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    std::vector<WrappedMessage> popMidiQueue();

    void startPlayback(const juce::MidiFile &sequence);
    void stopPlayback();

    State &getState();

private:
    void flushAndReset();
    bool isHostPlaying();

    juce::AudioParameterFloat *testParam;

    MidiQueue queue;
    std::unique_ptr<State> state;
    std::unique_ptr<Store> store;
    juce::MidiMessageSequence recordedMidiMessages;
    juce::int64 sampleCount = 0;
    juce::int64 lastNonZeroSampleRate = 44100;

    juce::CriticalSection playbackLock;
    std::unique_ptr<Playback> playbackRequest;
    std::unique_ptr<Playback> currentlyPlaying;
    juce::Atomic<bool> playbackInProgress = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
