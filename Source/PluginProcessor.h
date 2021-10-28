/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings
{
    float drive { 0 };
    float color { 0 };
    float trim { 0 };
    bool fButtonState { 0 };
    bool mButtonState { 0 };
    bool uButtonState { 0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class Fuzzmeup1AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Fuzzmeup1AudioProcessor();
    ~Fuzzmeup1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float fuzzExp1 (float x, float k);
    
    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    
    using Gain = juce::dsp::Gain<float>;
    
    using WaveShaper = juce::dsp::WaveShaper<float>;
    
    using Bias = juce::dsp::Bias<float>;
    
    using MonoChain = juce::dsp::ProcessorChain<Filter,
                                                Filter,
                                                Gain,
                                                Bias,
                                                WaveShaper,
                                                WaveShaper,
                                                Gain,
                                                Filter,
                                                Filter,
                                                Filter,
                                                Filter,
                                                Gain>;
    
    MonoChain leftChain, rightChain;
    
    enum ChainPositions
    {
        LowShelf,
        Peak,
        Drive,
        BiasPos,
        Distortion1,
        Distortion2,
        DriveComp,
        dcBlock1,
        dcBlock2,
        dcBlock3,
        dcBlock4,
        Trim
    };
    
    using Coefficients = Filter::CoefficientsPtr;
    
    void updateShelfCoefficients (const ChainSettings& chainSettings);
    void updateColor (const ChainSettings& chainSettings);
    void updateBias (const ChainSettings& chainSettings);
    static void updateCoefficients (Coefficients& old, const Coefficients& replacements);
    void updateDrive (const ChainSettings& chainSettings);
    void setFirstFunctionToUse(bool onOff);
    void setSecondFunctionToUse ();
    void updateDriveComp (const ChainSettings& chainSettings);
    void updateDCBlock (const ChainSettings& chainSettings);
    void updateTrim (const ChainSettings& chainSettings);
    
    void updateAll ();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Fuzzmeup1AudioProcessor)
};
