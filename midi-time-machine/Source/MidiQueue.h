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
    void push(const juce::MidiMessage &message, bool isPlayback)
    {
        fifo.write(1)
            .forEach(
                [&](int dest)
                {
                    WrappedMessage &wrapedMessage = messages[(size_t)dest];
                    wrapedMessage.message = message;
                    wrapedMessage.isPlayback = isPlayback;
                });
    }

    template <typename OutputIt>
    void pop(OutputIt out)
    {
        fifo.read(fifo.getNumReady())
            .forEach([&](int source)
                     { *out++ = messages[(size_t)source]; });
    }

    int size()
    {
        return fifo.getNumReady();
    }

private:
    static constexpr auto queueSize = 1 << 14;
    juce::AbstractFifo fifo{queueSize};
    std::vector<WrappedMessage> messages = std::vector<WrappedMessage>(queueSize);
};
