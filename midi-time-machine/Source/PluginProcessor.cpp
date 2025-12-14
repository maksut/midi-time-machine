#include "PluginProcessor.h"
#include "PluginEditor.h"

MTMAudioProcessor::MTMAudioProcessor()
{
}

MTMAudioProcessor::~MTMAudioProcessor()
{
}

const juce::String MTMAudioProcessor::getName() const
{
    return "MIDI Time Machine";
}

bool MTMAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MTMAudioProcessor::producesMidi() const
{
    return true;
}

bool MTMAudioProcessor::isMidiEffect() const
{
    return true;
}

double MTMAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MTMAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int MTMAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MTMAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String MTMAudioProcessor::getProgramName(int index)
{
    return {};
}

void MTMAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

void MTMAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MTMAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MTMAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    double sampleRate = getSampleRate();

    if (sampleRate != 0 && !midiMessages.isEmpty())
    {
        //
        // Midi messages from MidiBuffer has a timestamp indicating sample position.
        // Not an actual timstamp, not "ticks per quarter".
        // So here we calculate absolute time for them, in milliseconds.
        // It may be possible to capture them with "ticks per quarter" instead,
        // as they are provided from the host with the hosts BPM.
        // However tempo is not always available and can change.
        // It is simpler to capture absolute times. Then later they can be converted to TPQ if needed.
        //

        double millisPerSample = 1000.0 / sampleRate;

        for (const auto metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            auto originalTimestamp = message.getTimeStamp();

            message.setTimeStamp((sampleCount + metadata.samplePosition) * millisPerSample);
            queue.push(message);

            // Not sure this is needed. It is an attempt to keep midi through intact.
            message.setTimeStamp(originalTimestamp);
        }

        // Let the message/editor thread know
        sendChangeMessage();
    }

    sampleCount += buffer.getNumSamples();
}

bool MTMAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *MTMAudioProcessor::createEditor()
{
    return new MTMAudioProcessorEditor(*this);
}

void MTMAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MTMAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

std::vector<juce::MidiMessage> MTMAudioProcessor::popMidiQueue()
{
    std::vector<juce::MidiMessage> messages;

    queue.pop(std::back_inserter(messages));

    return messages;
}

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new MTMAudioProcessor();
}