#include "RoomIR.h"
#include "hrtf.h"
#include "hrtf44.h"
#include "hrtf48.h"
#include "hrtf88.h"
#include "hrtf96.h"

// ======================================================================

// This is the function where the impulse response is calculated
void IrBoxCalculator::run()
{
    std::cout << "Start calculate" << std::endl;

    isCalculating[0] = true;

    // inBuf is the buffer used for the non-binaural methods
    float outBuf[NSAMP96]={0.f}, inBuf[NSAMP96]={0.f};
    inBuf[10] = 1.f;
    float x,y,z;

    bp->setSize(2,longueur,true,true);
    bp->clear();
    auto* dataL = bp->getWritePointer(0);
    auto* dataR = bp->getWritePointer(1);

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
            int nbounds = abs(ix)+abs(iy)+abs(iz);            

            int indice = int(round((time+juce::Random::getSystemRandom().nextFloat()*SIGMA_DELTAT)*p.sampleRate));
            float r = pow(1-p.damp,nbounds);
            // float gain = pow(-1,ix+iy+iz)*r/dist;
            float gain = (r/dist) * float( !(ix==0 && iy==0 && iz==0) || calculateDirectPath ) ;
            
            float rp = sqrt((p.sx-p.lx)*(p.sx-p.lx)+(p.sy-p.ly)*(p.sy-p.ly));
            float elev = atan2f(z-p.lz,rp)*EIGHTYOVERPI;

            // Azimutal angle calculation
            float theta = atan2f(y-p.ly,-x+p.lx)*EIGHTYOVERPI-90-p.headAzim;
            
            // XY
            if (p.type==0){
              // Apply lowpass filter and add grain to buffer
              auto elevCardio = (1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(elev)));
              auto panGain = 0.25*(1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(theta+45*p.sWidth)))
                              * elevCardio;
              lop(&inBuf[0], &outBuf[0], p.sampleRate,p.hfDamp,nbounds,1);
              addArrayToBuffer(&dataL[indice], &outBuf[0], gain*panGain);
              panGain = 0.25*(1+juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(theta-45*p.sWidth)))
                          * elevCardio;
              lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
              addArrayToBuffer(&dataR[indice], &outBuf[0], gain*panGain);
            }

            // MS with cardio mic for mid channelabs(ix)+abs(iy)
            if (p.type==1){
              // Apply lowpass filter and add grain to buffer
              lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
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
              lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
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
              gain = gain * .707107f;
              if (juce::approximatelyEqual(nearestSampleRate[0],48000.f))
                {
                  lop(&lhrtf48[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
                  lop(&rhrtf48[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataR[indice], &outBuf[0], gain);

                }
              else if (juce::approximatelyEqual(nearestSampleRate[0],88200.f))
                {
                  lop(&lhrtf88[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
                  lop(&rhrtf88[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
                }
              else if (juce::approximatelyEqual(nearestSampleRate[0],96000.f))
                {
                  lop(&lhrtf96[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
                  lop(&rhrtf96[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
                }
              else // if 44.1kHz or any other cases, we use 44.1kHz HRTF
                {
                  lop(&lhrtf44[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
                  lop(&rhrtf44[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
                  addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
                }
            }
          }
        }
        progress = float(ix-nxmin)/float(nxmax-nxmin);
      }
      else return;
    }
    isCalculating[0] = false;
    cout << "Done" << endl;
}

// Add a given array to a buffer
void IrBoxCalculator::addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain)
{
  for (int i=0; i<nsamp[0]; i++)
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
    for (int i=1;i<nsamp[0];i++)
    {
      out[i] = alpha*in[i] + alpha1*out[i-1];
    }
    for (int j=0; j<order-1; j++)
    {
      out[0] *= alpha;
      for (int i=1;i<nsamp[0];i++)
      {
        out[i] = alpha*out[i] + alpha1*out[i-1];
      }
    }
}

// Get max (has been used for debugging puposes only)
float IrBoxCalculator::max(const float* in)
{
  float max = 0;
  for (int i=1;i<nsamp[0];i++)
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

void IrBoxCalculator::setHrtfVars(int* ns, float* nsr)
{
  nsamp = ns;
  nearestSampleRate = nsr;
}

IrBoxCalculator::IrBoxCalculator() : juce::Thread("calc")
{

}

// ===============================================================
// ===============================================================
IrTransfer::IrTransfer() : juce::Thread("transfer")
{

}

void IrTransfer::run()
{

  hasTransferred = false;
  
  // First we must wait for the buffers to be ready
  bool isCalc = true;
  while (isCalc)
  {
    if (threadShouldExit())
      return;
    else
      {
        isCalc = false;
        for (int i=0;i<threadsNum;i++)
          {
            isCalc = isCalc || isCalculating[i];
          }
        sleep(200);
        continue;
      }
  }

  std::cout << "Buffer copy...." ;
  tempBuf.makeCopyOf(bp[0],true);
  std:cout << "Buffer copy done. Size : " << tempBuf.getNumSamples() << std::endl;

  for (int i=0;i<threadsNum;i++)
    {
      tempBuf.addFrom(0,0,bp[i],0,0,bp[i].getNumSamples());
      tempBuf.addFrom(1,0,bp[i],1,0,bp[i].getNumSamples());
    }

  irp->loadImpulseResponse(std::move (tempBuf),
                      sampleRate,
                      juce::dsp::Convolution::Stereo::yes,
                      juce::dsp::Convolution::Trim::no,
                      juce::dsp::Convolution::Normalise::no);

  hasTransferred = true;
}

void IrTransfer::setBuffer(juce::AudioBuffer<float>* bufPointer)
{
  bp = bufPointer;

}
void IrTransfer::setIr(juce::dsp::Convolution* irPointer)
{
  irp = irPointer;
}

void IrTransfer::setCalculatingBool(bool* cp)
{
  isCalculating = cp;
}

void IrTransfer::setSampleRate(double sr)
{
  sampleRate = sr;
}

double IrTransfer::getSampleRate()
{
  return sampleRate;
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

BoxRoomIR::BoxRoomIR()
{

}

void BoxRoomIR::initialize()
{
    hasInitialized = false;

    std::cout << "In BoxRoomIR::initialize()" << std::endl;

    int numCpus = juce::SystemStats::getNumPhysicalCpus();

    cout << "Number of CPUs : " << juce::SystemStats::getNumCpus() << endl;
    cout << "Number of physical CPUs : " << numCpus << endl;
    threadsNum = std::max<int>(0,std::min<int>(numCpus-1,MAXTHREADS));
    cout << "Number of threads : " << threadsNum << endl;

    // Calculators for the room reflexions (box)

    for (int i=0; i<threadsNum; i++)
    {
      boxCalculator[i].setCalculatingBool(&isCalculating[i]);
      boxCalculator[i].setBuffer(&boxIrBuffer[i]);
      boxCalculator[i].setCalculateDirectPath(false);
      boxCalculator[i].setHrtfVars(&nsamp, &nearestSampleRate);
    }

    boxIrTransfer.setCalculatingBool(&isCalculating[0]);
    boxIrTransfer.setBuffer(&boxIrBuffer[0]);
    boxIrTransfer.setIr(&boxConvolution);
    boxIrTransfer.setThreadsNum(threadsNum);

    // Calculator for the direct path

    directCalculator.setCalculatingBool(&isCalculatingDirect);
    directCalculator.setBuffer(&directIrBuffer);
    directCalculator.setCalculateDirectPath(true);
    directCalculator.setHrtfVars(&nsamp, &nearestSampleRate);

    directIrTransfer.setCalculatingBool(&isCalculatingDirect);
    directIrTransfer.setBuffer(&directIrBuffer);
    directIrTransfer.setIr(&directConvolution);
    directIrTransfer.setThreadsNum(1);

    hasInitialized = true;

}

void BoxRoomIR::prepare(juce::dsp::ProcessSpec spec)
{

    inputBufferCopy.setSize(2, spec.maximumBlockSize ,false,true);

    cout << "Actual sampleRate : " << spec.sampleRate << " Hz." << endl;

    // We have hrtf only for a discrete set of samplerates
    // (typically 44.1, 48, 88.2, 96)
    nearestSampleRate = 44100.f;
    float distance = 200000.f;
    for (float sr : possibleSampleRates)
      if (abs(sr-spec.sampleRate)<distance)
      {
        distance = abs(sr-spec.sampleRate);
        nearestSampleRate = sr;
      };

      if (juce::approximatelyEqual(nearestSampleRate, 44100.f))
        nsamp = NSAMP44;
      else if (juce::approximatelyEqual(nearestSampleRate, 48000.f))
        nsamp = NSAMP48;
      else if (juce::approximatelyEqual(nearestSampleRate, 88200.f))
        nsamp = NSAMP88;
      else if (juce::approximatelyEqual(nearestSampleRate, 96000.f))
        nsamp = NSAMP96;
    
    if (!juce::approximatelyEqual(distance,0.f))
      cout << "Warning : sample rate of " << p.sampleRate
            << " Hz not in possible sample rates, using HRTF at "
            << nearestSampleRate << " Hz." << endl;
    else
      {
        cout << "Sample rate : " << nearestSampleRate << " Hz" << endl;
        cout << "HRTF size : " << nsamp << endl;
      }

    boxIrTransfer.setSampleRate(spec.sampleRate);
    boxConvolution.reset();
    boxConvolution.prepare(spec);

    directIrTransfer.setSampleRate(spec.sampleRate);
    directConvolution.reset();
    directConvolution.prepare(spec);

    // Output highpass filter to cut everything below 15Hz
    for (int i=0; i<2; i++)
    {
      filter[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate,15.f);
      filter[i].prepare(spec);  
    }

    std::cout << "BoxRoomIR::prepare has finished. \n";

}

void BoxRoomIR::calculate(IrBoxCalculatorParams& p)
{
    if (setIrCaclulatorsParams(p))     // (We run the calculation only if a parameter has changed)
    {    

      // We have to stop an eventual running thread (and next restart)

      for (int i=0;i<threadsNum;i++)
      {
        if (boxCalculator[i].isThreadRunning())
          {
            std::cout << "Thread no " << i << " running" << endl;
            if (boxCalculator[i].stopThread(1000))
              std::cout << "Thread no " << i << " stopped" << endl;
          }
      }
      if (directCalculator.isThreadRunning())
      {
        std::cout << "Thread direct running" << endl;
        if (directCalculator.stopThread(500))
          std::cout << "Thread direct stopped" << endl;            
      }
      
      // We should also ask for any IR transfer to stop

      if (boxIrTransfer.isThreadRunning())
        {
          std::cout << "Box IR thread running" << endl;
          if (boxIrTransfer.stopThread(500))
            std::cout << "Thread box IR stopped" << endl;
        }

      if (directIrTransfer.isThreadRunning())
      {
        std::cout << "Thread direct running" << endl;
        if (directIrTransfer.stopThread(500))
          std::cout << "Thread direct IR stopped" << endl;            
      }

      // Set some multithread loops parameters

      int n = int(log10(2e-2)/log10(1-p.damp));
      auto dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
      int longueur = int(ceil(dur*p.sampleRate)+nsamp+int(p.sampleRate*SIGMA_DELTAT));
      int chunksize = floor(2*float(n)/threadsNum);

      for (int i=0;i<threadsNum;i++)
      {
          boxCalculator[i].longueur = longueur;
          boxCalculator[i].n = n;
          boxCalculator[i].nxmin = -n+1 + i*chunksize;
          boxCalculator[i].nxmax = -n+1 + (i+1)*chunksize;
          if (i==threadsNum-1)
            boxCalculator[i].nxmax = n;
          boxCalculator[i].resetProgress();
      }

      n = 1;
      dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
      longueur = int(ceil(dur*p.sampleRate)+nsamp+int(p.sampleRate*SIGMA_DELTAT));

      directCalculator.longueur = longueur;
      directCalculator.n = n;
      directCalculator.nxmin = 0;
      directCalculator.nxmax = 1;
      directCalculator.resetProgress();

      // Start the threads

      for (int i=0;i<threadsNum;i++)
      {
        std::cout << "Start box thread no " << i << std::endl;
        boxCalculator[i].startThread();
      }
      std::cout << "Start direct path thread" << std::endl;
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

    for (int i=0;i<buffer.getNumChannels();i++)
    {
      inputBufferCopy.copyFrom(i,0,buffer,i,0,buffer.getNumSamples());
    }
    
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::AudioBlock<float> blockCopy (inputBufferCopy);
    juce::dsp::AudioBlock<float> blockL = block.getSingleChannelBlock(0);
    juce::dsp::AudioBlock<float> blockR = block.getSingleChannelBlock(1);

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

    filter[0].process(juce::dsp::ProcessContextReplacing<float>(blockL));
    filter[1].process(juce::dsp::ProcessContextReplacing<float>(blockR));

}

void BoxRoomIR::exportIrToWav(juce::File file)
{

  std::unique_ptr<juce::AudioFormatWriter> writer;
  juce::FileOutputStream stream(file);

  // Mix the Ir buffers to get a single 2-channels buffer
  juce::AudioBuffer<float> fullBuffer(2, boxCalculator[0].longueur);
  std::cout << "Box buffer length : " << fullBuffer.getNumSamples() << std::endl; 
  fullBuffer.clear();

  if (getBufferTransferState())
  {
    for (int i=0;i<threadsNum;i++)
    {
    fullBuffer.addFrom(0,0,boxIrBuffer[i],0,0,boxIrBuffer[i].getNumSamples());
    fullBuffer.addFrom(1,0,boxIrBuffer[i],1,0,boxIrBuffer[i].getNumSamples());
    }

    fullBuffer.addFrom(0,0,directIrBuffer,0,0,directIrBuffer.getNumSamples());
    fullBuffer.addFrom(1,0,directIrBuffer,1,0,directIrBuffer.getNumSamples());

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    writer.reset (format.createWriterFor (new juce::FileOutputStream (file),
                                          boxIrTransfer.getSampleRate(),
                                          fullBuffer.getNumChannels(),
                                          24,
                                          {},
                                          0));
    if (writer != nullptr)
      {
        cout << "fullBuffer length : " << fullBuffer.getNumSamples() << endl; 
        writer->writeFromAudioSampleBuffer (fullBuffer, 0, fullBuffer.getNumSamples());
      }
    else
      {
        std::cout << "Writer = nullptr" << std::endl;
      }     
  }
  else
  {
    std::cout << "Buffers not ready" << std::endl;
  }
  
}
