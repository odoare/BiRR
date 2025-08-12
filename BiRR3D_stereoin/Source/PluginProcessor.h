/*
  ==============================================================================

    Binaural Room Reverb 3D - PluginProcessor.h

    (c) Olivier Doaré, 2022-2025

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../lib/dsp/RoomIR_WDL.h"

//==============================================================================
/**
*/
class ReverbAudioProcessor  : public juce::AudioProcessor, public juce::Timer
{
public:
    //==============================================================================
    ReverbAudioProcessor();
    ~ReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

  //  #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
  //  #endif

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

    void setIrLoaderL(), setIrLoaderR();
    bool autoUpdate{true};

    BoxRoomIR roomIRL, roomIRR;

    juce::dsp::ProcessSpec spec;
    juce::AudioBuffer<float> bufferCopyR;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();  
    juce::AudioProcessorValueTreeState apvts{*this,nullptr,"Parameters",createParameters()};

private:

    void timerCallback() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioProcessor)
};
