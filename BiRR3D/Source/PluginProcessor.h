/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Squeeze.h"

#define CHOICES {"XY", "MS with Cardio", "MS with Omni", "Binaural"}

#define NPROC 6



// ==================================================================
class IrCalculator : public juce::Thread
  {

  public:

    struct IrCalculatorParams{
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

    IrCalculator();
    void run() override ;
    void setParams(IrCalculatorParams& pa);
    float getProgress();
    void resetProgress();
    void setCalculatingBool(bool* cp);
    void setBuffer(juce::AudioBuffer<float>* b);

    // min an max indices which iR is calculated in this thread
    int n, nxmin, nxmax;
    
  private:
    IrCalculatorParams p;
    float progress;
    bool* isCalculating;
    juce::AudioBuffer<float>* bp;
    
    void addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain);
    int proximityIndex(const float *data, const int length, const float value, const bool wrap);
    void lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order);
    float max(const float* in);
  };

// ==================================================================
class IrTransfer : public juce::Thread
{

public:
    IrTransfer();
    void run() override ;
    void setBuffer(juce::AudioBuffer<float>* bufPointer);
    void setIr(juce::dsp::Convolution* irPointer);
    void setCalculatingBool(bool* ic);
    void setSampleRate(double sr);
    bool getBufferTransferState();

private:
    juce::AudioBuffer<float>* bp;
    juce::dsp::Convolution* irp;
    bool* isCalculating;
    bool hasTransferred;
    double sampleRate;
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

    bool setIrCaclulatorsParams(IrCalculator::IrCalculatorParams& pa);

    float getProgress();
    bool getCalculatingState();
    bool getBufferTransferState();

    juce::dsp::Convolution irLoader;
    IrCalculator calculator[NPROC];
    bool isCalculating[NPROC];
    IrTransfer irTransfer;
    juce::AudioBuffer<float> irBuffer[NPROC];

    juce::dsp::ProcessSpec spec;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();  
    juce::AudioProcessorValueTreeState apvts{*this,nullptr,"Parameters",createParameters()};

private:

    IrCalculator::IrCalculatorParams p;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioProcessor)
};

