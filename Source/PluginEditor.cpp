/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider& slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    g.setColour(OLIVE_GREEN);
    g.fillEllipse(bounds);
    
    g.setColour(GUNMETAL_GRAY);
    g.drawEllipse(bounds, 5.f);
    
    auto center = bounds.getCentre();
    
    Path p;
    
    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());
    
    p.addRectangle(r);
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    
    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
    
    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
    
    g.fillPath(p);
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    return getLocalBounds();
}

//==============================================================================
Fuzzmeup1AudioProcessorEditor::Fuzzmeup1AudioProcessorEditor (Fuzzmeup1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
        driveSlider(*audioProcessor.apvts.getParameter("Drive"), ""),
        colorSlider(*audioProcessor.apvts.getParameter("Color"), ""),
        trimSlider(*audioProcessor.apvts.getParameter("Trim"), "dB"),
        driveSliderAttachment(audioProcessor.apvts, "Drive", driveSlider),
        colorSliderAttachment(audioProcessor.apvts, "Color", colorSlider),
        trimSliderAttachment(audioProcessor.apvts, "Trim", trimSlider)
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
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::COOL_GRAY);
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
