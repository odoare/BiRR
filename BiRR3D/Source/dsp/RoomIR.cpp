#include "RoomIR.h"
#include "../hrtf.h"

// ======================================================================

// This is the function where the impulse response is calculated
void IrCalculator::run()
{
    isCalculating[0] = true;

    // std::cout << "In irCalculator::run()" << endl;

    // inBuf is the buffer used for the non-binaural method
    float outBuf[NSAMP]={0.f}, inBuf[NSAMP]={0.f};
    inBuf[10] = 1.f;
    float x,y,z;

    // cout << "Start buffer fill..." << endl;

    bp[0].clear();
    auto* dataL = bp[0].getWritePointer(0);
    auto* dataR = bp[0].getWritePointer(1);
    
    // cout << "Start Loop ... " << endl;

    for (int ix = nxmin; ix < nxmax ; ++ix)
    {
      x = 2*float(ceil(float(ix)/2))*p.rx+pow(-1,ix)*p.sx;
      if (!threadShouldExit())
      {
        for (int iy = -n+1; iy < n ; ++iy)
        {
          y = 2*float(ceil(float(iy)/2))*p.ry+pow(-1,iy)*p.sy;
          for (int iz=-n+1; iz<n; ++iz)
          {
            z = 2*float(ceil(float(iz)/2))*p.rz+pow(-1,iz)*p.sz;
            float dist = sqrt((x-p.lx)*(x-p.lx)+(y-p.ly)*(y-p.ly)+(z-p.lz)*(z-p.lz));
            float time = dist*INV_SOUNDSPEED;

            int indice = int(round((time+juce::Random::getSystemRandom().nextFloat()*SIGMA_DELTAT)*p.sampleRate));
            float r = pow(1-p.damp,abs(ix)+abs(iy)+abs(iz));
            // float gain = pow(-1,ix+iy+iz)*r/dist;
            float gain = (r/dist) * ( (ix==0 && iy==0 && iz==0) ? p.directLevel : p.reflectionsLevel );
            
            float rp = sqrt((p.sx-p.lx)*(p.sx-p.lx)+(p.sy-p.ly)*(p.sy-p.ly));
            float elev = atan2f(p.sz-p.lz,rp)*EIGHTYOVERPI;

            // Azimutal angle calculation
            float theta = atan2f(y-p.ly,-x+p.lx)*EIGHTYOVERPI-90-p.headAzim;
            
            // XY
            if (p.type==0){
              // Apply lowpass filter and add grain to 
              auto elevCardio = (1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(elev)));
              auto panGain = 0.25*(1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(theta+45*p.sWidth)))
                              * elevCardio;
              lop(&inBuf[0], &outBuf[0], p.sampleRate,p.hfDamp,abs(ix)+abs(iy),1);
              addArrayToBuffer(&dataL[indice], &outBuf[0], gain*panGain);
              panGain = 0.25*(1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(theta-45*p.sWidth)))
                          * elevCardio;
              lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              addArrayToBuffer(&dataR[indice], &outBuf[0], gain*panGain);
            }

            // MS with cardio mic for mid channel
            if (p.type==1){
              // Apply lowpass filter and add grain to buffer
              lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              auto gainMid = 0.25*(1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(theta)))
                              * (1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(elev)));
              auto gainSide = juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(elev))
                              * juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*(theta));

              addArrayToBuffer(&dataL[indice], &outBuf[0], gain*(gainMid-gainSide*p.sWidth));
              addArrayToBuffer(&dataR[indice], &outBuf[0], gain*(gainMid+gainSide*p.sWidth));
            }

            // MS with omni mic for mid channel
            if (p.type==2){
              // Apply lowpass filter and add grain to buffer
              lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              auto gainMid = 1.f;
              auto gainSide = juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(elev))
                              * juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*(theta));

              addArrayToBuffer(&dataL[indice], &outBuf[0], gain*(gainMid-gainSide*p.sWidth));
              addArrayToBuffer(&dataR[indice], &outBuf[0], gain*(gainMid+gainSide*p.sWidth));
            }

            // Binaural
            if (p.type==3){      
              int elevationIndex = proximityIndex(&elevations[0],NELEV,elev,false);
              int azimutalIndex = proximityIndex(&azimuths[elevationIndex][0],NAZIM,theta,true);
              // Apply lowpass filter and add grain to buffer
              lop(&lhrtfn[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
              lop(&rhrtfn[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
            }
          }
        }
        progress = float(ix-nxmin)/float(nxmax-nxmin);
      }
      else return;
    }
    isCalculating[0] = false;
    // cout << "End Loop ... " << endl;
}

// Add a given array to a buffer
void IrCalculator::addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;
  }
}

// Compares the values in data to a float prameter value and returns the nearest index
int IrCalculator::proximityIndex(const float *data, const int length, const float value, const bool wrap)
{
  int proxIndex = 0;
  float minDistance = BIGVALUE;
  float val;
  if (wrap && value<0.f)
  {
    val = value+360.f;
  }
  else
  {
    val = value;
  }
  for (int i=0; i<length; i++)
  {
    float actualDistance = abs(data[i]-val);
    if (actualDistance < minDistance)
    {
      proxIndex = i;
      minDistance = actualDistance;
    }
  }
  return proxIndex;
}

// Basic lowpass filter
void IrCalculator::lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order)
{
    const float om = OMEGASTART*(exp(-hfDamping*nRebounds));
    const float alpha1 = exp(-om/sampleFreq);
    const float alpha = 1 - alpha1;
    out[0] = alpha*in[0];
    for (int i=1;i<NSAMP;i++)
    {
      out[i] = alpha*in[i] + alpha1*out[i-1];
    }
    for (int j=0; j<order-1; j++)
    {
      out[0] *= alpha;
      for (int i=1;i<NSAMP;i++)
      {
        out[i] = alpha*out[i] + alpha1*out[i-1];
      }
    }
}

// Get max
float IrCalculator::max(const float* in)
{
  float max = 0;
  for (int i=1;i<NSAMP;i++)
  {
    if (in[i]>max)
    max = in[i];
  }
  return max;
}

void IrCalculator::setParams(IrCalculatorParams& pa)
{
  p = pa ;
}

float IrCalculator::getProgress()
{
  return progress;
}

void IrCalculator::resetProgress()
{
  progress = 0.f;
}

void IrCalculator::setCalculatingBool(bool* cp)
{
  isCalculating = cp;
}

void IrCalculator::setBuffer(juce::AudioBuffer<float>* b)
{
  bp = b;
}

IrCalculator::IrCalculator() : juce::Thread("test")
{

}


// ===============================================================
// ===============================================================
IrTransfer::IrTransfer() : juce::Thread("transfer")
{

}

void IrTransfer::run()
{
  // cout << "In IrTransfer::run()" << endl;
  hasTransferred = false;
  // First we must wait for the buffers to be ready
  bool isCalc = true;
  while (isCalc)
  {
    isCalc = false;
    for (int i=0;i<NPROC;i++)
      {
        isCalc = isCalc || isCalculating[i];
        //cout << i << " : " << isCalculating[i] << "    " << endl;
      }
    //out << "isCalc : " << isCalc << endl;
    //cout << "Waiting for buffer fill" << endl;
    sleep(200);
    continue;
  }

  // std::cout << "Start buffer filling" << endl; 

  // std::cout << "Join..." << endl; 

  for (int i=1;i<NPROC;i++)
    {
      bp[0].addFrom(0,0,bp[i],0,0,bp[i].getNumSamples());
      bp[0].addFrom(1,0,bp[i],1,0,bp[i].getNumSamples());
    }

  // std::cout << "Convolution loader..." << endl; 
  irp->loadImpulseResponse(std::move (bp[0]),
                      sampleRate,
                      juce::dsp::Convolution::Stereo::yes,
                      juce::dsp::Convolution::Trim::no,
                      juce::dsp::Convolution::Normalise::no);

  hasTransferred = true;

  // std::cout << "Finished buffer filling" << endl; 
}

void IrTransfer::setBuffer(juce::AudioBuffer<float>* bufPointer)
{
  bp = bufPointer;

}
void IrTransfer::setIr(juce::dsp::Convolution* irPointer)
{
  irp = irPointer;
}

void IrTransfer::setCalculatingBool(bool* ic)
{
  isCalculating = ic;
}

void IrTransfer::setSampleRate(double sr)
{
  sampleRate = sr;
}

bool IrTransfer::getBufferTransferState()
{
  return hasTransferred;
}


// ========================================================
// ========================================================
// This is the function where the impulse response is calculated

RoomIR::RoomIR()
{
}

void RoomIR::prepare(juce::dsp::ProcessSpec spec)
{

    cout << "Number of CPUs : " << juce::SystemStats::getNumCpus() << endl;
    cout << "Number of physical CPUs : " << juce::SystemStats::getNumPhysicalCpus() << endl;

    for (int i=0; i<NPROC; i++)
    {
      calculator[i].setCalculatingBool(&isCalculating[i]);
      calculator[i].setBuffer(&irBuffer[i]);
    }
    irTransfer.setCalculatingBool(&isCalculating[0]);
    irTransfer.setBuffer(&irBuffer[0]);
    irTransfer.setIr(&convolution);
    irTransfer.setSampleRate(spec.sampleRate);

    convolution.reset();
    convolution.prepare(spec);

}
void RoomIR::calculate(IrCalculator::IrCalculatorParams& p)
{
    // std::cout << "In setIrLoader" << endl;

    if (setIrCaclulatorsParams(p))     // (We run the calculation only if a parameter has changed)
    {    

      for (int i=0;i<NPROC;i++)
      {
        // We have to stop an eventual running thread (and next restart)
        if (calculator[i].isThreadRunning())
          {
            std::cout << "Thread no " << i << " running" << endl;
            if (calculator[i].stopThread(300))
              std::cout << "Thread no " << i << " stopped" << endl;
          }
      }
      
      int n = int(log10(2e-2)/log10(1-p.damp));
      auto dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
      int longueur = int(ceil(dur*p.sampleRate)+NSAMP+int(p.sampleRate*SIGMA_DELTAT));
      int chunksize = floor(2*float(n)/NPROC);

      // Set the size of multithread loops
      for (int i=0;i<NPROC;i++)
      {
          irBuffer[i].setSize (2, int(longueur),false,true);
          calculator[i].n = n;
          calculator[i].nxmin = -n+1 + i*chunksize;
          calculator[i].nxmax = -n+1 + (i+1)*chunksize;
          if (i==NPROC-1)
            calculator[i].nxmax = n;
          calculator[i].resetProgress();
      }

      for (int i=0;i<NPROC;i++)
      {
        std::cout << "Start thread no " << i << std::endl;
        calculator[i].startThread();
      }
      irTransfer.startThread();
    }
}

bool RoomIR::setIrCaclulatorsParams(IrCalculator::IrCalculatorParams& pa)
{
    // We check if a parameter has changed
    // If nothing has changed, we do nothing and return false
    // If at least one parameter has changed we update params
    // of each threaded calculator
    if (juce::approximatelyEqual(p.rx,pa.rx)
      && juce::approximatelyEqual(p.ry,pa.ry)
      && juce::approximatelyEqual(p.rz,pa.rz)
      && juce::approximatelyEqual(p.lx,pa.lx)
      && juce::approximatelyEqual(p.ly,pa.ly)
      && juce::approximatelyEqual(p.lz,pa.lz)
      && juce::approximatelyEqual(p.sx,pa.sx)
      && juce::approximatelyEqual(p.sy,pa.sy)
      && juce::approximatelyEqual(p.sz,pa.sz)
      && juce::approximatelyEqual(p.damp,pa.damp)
      && juce::approximatelyEqual(p.hfDamp,pa.hfDamp)
      && juce::approximatelyEqual(p.type,pa.type)
      && juce::approximatelyEqual(p.headAzim,pa.headAzim)
      && juce::approximatelyEqual(p.sWidth,pa.sWidth)
      && juce::approximatelyEqual(p.directLevel,pa.directLevel)
      && juce::approximatelyEqual(p.reflectionsLevel,pa.reflectionsLevel)
      && juce::approximatelyEqual(p.sampleRate,pa.sampleRate))
      return false;
    else
      {
        p = pa;
        for (int i=0;i<NPROC;i++)
          calculator[i].setParams(pa);
        return true;
      }
}

float RoomIR::getProgress()
{
  if (!getCalculatingState() && getBufferTransferState() )
    return 1.0;
  else
  {
    float prog = 0;
    for (int i=0;i<NPROC;i++)
      prog += calculator[i].getProgress();
    return prog/NPROC;
  }
}

bool RoomIR::getCalculatingState()
{
  bool isCalc = false;
  for (int i=0;i<NPROC;i++)
    isCalc = isCalc || isCalculating[i];
  return isCalc;
}

bool RoomIR::getBufferTransferState()
{
  return irTransfer.getBufferTransferState();
}

void RoomIR::process(juce::AudioBuffer<float> &buffer)
{
    juce::dsp::AudioBlock<float> block {buffer};
    if (convolution.getCurrentIRSize()>0)
    {
        convolution.process(juce::dsp::ProcessContextReplacing<float>(block));        
    }
}

