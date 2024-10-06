/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

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
                       .withInput  ("Input",  juce::AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ReverbAudioProcessor::~ReverbAudioProcessor()
{
}

//==============================================================================
const juce::String ReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
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

    roomIR.prepare(spec);


    // We start loading of IRs
    // This is necessary when the plugin is newly loaded
    setIrLoader();

    // In case of project loading, if the host loads the parameters
    // too fast, the system can crash. A temporary dirty workaround
    // is to wait a little bit before going the next steps.
    // TODO : find a better solution
    juce::Time::waitForMillisecondCounter(2000);
}

void ReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

// #ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono())
        return true;
    else
        return false;

}
// #endif

void ReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.copyFrom(1,0,buffer,0,0,buffer.getNumSamples());

    roomIR.directLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Direct Level")->load());
    roomIR.reflectionsLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Reflections Level")->load());

    roomIR.process(buffer);
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

    // Call the impulse response calculator and loader
    setIrLoader();
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
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomX","RoomX",1.0f,MAXSIZE,3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomY","RoomY",1.0f,MAXSIZE,4.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomZ","RoomZ",1.0f,MAXSIZE,2.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerX","ListenerX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerY","ListenerY",0.01f,0.99f,0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerZ","ListenerZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerO","ListenerO",juce::NormalisableRange<float>(-180.f,180.f,1.f,1.f),0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceX","SourceX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceY","SourceY",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceZ","SourceZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("D","D",juce::NormalisableRange<float>(MINDAMPING,0.99f,0.001f,0.3f),0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HFD","HFD",juce::NormalisableRange<float>(0.01f,0.3f,0.001f,0.3f),0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Stereo Width","Stereo Width",juce::NormalisableRange<float>(0.0f,1.f,0.001f,1.f),0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Direct Level","Direct Level",juce::NormalisableRange<float>(-90.0f,6.f,0.1f,1.f),0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Reflections Level","Reflections Level",juce::NormalisableRange<float>(-90.0f,6.f,0.1f,1.f),0.f));
    
    juce::StringArray choices;
    choices.addArray(CHOICES);
    layout.add(std::make_unique<juce::AudioParameterChoice>("Reverb type", "Reverb type", choices, 1));
    layout.add(std::make_unique<juce::AudioParameterBool>("Auto update","Auto update", true));

    return layout;
}

// This is the function where the impulse response is calculated
void ReverbAudioProcessor::setIrLoader()
{
    // std::cout << "In setIrLoader" << endl;

    IrBoxCalculatorParams p;

    // std::cout << "Set parameters" << endl;

    p.rx = apvts.getRawParameterValue("RoomX")->load();
    p.ry = apvts.getRawParameterValue("RoomY")->load();
    p.rz = apvts.getRawParameterValue("RoomZ")->load();
    p.lx = p.rx*(apvts.getRawParameterValue("ListenerX")->load());
    p.ly = p.ry*(apvts.getRawParameterValue("ListenerY")->load());
    p.lz = p.rz*(apvts.getRawParameterValue("ListenerZ")->load());
    p.sx = p.rx*(apvts.getRawParameterValue("SourceX")->load());
    p.sy = p.ry*(apvts.getRawParameterValue("SourceY")->load());
    p.sz = p.rz*(apvts.getRawParameterValue("SourceZ")->load());
    p.damp = apvts.getRawParameterValue("D")->load();
    p.hfDamp = apvts.getRawParameterValue("HFD")->load();
    p.type = apvts.getRawParameterValue("Reverb type")->load();
    p.headAzim = apvts.getRawParameterValue("ListenerO")->load();
    p.sWidth = apvts.getRawParameterValue("Stereo Width")->load();
    p.sampleRate = spec.sampleRate;

    roomIR.calculate(p);
}
