#pragma once

#include <JuceHeader.h>

#include <iostream>
using namespace std;

#define CHOICES {"XY", "MS with Cardio", "MS with Omni", "Binaural"}

#define NPROC 6

#define INV_SOUNDSPEED 2.9412e-03f
#define PIOVEREIGHTY 1.745329252e-02f
#define EIGHTYOVERPI 57.295779513f
#define OMEGASTART 125663.706f
#define SIGMA_DELTAT 1e-3f


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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IrCalculator)
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IrTransfer)
};

// ====================================================
class RoomIR{

public:
    RoomIR();
    void prepare(juce::dsp::ProcessSpec spec);
    void calculate(IrCalculator::IrCalculatorParams& p);
    bool setIrCaclulatorsParams(IrCalculator::IrCalculatorParams& pa);
    float getProgress();
    bool getCalculatingState();
    bool getBufferTransferState();
    void process(juce::AudioBuffer<float>& buffer);

    juce::dsp::Convolution convolution;
    IrCalculator calculator[NPROC];
    bool isCalculating[NPROC];
    juce::AudioBuffer<float> irBuffer[NPROC];
    IrTransfer irTransfer;

private:
    IrCalculator::IrCalculatorParams p;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomIR)

};
