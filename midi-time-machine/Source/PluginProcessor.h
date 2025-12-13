#pragma once

#include <JuceHeader.h>

class MidiQueue
{
public:
    void push(const juce::MidiMessage &message)
    {
        fifo.write(1).forEach([&](int dest)
                              { messages[(size_t)dest] = message; });
    }

    template <typename OutputIt>
    void pop(OutputIt out)
    {
        fifo.read(fifo.getNumReady()).forEach([&](int source)
                                              { *out++ = messages[(size_t)source]; });
    }

private:
    static constexpr auto queueSize = 1 << 14;
    juce::AbstractFifo fifo{queueSize};
    std::vector<juce::MidiMessage> messages = std::vector<juce::MidiMessage>(queueSize);
};

class MTMAudioProcessor : public juce::AudioProcessor, juce::ChangeBroadcaster
{
public:
    MTMAudioProcessor();
    ~MTMAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
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

    std::vector<juce::MidiMessage> popMidiQueue();

private:
    juce::MidiMessageSequence recordedMidiMessages;
    MidiQueue queue;
    juce::int64 sampleCount = 0;
    juce::int64 lastNonZeroSampleRate = 44100;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MTMAudioProcessor)
};
