/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BinauRoomRevAudioProcessor::BinauRoomRevAudioProcessor()
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

BinauRoomRevAudioProcessor::~BinauRoomRevAudioProcessor()
{
}

//==============================================================================
const juce::String BinauRoomRevAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BinauRoomRevAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BinauRoomRevAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BinauRoomRevAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BinauRoomRevAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BinauRoomRevAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BinauRoomRevAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BinauRoomRevAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BinauRoomRevAudioProcessor::getProgramName (int index)
{
    return {};
}

void BinauRoomRevAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BinauRoomRevAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    irLoader.reset();
    irLoader.prepare(spec);
}

void BinauRoomRevAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BinauRoomRevAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BinauRoomRevAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    juce::dsp::AudioBlock<float> block {buffer};
    if (irLoader.getCurrentIRSize()>0)
    {
        irLoader.process(juce::dsp::ProcessContextReplacing<float>(block));
    }
}

//==============================================================================
bool BinauRoomRevAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BinauRoomRevAudioProcessor::createEditor()
{
    return new BinauRoomRevAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BinauRoomRevAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BinauRoomRevAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinauRoomRevAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout BinauRoomRevAudioProcessor::createParameters()
{
    // std::vector<std::unique_ptr<juce::RangedAudioParameter>> params ;
    // params.push_back (std::make_unique<juce::AudioParameterFloat>("Gain","Gain",0.0f,1.0f,0.5f));
    // params.push_back (std::make_unique<juce::AudioParameterBool>("Phase","Phase",false));

    // return {params.begin(),params.end()};

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomX","RoomX",1.0f,10.0f,3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomY","RoomY",1.0f,10.0f,3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerX","ListenerX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerY","ListenerY",0.01f,0.99f,0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceX","SourceX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceY","SourceY",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterInt>("N","N",10,100,20));
    layout.add(std::make_unique<juce::AudioParameterFloat>("D","D",0.0f,1.0f,0.2f));

    return layout;

}
