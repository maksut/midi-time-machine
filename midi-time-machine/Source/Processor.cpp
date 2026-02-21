#include "Processor.h"
#include "ui/Editor.h"
#include "Store.h"
#include "Playback.h"
#include "ValueTreeLogger.h"

const int POLL_TIME_MILLIS = 1000 / 120; // ~ 60Hz

Processor::Processor()
{
    state.reset(new State());
    valueTreeLogger.reset(new ValueTreeLogger(*state));
    store.reset(new Store(*this));

    playbackRequest.reset(new Playback());
    currentlyPlaying.reset(new Playback());

    // Processor is listening itself!. The change events are triggered in the audio thread.
    // But the listener callbacks are executed in the non-audio threads.
    // Then the processor modifies related bit of the ValueTree in the state.
    // So other listener can listen for specific changes.
    addChangeListener(this);
}

Processor::~Processor()
{
    removeChangeListener(this);
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
    int initialNumEvents = midiMessages.getNumEvents();

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

        // Playback midi if available
        juce::ScopedTryLock tryLock(playbackLock);

        if (tryLock.isLocked())
        {
            if (playbackRequest->isReadyToPlay()) // Is there a new playback request?
            {
                // Mark the currentlyPlaying sequence as done
                currentlyPlaying->stop(midiMessages, numOfSamples);
                playbackTimeSec = -1;

                // Then swap the pointers
                std::swap(currentlyPlaying, playbackRequest);

                sendChangeMessage();
            }
        }

        // If there is a current playback ready then play it
        if (currentlyPlaying->isReadyToPlay())
        {
            playbackTimeSec = currentlyPlaying->play(midiMessages, numOfSamples, millisPerSample);

            if (playbackTimeSec.get() < 0.0f)
            {
                // The playback is stopped. All midi messages are played.
                sendChangeMessage();
            }
        }

        // Now process all messages. Both coming from the host and from the plugin playback above.
        if (!midiMessages.isEmpty())
        {
            // Midi messages from MidiBuffer have timestamps indicating sample position.
            // Not an actual timstamp, not "ticks per quarter" either.
            // So here we calculate absolute time for them, in milliseconds.
            // It may be possible to capture them with "ticks per quarter" instead,
            // as they are provided from the host with the hosts BPM.
            // However tempo is not always available and it can change.
            // It is simpler to capture absolute times. Then later they can be converted to TPQ if needed.

            int numMessages = 0;
            for (const auto metadata : midiMessages)
            {
                juce::MidiMessage message = metadata.getMessage();
                double originalTimestamp = message.getTimeStamp();

                message.setTimeStamp((sampleCount + metadata.samplePosition) * millisPerSample);
                ++numMessages;

                bool isPluginPlayback = numMessages > initialNumEvents;
                queue.push(message, isHostPlaying() || isPluginPlayback);

                // Not sure this is needed. It is an attempt to keep midi through intact.
                message.setTimeStamp(originalTimestamp);
            }

            // Let the listeners know there has been changes (eg. midi messages received).
            sendChangeMessage();
        }
    }

    // Finally update the sampleCount. Using it to keep track of time.
    sampleCount += numOfSamples;
}

void Processor::startPlayback(const juce::MidiFile &midiFile)
{
    juce::ScopedLock lock(playbackLock);

    playbackRequest->start(midiFile, state->getStartMarkerPosition());
}

void Processor::stopPlayback()
{
    juce::ScopedLock lock(playbackLock);

    // Send a playback request with an empty file.
    // So the processor will do its usual swapping of currentlyPlaying & playbackRequest.
    // Slightly hacky but simplifies the state management of these requests.
    juce::MidiFile emptyMidiFile;
    playbackRequest->start(emptyMidiFile, 0.0f);
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
    if (auto xml = state->toXml())
        copyXmlToBinary(*xml, destData);

    // Reset the state properties according the Processor state
    state->setPlaybackTimeSec(playbackTimeSec.get());
}

void Processor::setStateInformation(const void *data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
        state->fromXml(xmlState);
}

void Processor::timerCallback()
{
    state->setPlaybackTimeSec(playbackTimeSec.get());
}

void Processor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (source != this)
        return;

    store->drainProcessorMidiQueue();

    double playbackTime = playbackTimeSec.get();
    state->setPlaybackTimeSec(playbackTime);

    // Only run the timer during playback to save tiny bit of processing
    if (playbackTime >= 0)
        startTimer(POLL_TIME_MILLIS);
    else
        stopTimer();
}

State &Processor::getState()
{
    return *state;
}

std::vector<WrappedMessage> Processor::popMidiQueue()
{
    std::vector<WrappedMessage> messages;

    queue.pop(std::back_inserter(messages));

    return messages;
}

bool Processor::isHostPlaying()
{
    if (auto *playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo positionInfo;

        if (auto position = playHead->getPosition())
            return position->getIsPlaying();
    }

    return false;
}

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new Processor();
}