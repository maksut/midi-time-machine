#include "Processor.h"
#include "Editor.h"
#include "Store.h"
#include "Playback.h"

Processor::Processor()
{
    store = new Store(*this);

    addParameter(testParam = new juce::AudioParameterFloat(
                     juce::ParameterID{"testParam", 1}, // parameterID
                     "Test Parameter",                  // parameter name
                     0.0f,                              // minimum value
                     1.0f,                              // maximum value
                     0.5f));                            // default value

    playbackRequest = new Playback();
    currentlyPlaying = new Playback();
}

Processor::~Processor()
{
    delete store;
    delete playbackRequest;
    delete currentlyPlaying;
}

const juce::String Processor::getName() const
{
    return "MIDI Time Machine";
}

bool Processor::acceptsMidi() const
{
    return true;
}

bool Processor::producesMidi() const
{
    return true;
}

bool Processor::isMidiEffect() const
{
    return true;
}

double Processor::getTailLengthSeconds() const
{
    return 0.0;
}

int Processor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int Processor::getCurrentProgram()
{
    return 0;
}

void Processor::setCurrentProgram(int index)
{
}

const juce::String Processor::getProgramName(int index)
{
    return {};
}

void Processor::changeProgramName(int index, const juce::String &newName)
{
}

void Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    flushAndReset();
}

void Processor::releaseResources()
{
    flushAndReset();
}

void Processor::reset()
{
    flushAndReset();
}

void Processor::flushAndReset()
{
    store->prepareAndSaveLastMidi();
    sampleCount = 0;
}

void Processor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numOfSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numOfSamples);

    double sampleRate = getSampleRate();

    if (sampleRate != 0)
    {
        double millisPerSample = 1000.0 / sampleRate;

        if (!midiMessages.isEmpty())
        {
            // Midi messages from MidiBuffer have timestamps indicating sample position.
            // Not an actual timstamp, not "ticks per quarter" either.
            // So here we calculate absolute time for them, in milliseconds.
            // It may be possible to capture them with "ticks per quarter" instead,
            // as they are provided from the host with the hosts BPM.
            // However tempo is not always available and it can change.
            // It is simpler to capture absolute times. Then later they can be converted to TPQ if needed.

            for (const auto metadata : midiMessages)
            {
                auto message = metadata.getMessage();
                auto originalTimestamp = message.getTimeStamp();

                message.setTimeStamp((sampleCount + metadata.samplePosition) * millisPerSample);
                queue.push(message);

                // Not sure this is needed. It is an attempt to keep midi through intact.
                message.setTimeStamp(originalTimestamp);
            }

            // Let the listeners know there has been changes (eg. midi messages received).
            sendChangeMessage();
        }

        // Now playback midi if available
        juce::ScopedTryLock tryLock(playbackLock);

        if (tryLock.isLocked())
        {
            if (playbackRequest->isReadyToPlay()) // Is there a new playback request?
            {
                // Mark the currentlyPlaying sequence as done
                currentlyPlaying->stop();

                // Then swap the pointers
                std::swap(currentlyPlaying, playbackRequest);
            }
        }

        // If there is a current playback ready then play it
        if (currentlyPlaying->isReadyToPlay())
            currentlyPlaying->play(midiMessages, numOfSamples, millisPerSample);
    }

    // Finally update the sampleCount. Using it to keep track of time.
    sampleCount += numOfSamples;
}

void Processor::startPlayback(const juce::MidiFile &midiFile)
{
    juce::ScopedLock lock(playbackLock);

    playbackRequest->start(midiFile);
}

void Processor::stopPlayback()
{
    juce::ScopedLock lock(playbackLock);

    playbackRequest->stop();
}

bool Processor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *Processor::createEditor()
{
    return new Editor(*this, *store);
}

void Processor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream(destData, true).writeFloat(*testParam);
}

void Processor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    *testParam = juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readFloat();
}

std::vector<juce::MidiMessage> Processor::popMidiQueue()
{
    std::vector<juce::MidiMessage> messages;

    queue.pop(std::back_inserter(messages));

    return messages;
}

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new Processor();
}