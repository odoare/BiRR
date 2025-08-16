/*
  ==============================================================================

    Binaural Room Reverb 3D - PluginProcessor.cpp

    (c) Olivier Doaré, 2022-2025

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define DEBUG_OUTPUTS

#ifdef DEBUG_OUTPUTS
#include <iostream>
#include <chrono>
using namespace std;
#endif

//==============================================================================
ReverbAudioProcessor::ReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::quadraphonic(), true)
                     #endif
                       )
#endif
{
    startTimerHz(5);
}

ReverbAudioProcessor::~ReverbAudioProcessor()
{
}

//==============================================================================
const juce::String ReverbAudioProcessor::getName() const
{
    return "Binaural Room Reverb Ambisonic 2ch in/4ch out";
}

bool ReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    roomIRL.initialize();
    roomIRR.initialize();

    roomIRL.prepare(spec);
    roomIRR.prepare(spec);

    std::cout << "Has prepared." << std::endl;
}

void ReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

// #ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{

    // Ici faire du 2 in 4 out
    
    if ((layouts.getMainOutputChannelSet() == juce::AudioChannelSet::quadraphonic())
        && (layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()))
        return true;
    else
        return false;

}
// #endif

void ReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // std::cout << "In process Block \n";
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // std::cout << "Num of input channels : " << totalNumInputChannels << std::endl;
    // std::cout << "Num of output channels : " << totalNumOutputChannels << std::endl;
    // std::cout << "Num of channels in audiobuffer : " << buffer.getNumChannels() << std::endl;

    // std::cout << "Start buffer copy \n";
    bufferCopyR.makeCopyOf(buffer,true);
    bufferCopyR.copyFrom(0,0,bufferCopyR,1,0,bufferCopyR.getNumSamples());
    bufferCopyR.copyFrom(2,0,bufferCopyR,1,0,bufferCopyR.getNumSamples());
    bufferCopyR.copyFrom(3,0,bufferCopyR,1,0,bufferCopyR.getNumSamples());
    buffer.copyFrom(1,0,buffer,0,0,buffer.getNumSamples());
    buffer.copyFrom(2,0,buffer,0,0,buffer.getNumSamples());
    buffer.copyFrom(3,0,buffer,0,0,buffer.getNumSamples());
    // std::cout << "Buffer copy done\n";

    // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    // {
    //     //buffer.clear (i, 0, buffer.getNumSamples());
    // }

    // std::cout << "Get parameters in process \n";

    roomIRL.directLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Direct Level")->load());
    roomIRL.reflectionsLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Reflections Level")->load());
    roomIRR.directLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Direct Level")->load());
    roomIRR.reflectionsLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Reflections Level")->load());

    // std::cout << "Start process L \n";
    roomIRL.process(buffer);
    // std::cout << "Start process R \n";
    roomIRR.process(bufferCopyR);
    // std::cout << "Processed\n";

    buffer.addFrom(0,0,bufferCopyR,0,0,buffer.getNumSamples(),1.0f);
    buffer.addFrom(1,0,bufferCopyR,1,0,buffer.getNumSamples(),1.0f);
    buffer.addFrom(2,0,bufferCopyR,2,0,buffer.getNumSamples(),1.0f);
    buffer.addFrom(3,0,bufferCopyR,3,0,buffer.getNumSamples(),1.0f);

    // std::cout << "End of process Block \n";

}

//==============================================================================
bool ReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReverbAudioProcessor::createEditor()
{
    return new ReverbAudioProcessorEditor (*this);
}

//==============================================================================
void ReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void ReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data,sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout ReverbAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("Room Size X","Room Size X",1.0f,MAXSIZE,3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Room Size Y","Room Size Y",1.0f,MAXSIZE,4.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Room Size Z","Room Size Z",1.0f,MAXSIZE,2.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerX","ListenerX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerY","ListenerY",0.01f,0.99f,0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerZ","ListenerZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerO","ListenerO",juce::NormalisableRange<float>(-180.f,180.f,1.f,1.f),0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceLX","SourceLX",0.01f,0.99f,0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceLY","SourceLY",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceLZ","SourceLZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceRX","SourceRX",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceRY","SourceRY",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceRZ","SourceRZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Damping","Damping",juce::NormalisableRange<float>(MINDAMPING,0.99f,0.001f,0.3f),0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HF Damping","HF Damping",juce::NormalisableRange<float>(0.01f,0.3f,0.001f,0.3f),0.05f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Diffusion","Diffusion",juce::NormalisableRange<float>(0.f,1e-2f,1e-5f,0.5f),1e-3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Direct Level","Direct Level",juce::NormalisableRange<float>(-90.0f,6.f,0.1f,1.f),0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Reflections Level","Reflections Level",juce::NormalisableRange<float>(-90.0f,6.f,0.1f,1.f),0.f));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("Update","Update", true));

    return layout;
}

// This is the function where the impulse response is calculated
void ReverbAudioProcessor::setIrLoaderL()
{
    // std::cout << "In setIrLoader L" << endl;

    IrBoxCalculatorParams p;

    // std::cout << "Set parameters" << endl;

    p.rx = apvts.getRawParameterValue("Room Size X")->load();
    p.ry = apvts.getRawParameterValue("Room Size Y")->load();
    p.rz = apvts.getRawParameterValue("Room Size Z")->load();
    p.lx = p.rx*(apvts.getRawParameterValue("ListenerX")->load());
    p.ly = p.ry*(apvts.getRawParameterValue("ListenerY")->load());
    p.lz = p.rz*(apvts.getRawParameterValue("ListenerZ")->load());
    p.sx = p.rx*(apvts.getRawParameterValue("SourceLX")->load());
    p.sy = p.ry*(apvts.getRawParameterValue("SourceLY")->load());
    p.sz = p.rz*(apvts.getRawParameterValue("SourceLZ")->load());
    p.damp = apvts.getRawParameterValue("Damping")->load();
    p.hfDamp = apvts.getRawParameterValue("HF Damping")->load();
    p.headAzim = apvts.getRawParameterValue("ListenerO")->load();
    p.diffusion = apvts.getRawParameterValue("Diffusion")->load();
    p.dimension = 13;
    p.sampleRate = spec.sampleRate;

    // std::cout << "Start roomIRL.calculate in setIrLoaderL" << endl;    
    if (roomIRL.hasInitialized) roomIRL.calculate(p);
    // std::cout << "Finished" << endl;
}

// This is the function where the impulse response is calculated
void ReverbAudioProcessor::setIrLoaderR()
{
    // std::cout << "In setIrLoader R" << endl;

    IrBoxCalculatorParams p;

    // std::cout << "Set parameters" << endl;

    p.rx = apvts.getRawParameterValue("Room Size X")->load();
    p.ry = apvts.getRawParameterValue("Room Size Y")->load();
    p.rz = apvts.getRawParameterValue("Room Size Z")->load();
    p.lx = p.rx*(apvts.getRawParameterValue("ListenerX")->load());
    p.ly = p.ry*(apvts.getRawParameterValue("ListenerY")->load());
    p.lz = p.rz*(apvts.getRawParameterValue("ListenerZ")->load());
    p.sx = p.rx*(apvts.getRawParameterValue("SourceRX")->load());
    p.sy = p.ry*(apvts.getRawParameterValue("SourceRY")->load());
    p.sz = p.rz*(apvts.getRawParameterValue("SourceRZ")->load());
    p.damp = apvts.getRawParameterValue("Damping")->load();
    p.hfDamp = apvts.getRawParameterValue("HF Damping")->load();
    p.headAzim = apvts.getRawParameterValue("ListenerO")->load();
    p.diffusion = apvts.getRawParameterValue("Diffusion")->load();
    p.dimension = 13;
    p.sampleRate = spec.sampleRate;

    // std::cout << "Start roomIRR.calculate in setIrLoaderR" << endl;
    if (roomIRR.hasInitialized) roomIRR.calculate(p);
    // std::cout << "Finished" << endl;
}

void ReverbAudioProcessor::timerCallback()
{
    if (autoUpdate)
    {
        setIrLoaderL();
        setIrLoaderR();
    }
}
