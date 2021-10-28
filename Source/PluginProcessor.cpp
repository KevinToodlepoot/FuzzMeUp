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
    
    setSecondFunctionToUse();
    
    updateAll();
    
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
    
    updateAll();
    
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
    return new Fuzzmeup1AudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void Fuzzmeup1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void Fuzzmeup1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if ( tree.isValid() )
    {
        apvts.replaceState(tree);
        updateAll();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.drive = apvts.getRawParameterValue("Drive")->load();
    settings.color = apvts.getRawParameterValue("Color")->load();
    settings.trim = apvts.getRawParameterValue("Trim")->load();
    settings.fButtonState = apvts.getRawParameterValue("F Button")->load();
    settings.mButtonState = apvts.getRawParameterValue("M Button")->load();
    settings.uButtonState = apvts.getRawParameterValue("U Button")->load();
    
    return settings;
}

void Fuzzmeup1AudioProcessor::updateShelfCoefficients (const ChainSettings& chainSettings)
{
    
    auto shelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 20.f, 0.5f, 2.f);
    
    if (chainSettings.fButtonState)
        shelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 20.f, 0.5f, 2.f);
    else if (chainSettings.mButtonState)
        shelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 20.f, 0.5f, 0.5f);
    else if (chainSettings.uButtonState)
        shelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 20.f, 0.5f, 1.f);
    else
        jassert("no buttons are on");
    
    updateCoefficients(leftChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
}

void Fuzzmeup1AudioProcessor::updateColor (const ChainSettings& chainSettings)
{
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.color, 0.33f, 2.f);
    
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void Fuzzmeup1AudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
}

void Fuzzmeup1AudioProcessor::updateBias(const ChainSettings &chainSettings)
{
    if (chainSettings.uButtonState)
    {
        leftChain.get<ChainPositions::BiasPos>().setBias(1.f);
        rightChain.get<ChainPositions::BiasPos>().setBias(1.f);
    }
    else
    {
        leftChain.get<ChainPositions::BiasPos>().setBias(0.f);
        rightChain.get<ChainPositions::BiasPos>().setBias(0.f);
    }
}

void Fuzzmeup1AudioProcessor::updateDrive(const ChainSettings& chainSettings)
{
    if (chainSettings.mButtonState)
    {
        leftChain.get<ChainPositions::Drive>().setGainLinear(chainSettings.drive * 10.f);
        rightChain.get<ChainPositions::Drive>().setGainLinear(chainSettings.drive * 10.f);
    }
    else
    {
        leftChain.get<ChainPositions::Drive>().setGainLinear(chainSettings.drive * 5.f);
        rightChain.get<ChainPositions::Drive>().setGainLinear(chainSettings.drive * 5.f);
    }
}

void Fuzzmeup1AudioProcessor::setFirstFunctionToUse(bool onOff)
{
    if (onOff)
    {
        leftChain.get<ChainPositions::Distortion1>().functionToUse = [] (float x)
        {
            return ((-x < 0) ? -1.f : 1.f) * (1.f - exp(fabs(x))) / (juce::MathConstants<float>::euler - 1.f);
        };
        
        rightChain.get<ChainPositions::Distortion1>().functionToUse = [] (float x)
        {
            return ((-x < 0) ? -1.f : 1.f) * (1.f - exp(fabs(x))) / (juce::MathConstants<float>::euler - 1.f);
        };
    }
    else
    {
        leftChain.get<ChainPositions::Distortion1>().functionToUse = [] (float x)
        {
            return x;
        };
        
        rightChain.get<ChainPositions::Distortion1>().functionToUse = [] (float x)
        {
            return x;
        };
    }
}

void Fuzzmeup1AudioProcessor::setSecondFunctionToUse()
{
    leftChain.get<ChainPositions::Distortion2>().functionToUse = [] (float x)
    {
        return ((x < 0) ? -1.f : 1.f) * (1.f - exp(-1.f * fabs(x)));
    };
    
    rightChain.get<ChainPositions::Distortion2>().functionToUse = [] (float x)
    {
        return ((x < 0) ? -1.f : 1.f) * (1.f - exp(-1.f * fabs(x)));
    };
}

void Fuzzmeup1AudioProcessor::updateDriveComp(const ChainSettings& chainSettings)
{
    if (chainSettings.mButtonState)
    {
        leftChain.get<ChainPositions::DriveComp>().setGainLinear(1.f / (1.f - exp(-1.f * (chainSettings.drive * 10.f))));
        rightChain.get<ChainPositions::DriveComp>().setGainLinear(1.f / (1.f - exp(-1.f * (chainSettings.drive * 10.f))));
    }
    else
    {
        leftChain.get<ChainPositions::DriveComp>().setGainLinear(1.f / (1.f - exp(-1.f * (chainSettings.drive * 5.f))));
        rightChain.get<ChainPositions::DriveComp>().setGainLinear(1.f / (1.f - exp(-1.f * (chainSettings.drive * 5.f))));
    }
}

void Fuzzmeup1AudioProcessor::updateDCBlock(const ChainSettings& chainSettings)
{
    auto hpfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), 18.f);
    
    updateCoefficients(leftChain.get<ChainPositions::dcBlock1>().coefficients, hpfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::dcBlock1>().coefficients, hpfCoefficients);
    updateCoefficients(leftChain.get<ChainPositions::dcBlock2>().coefficients, hpfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::dcBlock2>().coefficients, hpfCoefficients);
    updateCoefficients(leftChain.get<ChainPositions::dcBlock3>().coefficients, hpfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::dcBlock3>().coefficients, hpfCoefficients);
    updateCoefficients(leftChain.get<ChainPositions::dcBlock4>().coefficients, hpfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::dcBlock4>().coefficients, hpfCoefficients);
}

void Fuzzmeup1AudioProcessor::updateTrim(const ChainSettings& chainSettings)
{
    leftChain.get<ChainPositions::Trim>().setGainDecibels(chainSettings.trim);
    rightChain.get<ChainPositions::Trim>().setGainDecibels(chainSettings.trim);
}

void Fuzzmeup1AudioProcessor::updateAll()
{
    auto chainSettings = getChainSettings(apvts);
    
    updateShelfCoefficients(chainSettings);
    
    updateColor(chainSettings);
    
    updateBias(chainSettings);

    updateDrive(chainSettings);
    
    setFirstFunctionToUse(chainSettings.mButtonState);
    
    updateDriveComp(chainSettings);
    
    updateDCBlock(chainSettings);
    
    updateTrim(chainSettings);
}

juce::AudioProcessorValueTreeState::ParameterLayout
    Fuzzmeup1AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Drive",
                                                           "Drive",
                                                           juce::NormalisableRange<float>(0.04f, 10.f, 0.01f, 1.f),
                                                           0.5f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Color",
                                                           "Color",
                                                           juce::NormalisableRange<float>(20.f, 10000.f, 1.f, 0.25f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Trim",
                                                           "Trim",
                                                           juce::NormalisableRange<float>(-24.f, 6.f, 0.1f, 1.0f),
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("F Button", "F Button", true));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("M Button", "M Button", false));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("U Button", "U Button", false));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Fuzzmeup1AudioProcessor();
}
