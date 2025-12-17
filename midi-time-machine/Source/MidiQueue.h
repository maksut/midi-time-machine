#pragma once

#include <JuceHeader.h>

class MidiQueue
{
public:
    void push(const juce::MidiMessage &message)
    {
        fifo.write(1)
            .forEach([&](int dest)
                     { messages[(size_t)dest] = message; });
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
    std::vector<juce::MidiMessage> messages = std::vector<juce::MidiMessage>(queueSize);
};
