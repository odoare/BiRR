#include "RoomIR.h"
#include "../hrtf.h"

// ======================================================================

// This is the function where the impulse response is calculated
void IrBoxCalculator::run()
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
            float gain = (r/dist) * float( !(ix==0 && iy==0 && iz==0) || calculateDirectPath ) ;
            
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
void IrBoxCalculator::addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;
  }
}

// Compares the values in data to a float prameter value and returns the nearest index
int IrBoxCalculator::proximityIndex(const float *data, const int length, const float value, const bool wrap)
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
void IrBoxCalculator::lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order)
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
float IrBoxCalculator::max(const float* in)
{
  float max = 0;
  for (int i=1;i<NSAMP;i++)
  {
    if (in[i]>max)
    max = in[i];
  }
  return max;
}

void IrBoxCalculator::setParams(IrBoxCalculatorParams& pa)
{
  p = pa ;
}

float IrBoxCalculator::getProgress()
{
  return progress;
}

void IrBoxCalculator::resetProgress()
{
  progress = 0.f;
}

void IrBoxCalculator::setCalculatingBool(bool* cp)
{
  isCalculating = cp;
}

void IrBoxCalculator::setBuffer(juce::AudioBuffer<float>* b)
{
  bp = b;
}

void IrBoxCalculator::setCalculateDirectPath(bool c)
{
  calculateDirectPath = c;
}

// void IrBoxCalculator::setThreadsNum(int n)
// {
//   threadsNum = std::max<int>(n,MAXTHREADS);
// }

IrBoxCalculator::IrBoxCalculator() : juce::Thread("test")
{

}

// ===============================================================
// ===============================================================
IrTransfer::IrTransfer() : juce::Thread("transfer")
{

}

void IrTransfer::run()
{

  // A FAIRE

  cout << "In IrTransfer::run()" << endl;
  hasTransferred = false;
  // First we must wait for the buffers to be ready
  bool isCalc = true;
  while (isCalc)
  {
    isCalc = false;
    for (int i=0;i<threadsNum;i++)
      {
        isCalc = isCalc || isCalculating[i];
        //cout << i << " : " << isCalculating[i] << "    " << endl;
      }
    //out << "isCalc : " << isCalc << endl;
    //cout << "Waiting for buffer fill" << endl;
    sleep(200);
    continue;
  }

  std::cout << "Start buffer filling" << endl; 

  std::cout << "Join... " << threadsNum << endl;; 

  for (int i=1;i<threadsNum;i++)
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

  std::cout << "Finished buffer filling... " << threadsNum << endl; 
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

void IrTransfer::setThreadsNum(int n)
{
  threadsNum = std::min<int>(n,MAXTHREADS);
}


// ========================================================
// ========================================================
// This is the function where the impulse response is calculated

BoxRoomIR::BoxRoomIR()
{

}

void BoxRoomIR::prepare(juce::dsp::ProcessSpec spec)
{

    int numCpus = juce::SystemStats::getNumPhysicalCpus();

    cout << "Number of CPUs : " << juce::SystemStats::getNumCpus() << endl;
    cout << "Number of physical CPUs : " << juce::SystemStats::getNumPhysicalCpus() << endl;

    inputBufferCopy.setSize(2, spec.maximumBlockSize ,false,true);

    threadsNum = numCpus;

    for (int i=0; i<threadsNum; i++)
    {
      boxCalculator[i].setCalculatingBool(&isCalculating[i]);
      boxCalculator[i].setBuffer(&boxIrBuffer[i]);
      boxCalculator[i].setCalculateDirectPath(false);
    }

    boxIrTransfer.setCalculatingBool(&isCalculating[0]);
    boxIrTransfer.setBuffer(&boxIrBuffer[0]);
    boxIrTransfer.setIr(&boxConvolution);
    boxIrTransfer.setSampleRate(spec.sampleRate);
    boxIrTransfer.setThreadsNum(threadsNum);

    boxConvolution.reset();
    boxConvolution.prepare(spec);

    directCalculator.setCalculatingBool(&isCalculatingDirect);
    directCalculator.setBuffer(&directIrBuffer);
    directCalculator.setCalculateDirectPath(true);

    directIrTransfer.setCalculatingBool(&isCalculatingDirect);
    directIrTransfer.setBuffer(&directIrBuffer);
    directIrTransfer.setIr(&directConvolution);
    directIrTransfer.setSampleRate(spec.sampleRate);
    directIrTransfer.setThreadsNum(1);

    directConvolution.reset();
    directConvolution.prepare(spec);
}

void BoxRoomIR::calculate(IrBoxCalculatorParams& p)
{
    // std::cout << "In setIrLoader" << endl;

    if (setIrCaclulatorsParams(p))     // (We run the calculation only if a parameter has changed)
    {    

      // We have to stop an eventual running thread (and next restart)

      for (int i=0;i<threadsNum;i++)
      {
        if (boxCalculator[i].isThreadRunning())
          {
            std::cout << "Thread no " << i << " running" << endl;
            if (boxCalculator[i].stopThread(500))
              std::cout << "Thread no " << i << " stopped" << endl;
          }
      }
      if (directCalculator.isThreadRunning())
      {
        std::cout << "Thread direct running" << endl;
        if (directCalculator.stopThread(500))
          std::cout << "Thread direct stopped" << endl;            
      }

      // Set the size of multithread loops parameters

      int n = int(log10(2e-2)/log10(1-p.damp));
      auto dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
      int longueur = int(ceil(dur*p.sampleRate)+NSAMP+int(p.sampleRate*SIGMA_DELTAT));
      int chunksize = floor(2*float(n)/threadsNum);
      
      for (int i=0;i<threadsNum;i++)
      {
          boxIrBuffer[i].setSize (2, int(longueur),false,true);
          boxCalculator[i].n = n;
          boxCalculator[i].nxmin = -n+1 + i*chunksize;
          boxCalculator[i].nxmax = -n+1 + (i+1)*chunksize;
          if (i==threadsNum-1)
            boxCalculator[i].nxmax = n;
          boxCalculator[i].resetProgress();
      }

      n = 1;
      dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
      longueur = int(ceil(dur*p.sampleRate)+NSAMP+int(p.sampleRate*SIGMA_DELTAT));

      directIrBuffer.setSize(2, int(longueur),false,true);
      directCalculator.n = n;
      directCalculator.nxmin = 0;
      directCalculator.nxmax = 1;
      directCalculator.resetProgress();

      // Start the threads

      for (int i=0;i<threadsNum;i++)
      {
        std::cout << "Start thread no " << i << std::endl;
        boxCalculator[i].startThread();
      }
      directCalculator.startThread();

      boxIrTransfer.startThread();
      directIrTransfer.startThread();
    }
}

bool BoxRoomIR::setIrCaclulatorsParams(IrBoxCalculatorParams& pa)
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
      && juce::approximatelyEqual(p.sampleRate,pa.sampleRate))
      {
        return false;
      }
    else
      {
        p = pa;
        for (int i=0;i<threadsNum;i++)
          boxCalculator[i].setParams(pa);
        directCalculator.setParams(pa);
        return true;
      }
}

float BoxRoomIR::getProgress()
{
  if (!getCalculatingState() && getBufferTransferState() )
    return 1.0;
  else
  {
    float prog = 0;
    for (int i=0;i<threadsNum;i++)
      prog += boxCalculator[i].getProgress();
    return prog/threadsNum;
  }
}

bool BoxRoomIR::getCalculatingState()
{
  bool isCalc = false;
  for (int i=0;i<threadsNum;i++)
    isCalc = isCalc || isCalculating[i];
  return isCalc;
}

bool BoxRoomIR::getBufferTransferState()
{
  return boxIrTransfer.getBufferTransferState();
}

void BoxRoomIR::process(juce::AudioBuffer<float> &buffer)
{
    inputBufferCopy.copyFrom(0,0,buffer,0,0,buffer.getNumSamples());
    inputBufferCopy.copyFrom(1,0,buffer,1,0,buffer.getNumSamples());

    
    juce::dsp::AudioBlock<float> block {buffer};
    juce::dsp::AudioBlock<float> blockCopy {inputBufferCopy};
    
    if (boxConvolution.getCurrentIRSize()>0)
    {
        boxConvolution.process(juce::dsp::ProcessContextReplacing<float>(block));        
    }

    buffer.applyGain(reflectionsLevel);

    if (directConvolution.getCurrentIRSize()>0)
    {
        directConvolution.process(juce::dsp::ProcessContextReplacing<float>(blockCopy));        
    }

    buffer.addFrom(0,0,inputBufferCopy,0,0,inputBufferCopy.getNumSamples(),directLevel);
    buffer.addFrom(1,0,inputBufferCopy,1,0,inputBufferCopy.getNumSamples(),directLevel);

}
