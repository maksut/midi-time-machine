#include "Editor.h"

Editor::Editor(Processor &inProcessor, Store &midiStore)
    : AudioProcessorEditor(&inProcessor),
      mState(inProcessor.getState()),
      mStore(midiStore),
      mMidiRoll(mState, mStore, 5),
      mMidiPreview(mState, mStore, mMidiRoll),
      mToolbar(inProcessor, mStore, mMidiPreview, mMidiRoll),
      mContent(mState, mStore, mMidiRoll)
{
    mOpenGLContext.attachTo(*this);

    addAndMakeVisible(mToolbar);
    addAndMakeVisible(mContent);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(800, 400);
    setResizable(true, true);

    mState.addListener(this);
}

Editor::~Editor()
{
    mState.removeListener(this);
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
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.templateRows = {Track(Fr(1)), Track(Fr(5))};
    grid.templateColumns = {Track(Fr(1))};
    grid.items = {juce::GridItem(mToolbar), juce::GridItem(mContent)};
    grid.performLayout(getLocalBounds());
}

void Editor::valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property)
{
    if (mState.isSettinsOpenChange(tree, property))
    {
    }
}