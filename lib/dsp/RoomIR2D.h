#pragma once

#include <JuceHeader.h>


#include <iostream>
using namespace std;

#define CHOICES {"XY", "MS with Cardio", "MS with Omni", "Binaural"}

#define NPROC 6
#define MAXTHREADS 12

#define INV_SOUNDSPEED 2.9412e-03f
#define PIOVEREIGHTY 1.745329252e-02f
#define EIGHTYOVERPI 57.295779513f
#define OMEGASTART 125663.706f
#define SIGMA_DELTAT 1e-3f

#define MAXSIZE 10.f
#define MINDAMPING 0.005f

struct IrBoxCalculatorParams{
  float rx;
  float ry;
  float lx;
  float ly;
  float sx;
  float sy;
  float damp;
  float hfDamp;
  int type;
  float headAzim;
  float sWidth;
  double sampleRate;
};

// ==================================================================
class IrBoxCalculator : public juce::Thread
  {

  public:

    IrBoxCalculator();
    void run() override ;
    void setParams(IrBoxCalculatorParams& pa);
    float getProgress();
    void resetProgress();
    void setCalculatingBool(bool* cp);
    void setBuffer(juce::AudioBuffer<float>* b);
    void setCalculateDirectPath(bool c);
    void setHrtfVars(int* ns, float* nsr);

    // min and max indices which iR is calculated in this thread
    // If only the direct sound is needed, one has to select
    // n=1, nxmin=0, nxmax=1
    int n, nxmin, nxmax;
    int longueur;
    
  private:
    IrBoxCalculatorParams p;
    float progress;
    bool* isCalculating;
    juce::AudioBuffer<float>* bp;
    bool calculateDirectPath;
    int* nsamp;
    float* nearestSampleRate;
    // int threadsNum;
    
    void addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain);
    int proximityIndex(const float *data, const int length, const float value, const bool wrap);
    void lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order);
    float max(const float* in);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IrBoxCalculator)
  };

// ==================================================================
class IrTransfer : public juce::Thread
{

public:
    IrTransfer();
    void run() override ;
    void setBuffer(juce::AudioBuffer<float>* bufPointer);
    void setIr(juce::dsp::Convolution* irPointer);
    void setCalculatingBool(bool* cp);
    void setSampleRate(double sr);
    double getSampleRate();    
    bool getBufferTransferState();
    void setThreadsNum(int n);

private:
    juce::AudioBuffer<float> tempBuf;
    juce::AudioBuffer<float>* bp;
    juce::dsp::Convolution* irp;
    bool* isCalculating;
    bool hasTransferred;
    double sampleRate;
    int threadsNum;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IrTransfer)
};

// ====================================================
class BoxRoomIR{

public:
    BoxRoomIR();
    void initialize();
    void prepare(juce::dsp::ProcessSpec spec);
    void calculate(IrBoxCalculatorParams& p);
    bool setIrCaclulatorsParams(IrBoxCalculatorParams& pa);
    float getProgress();
    bool getCalculatingState();
    bool getBufferTransferState();
    void process(juce::AudioBuffer<float>& buffer);
    void exportIrToWav(juce::File file);

    juce::AudioBuffer<float> inputBufferCopy;
    juce::dsp::Convolution boxConvolution, directConvolution;
    IrBoxCalculator boxCalculator[MAXTHREADS], directCalculator;
    bool isCalculating[MAXTHREADS], isCalculatingDirect;
    juce::AudioBuffer<float> boxIrBuffer[MAXTHREADS], directIrBuffer;
    IrTransfer boxIrTransfer, directIrTransfer;
    float directLevel, reflectionsLevel;
    bool hasInitialized{false};

private:
    IrBoxCalculatorParams p;
    int threadsNum;
    int nsamp;
    float nearestSampleRate;


    juce::dsp::IIR::Filter<float> filter[2];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoxRoomIR)

};
