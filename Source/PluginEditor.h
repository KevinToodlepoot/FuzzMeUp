/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define COOL_GRAY       Colour( 228u, 229u, 232u )
#define GUNMETAL_GRAY   Colour( 83u, 86u, 90u )
#define BURGUNDY        Colour( 77u, 0u, 17u )
#define OLIVE_GREEN     Colour( 75u, 68u, 60u )

#define ROSE_QUARTZ     Colour(246u, 205u, 196u)
#define TANGERINE       Colour(252u, 132u, 31u)
#define LILAC           Colour(180u, 151u, 191u)
#define INDIGO          Colour(93u, 25u, 102u)

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
    juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
    
private:
    LookAndFeel lnf;
    
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

//==============================================================================
/**
*/
class Fuzzmeup1AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Fuzzmeup1AudioProcessorEditor (Fuzzmeup1AudioProcessor&);
    ~Fuzzmeup1AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void loadImages();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Fuzzmeup1AudioProcessor& audioProcessor;
    
    juce::Image bgImage,
                fButtonOff,
                fButtonOn,
                mButtonOff,
                mButtonOn,
                uButtonOff,
                uButtonOn;
    
    RotarySliderWithLabels  driveSlider,
                            colorSlider,
                            trimSlider;
    
    juce::ImageButton   fButton,
                        mButton,
                        uButton;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment  driveSliderAttachment,
                colorSliderAttachment,
                trimSliderAttachment;
    
    APVTS::ButtonAttachment fButtonAttachment,
                            mButtonAttachment,
                            uButtonAttachment;
    
    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Fuzzmeup1AudioProcessorEditor)
};
