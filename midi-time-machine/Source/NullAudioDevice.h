#pragma once

#include <JuceHeader.h>

//==============================================================================
// A silent audio device that drives the audio callback via a background thread.
// This gives processBlock its heartbeat with no real hardware involved.
//==============================================================================
class NullAudioDevice : public juce::AudioIODevice,
                        private juce::Thread
{
public:
    NullAudioDevice()
        : juce::AudioIODevice ("No Audio", "No Audio"),
          juce::Thread ("Null Audio Thread") {}

    ~NullAudioDevice() override { close(); }

    // --- Device capabilities ---
    juce::StringArray getOutputChannelNames() override { return {}; }
    juce::StringArray getInputChannelNames()  override { return {}; }

    juce::Array<double> getAvailableSampleRates() override
    {
        return { 44100.0, 48000.0, 96000.0 };
    }

    juce::Array<int> getAvailableBufferSizes() override { return { 128, 256, 512 }; }
    int getDefaultBufferSize() override { return 512; }

    // --- Lifecycle ---
    juce::String open (const juce::BigInteger&, const juce::BigInteger&,
                       double newSampleRate, int newBufferSize) override
    {
        sampleRate = newSampleRate;
        bufferSize = newBufferSize;
        isDeviceOpen = true;
        startThread (juce::Thread::Priority::high);
        return {};
    }

    void close() override
    {
        isDeviceOpen = false;
        stopThread (2000);
    }

    bool isOpen() override { return isDeviceOpen; }

    void start (juce::AudioIODeviceCallback* cb) override
    {
        callback = cb;
        if (callback)
            callback->audioDeviceAboutToStart (this);
    }

    void stop() override
    {
        if (callback)
            callback->audioDeviceStopped();
        callback = nullptr;
    }

    bool isPlaying() override { return callback != nullptr; }

    // --- State ---
    double getCurrentSampleRate()       override { return sampleRate; }
    int    getCurrentBufferSizeSamples() override { return bufferSize; }
    int    getCurrentBitDepth()          override { return 16; }
    int    getOutputLatencyInSamples()   override { return 0; }
    int    getInputLatencyInSamples()    override { return 0; }
    juce::String getLastError()          override { return {}; }

    juce::BigInteger getActiveOutputChannels() const override
    {
        juce::BigInteger b; b.setBit (0); return b;
    }

    juce::BigInteger getActiveInputChannels() const override { return {}; }

private:
    void run() override
    {
        while (! threadShouldExit())
        {
            if (callback != nullptr)
            {
                juce::AudioBuffer<float> buffer (1, bufferSize);
                buffer.clear();

                const float* ins[]  = { nullptr };
                float*       outs[] = { buffer.getWritePointer (0) };

                juce::AudioIODeviceCallbackContext context;
                callback->audioDeviceIOCallbackWithContext (
                    ins,  0,
                    outs, 1,
                    bufferSize, context);
            }

            // Sleep for exactly one buffer period
            wait ((int) (1000.0 * bufferSize / sampleRate));
        }
    }

    juce::AudioIODeviceCallback* callback = nullptr;
    double sampleRate   = 44100.0;
    int    bufferSize   = 512;
    bool   isDeviceOpen = false;
};

//==============================================================================
class NullAudioDeviceType : public juce::AudioIODeviceType
{
public:
    NullAudioDeviceType() : juce::AudioIODeviceType ("No Audio") {}

    void scanForDevices() override {}

    juce::StringArray getDeviceNames (bool) const override
    {
        return { "No Audio" };
    }

    int getDefaultDeviceIndex (bool)                     const override { return 0; }
    int getIndexOfDevice (juce::AudioIODevice*, bool)    const override { return 0; }
    bool hasSeparateInputsAndOutputs()                   const override { return false; }

    juce::AudioIODevice* createDevice (const juce::String&,
                                       const juce::String&) override
    {
        return new NullAudioDevice();
    }
};