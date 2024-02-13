/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define CHOICES {"XY", "MS with Cardio", "MS with Omni", "Binaural"}

// ==================================================================
class irCalculator : public juce::Thread
  {

  public:

    struct irCalculatorParams{
      float rx;
      float ry;
      float rz;
      float lx;
      float ly;
      float lz;
      float sx;
      float sy;
      float sz;
      float damp;
      float hfDamp;
      int type;
      float headAzim;
      float sWidth;
      float directLevel;
      float reflectionsLevel;
      double sampleRate;
    };

    irCalculator();
    void run() override ;
    bool setParams(irCalculatorParams pa);
    void setConvPointer(juce::dsp::Convolution* ip);
    float getProgress();

    juce::dsp::Convolution* irp;
    
    bool isCalculating;
    bool bufferTransferred;
    juce::AudioBuffer<float> buf;

  private:
    irCalculatorParams p;
    float progress;

    // juce::dsp::Convolution& irr;
    void addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain);
    int proximityIndex(const float *data, const int length, const float value, const bool wrap);
    void lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order);
  };

//==============================================================================
/**
*/
class ReverbAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ReverbAudioProcessor();
    ~ReverbAudioProcessor() override;

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

    void setIrLoader();

    juce::dsp::Convolution irLoader;
    irCalculator calculator;

    juce::dsp::ProcessSpec spec;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();  
    juce::AudioProcessorValueTreeState apvts{*this,nullptr,"Parameters",createParameters()};

private:

    juce::AudioBuffer<float> buf;

    void addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain);
    int proximityIndex(const float *data, const int length, const float value, const bool wrap);
    void lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order);
    // void alp(const float* in, float* out, int sampleFreq, float amount, int nRebounds);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioProcessor)
};
