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
    int distType { 0 };
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
    
    using MonoChain = juce::dsp::ProcessorChain<Filter, Filter, Gain, WaveShaper, Gain, Gain>;
    
    MonoChain leftChain, rightChain;
    
    enum ChainPositions
    {
        LowShelf,
        Peak,
        Drive,
        Distortion,
        DriveComp,
        Trim
    };
    
    using Coefficients = Filter::CoefficientsPtr;
    
    void setShelfCoeff (float cutoff, float q, float gain);
    void updateColor (const ChainSettings& chainSettings);
    static void updateCoefficients (Coefficients& old, const Coefficients& replacements);
    void updateDrive (const ChainSettings& chainSettings);
    void setFunctionToUse ();
    void updateDriveComp (const ChainSettings& chainSettings);
    void updateTrim (const ChainSettings& chainSettings);
    
    void updateAll ();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Fuzzmeup1AudioProcessor)
};
