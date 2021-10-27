/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Fuzzmeup1AudioProcessorEditor::Fuzzmeup1AudioProcessorEditor (Fuzzmeup1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for ( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    setSize (500, 500);
}

Fuzzmeup1AudioProcessorEditor::~Fuzzmeup1AudioProcessorEditor()
{
}

//==============================================================================
void Fuzzmeup1AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Fuzzmeup1AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto titleArea = bounds.removeFromTop(bounds.getHeight() * 0.25);
    
    auto colorArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto trimArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    colorSlider.setBounds(colorArea.removeFromBottom(colorArea.getHeight() * 0.75));
    trimSlider.setBounds(trimArea.removeFromBottom(trimArea.getHeight() * 0.75));
    
    driveSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    
}

std::vector<juce::Component*> Fuzzmeup1AudioProcessorEditor::getComps()
{
    return
    {
        &driveSlider,
        &colorSlider,
        &trimSlider
    };
}
