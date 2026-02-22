#pragma once

#include <JuceHeader.h>

struct WrappedMessage
{
    juce::MidiMessage message;
    bool isPlayback;
};

class MidiQueue
{
public:
    MidiQueue() {}

    void push(const juce::MidiMessage &message, bool isPlayback)
    {
        fifo.write(1)
            .forEach(
                [&](int dest)
                {
                    WrappedMessage &wrapedMessage = mMessages[(size_t)dest];
                    wrapedMessage.message = message;
                    wrapedMessage.isPlayback = isPlayback;
                });
    }

    template <typename OutputIt>
    void pop(OutputIt out)
    {
        fifo.read(fifo.getNumReady())
            .forEach([&](int source)
                     { *out++ = mMessages[(size_t)source]; });
    }

    int size()
    {
        return fifo.getNumReady();
    }

private:
    static constexpr auto mQueueSize = 1 << 14;
    juce::AbstractFifo fifo{mQueueSize};
    std::vector<WrappedMessage> mMessages = std::vector<WrappedMessage>(mQueueSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiQueue)
};
