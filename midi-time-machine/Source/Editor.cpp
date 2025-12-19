#include "Editor.h"

Editor::Editor(Processor &processor, Store &midiStore)
    : AudioProcessorEditor(&processor), processor(processor), store(midiStore), toolbar(processor, store)
{
    addAndMakeVisible(toolbar);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
    setResizable(true, true);

    // Start listening the processor for changes
    processor.addChangeListener(this);
}

Editor::~Editor()
{
    // And stop listening the processor
    processor.removeChangeListener(this);
}

void Editor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    g.drawFittedText("Hello World! Hebele", getLocalBounds(), juce::Justification::centred, 1);
}

void Editor::resized()
{
    auto windowBounds = getLocalBounds();

    auto oneFifth = windowBounds.getHeight() / 5;
    windowBounds.setHeight(oneFifth);
    windowBounds.setY(windowBounds.getHeight() - oneFifth);
    toolbar.setBounds(windowBounds);
}

void Editor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
}
