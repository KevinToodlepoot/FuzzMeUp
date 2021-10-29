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
    
    /* Slider Fill */
    g.setColour(TANGERINE);
    g.setGradientFill(juce::ColourGradient(TANGERINE, bounds.getCentreX(), bounds.getCentreY(), DARK_TANGERINE, bounds.getCentreX() + (bounds.getWidth() * 0.85f), bounds.getCentreY() + (bounds.getHeight() * 0.85f), true));
    g.fillEllipse(bounds);
    
    /* Slider outline */
    g.setColour(DARK_TANGERINE);
    g.drawEllipse(bounds, 3.f);
    
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        /* rotary line */
        auto center = bounds.getCentre();
        
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 4);
        r.setRight(center.getX() + 4);
        r.setTop(bounds.getY() + 6);
//        r.setBottom(center.getY() - rswl->getTextHeight() * 2.f);
        r.setBottom(bounds.getY() + 14);
        
        p.addEllipse(r);
        
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        /* display text in slider */
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentreX(), bounds.getBottom() + rswl->getTextHeight());
//        r.setCentre(bounds.getCentre());
        
//        g.setColour(Colours::black);
//        g.fillRect(r);
        
        g.setColour(LILAC);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
    
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(LILAC);
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    juce::String str;
    bool addK = false;
    
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        
        if (suffix.compare("dB") == 0 && val > 0.f)
        {
            str = "+";
            str << juce::String(val, 1);
        }
        else if (val > 999.f)
        {
            val /= 1000.f;
            addK = true;
            str = juce::String(val, 1);
        }
        else
        {
            str = juce::String(val, 1);
        }
    }
    else
    {
        jassertfalse;
    }
    
    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "k";
            
        str << suffix;
        
    }
    
    return str;
}

//==============================================================================
Fuzzmeup1AudioProcessorEditor::Fuzzmeup1AudioProcessorEditor (Fuzzmeup1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
        driveSlider(*audioProcessor.apvts.getParameter("Drive"), ""),
        colorSlider(*audioProcessor.apvts.getParameter("Color"), "Hz"),
        trimSlider(*audioProcessor.apvts.getParameter("Trim"), "dB"),
        driveSliderAttachment(audioProcessor.apvts, "Drive", driveSlider),
        colorSliderAttachment(audioProcessor.apvts, "Color", colorSlider),
        trimSliderAttachment(audioProcessor.apvts, "Trim", trimSlider),
        fButtonAttachment(audioProcessor.apvts, "F", fButton),
        mButtonAttachment(audioProcessor.apvts, "M", mButton),
        uButtonAttachment(audioProcessor.apvts, "U", uButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    loadImages();
    
    driveSlider.labels.add({0.f, "0"});
    driveSlider.labels.add({1.f, "10"});
    colorSlider.labels.add({0.f, "20Hz"});
    colorSlider.labels.add({1.f, "10kHz"});
    trimSlider.labels.add({0.f, "-24dB"});
    trimSlider.labels.add({1.f, "+6dB"});
    
    for ( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    fButton.setImages(false, true, true, fButtonOff, 1.f, juce::Colour(), juce::Image(), 1.f, juce::Colour(), fButtonOn, 1.f, juce::Colour());
    
    mButton.setImages(false, true, true, mButtonOff, 1.f, juce::Colour(), juce::Image(), 1.f, juce::Colour(), mButtonOn, 1.f, juce::Colour());
    
    uButton.setImages(false, true, true, uButtonOff, 1.f, juce::Colour(), juce::Image(), 1.f, juce::Colour(), uButtonOn, 1.f, juce::Colour());
        
    fButton.setClickingTogglesState(true);
    mButton.setClickingTogglesState(true);
    uButton.setClickingTogglesState(true);
    
    fButton.setRadioGroupId(1001);
    mButton.setRadioGroupId(1001);
    uButton.setRadioGroupId(1001);
    
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
//    g.fillAll (juce::INDIGO);
    g.drawImage(bgImage, 0, 0, 500, 500, 0, 0, 500, 500);
}

void Fuzzmeup1AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
        
    bounds.removeFromTop(bounds.getHeight() * 0.25);
    
    auto colorArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto trimArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
        
    colorSlider.setBounds(colorArea.removeFromBottom(colorArea.getHeight() * 0.75));
    trimSlider.setBounds(trimArea.removeFromBottom(trimArea.getHeight() * 0.75));
    
    driveSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    
    bounds.removeFromTop(bounds.getHeight() * 0.2);
    bounds.removeFromBottom(bounds.getHeight() * 0.1);
    auto fButtonArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto uButtonArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    fButton.setBounds(fButtonArea.removeFromBottom(fButtonArea.getHeight() * 0.75));
    uButton.setBounds(uButtonArea.removeFromBottom(uButtonArea.getHeight() * 0.75));
    mButton.setBounds(bounds.removeFromBottom(bounds.getHeight() * 0.75));
}

void Fuzzmeup1AudioProcessorEditor::loadImages()
{
    bgImage = juce::ImageCache::getFromMemory(BinaryData::BG_png,
                                              BinaryData::BG_pngSize);
    
    if (bgImage.isNull())
        jassert(bgImage.isNull());
    
    fButtonOff = juce::ImageCache::getFromMemory(BinaryData::FButtonOff_png,
                                                 BinaryData::FButtonOff_pngSize);
    
    if (fButtonOff.isNull())
        jassert(fButtonOff.isNull());
    
    fButtonOn = juce::ImageCache::getFromMemory(BinaryData::FButtonOn_png,
                                                 BinaryData::FButtonOn_pngSize);
    
    if (fButtonOn.isNull())
        jassert(fButtonOn.isNull());
    
    mButtonOff = juce::ImageCache::getFromMemory(BinaryData::MButtonOff_png,
                                                 BinaryData::MButtonOff_pngSize);
    
    if (mButtonOff.isNull())
        jassert(mButtonOff.isNull());
    
    mButtonOn = juce::ImageCache::getFromMemory(BinaryData::MButtonOn_png,
                                                 BinaryData::MButtonOn_pngSize);
    
    if (mButtonOn.isNull())
        jassert(mButtonOn.isNull());
    
    uButtonOff = juce::ImageCache::getFromMemory(BinaryData::UButtonOff_png,
                                                 BinaryData::UButtonOff_pngSize);
    
    if (uButtonOff.isNull())
        jassert(uButtonOff.isNull());
    
    uButtonOn = juce::ImageCache::getFromMemory(BinaryData::UButtonOn_png,
                                                 BinaryData::UButtonOn_pngSize);
    
    if (uButtonOn.isNull())
        jassert(uButtonOn.isNull());
}

std::vector<juce::Component*> Fuzzmeup1AudioProcessorEditor::getComps()
{
    return
    {
        &driveSlider,
        &colorSlider,
        &trimSlider,
        &fButton,
        &mButton,
        &uButton
    };
}
