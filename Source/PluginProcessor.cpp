/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Fuzzmeup1AudioProcessor::Fuzzmeup1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Fuzzmeup1AudioProcessor::~Fuzzmeup1AudioProcessor()
{
}

//==============================================================================
const juce::String Fuzzmeup1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Fuzzmeup1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Fuzzmeup1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Fuzzmeup1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Fuzzmeup1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Fuzzmeup1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Fuzzmeup1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Fuzzmeup1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Fuzzmeup1AudioProcessor::getProgramName (int index)
{
    return {};
}

void Fuzzmeup1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Fuzzmeup1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    
    spec.numChannels = 1;
    
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    auto chainSettings = getChainSettings(apvts);
    
    setShelfCoeff(20.f, 0.5f, 2.f);
    
    updateColor(chainSettings);
    
    updateDrive(chainSettings);
    
    setFunctionToUse();
    
    updateDriveComp(chainSettings);
    
    updateTrim(chainSettings);
    
}

void Fuzzmeup1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Fuzzmeup1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Fuzzmeup1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto chainSettings = getChainSettings(apvts);
    
    updateColor(chainSettings);

    updateDrive(chainSettings);
    
    updateDriveComp(chainSettings);
    
    updateTrim(chainSettings);
    
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
     
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

float Fuzzmeup1AudioProcessor::fuzzExp1(float x, float k)
{
    float out;
    int sign;
    
    sign = (x < 0) ? -1 : 1;
    out = sign * (1 - exp( -1 * fabs(x * k))) / (1 - exp(-1 * k));
                  
    return out;
}

//==============================================================================
bool Fuzzmeup1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Fuzzmeup1AudioProcessor::createEditor()
{
    //return new Fuzzmeup1AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void Fuzzmeup1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Fuzzmeup1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.drive = apvts.getRawParameterValue("Drive")->load();
    settings.color = apvts.getRawParameterValue("Color")->load();
    settings.trim = apvts.getRawParameterValue("Trim")->load();
    settings.distType = apvts.getRawParameterValue("Distortion Type")->load();
    
    return settings;
}

void Fuzzmeup1AudioProcessor::setShelfCoeff (float cutoff, float q, float gain)
{
    auto shelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), cutoff, q, gain);
    
    updateCoefficients(leftChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
}

void Fuzzmeup1AudioProcessor::updateColor (const ChainSettings& chainSettings)
{
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.color, 0.2f, 1.5f);
    
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void Fuzzmeup1AudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
}

void Fuzzmeup1AudioProcessor::updateDrive(const ChainSettings& chainSettings)
{
    leftChain.get<ChainPositions::Drive>().setGainLinear(chainSettings.drive);
    rightChain.get<ChainPositions::Drive>().setGainLinear(chainSettings.drive);
}

void Fuzzmeup1AudioProcessor::setFunctionToUse()
{
    leftChain.get<ChainPositions::Distortion>().functionToUse = [] (float x)
    {
        return ((x < 0) ? -1 : 1) * (1 - exp(-1 * fabs(x)));
    };
    
    rightChain.get<ChainPositions::Distortion>().functionToUse = [] (float x)
    {
        return ((x < 0) ? -1 : 1) * (1 - exp(-1 * fabs(x)));
    };
}

void Fuzzmeup1AudioProcessor::updateDriveComp(const ChainSettings& chainSettings)
{
    leftChain.get<ChainPositions::DriveComp>().setGainLinear(1.f / (1.f - exp(-1.f * chainSettings.drive)));
    rightChain.get<ChainPositions::DriveComp>().setGainLinear(1.f / (1.f - exp(-1.f * chainSettings.drive)));
}

void Fuzzmeup1AudioProcessor::updateTrim(const ChainSettings& chainSettings)
{
    leftChain.get<ChainPositions::Trim>().setGainDecibels(chainSettings.trim);
    rightChain.get<ChainPositions::Trim>().setGainDecibels(chainSettings.trim);
}

juce::AudioProcessorValueTreeState::ParameterLayout
    Fuzzmeup1AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Drive",
                                                           "Drive",
                                                           juce::NormalisableRange<float>(0.5f, 30.f, 0.1f, 1.f),
                                                           0.5f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Color",
                                                           "Color",
                                                           juce::NormalisableRange<float>(100.f, 18000.f, 1.f, 0.25f),
                                                           100.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Trim",
                                                           "Trim",
                                                           juce::NormalisableRange<float>(-24.f, 6.f, 0.1f, 1.0f),
                                                           0.f));
    
    juce::StringArray stringArray;
    stringArray.add("F");
    stringArray.add("B");
    stringArray.add("T");
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("Distortion Type", "Distortion Type", stringArray, 0));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Fuzzmeup1AudioProcessor();
}
