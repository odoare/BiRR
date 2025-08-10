#include "RoomIR_ambi.h"

// ======================================================================

// This is the function where the impulse response is calculated
void IrBoxCalculator::run()
{
    
    isCalculating[0] = true;

    // inBuf is the buffer used for the non-binaural methods
    float outBuf[NSAMP]={0.f}, inBuf[NSAMP]={0.f};
    inBuf[2] = 1.f;
    float x,y,z;
    float dist, time, r, gain, rp, elev, theta, costheta, sintheta, cosphi, sinphi;
    int nbounds, indice;

    bpWY->setSize(2,longueur,true,true);
    bpWY->clear();
    bpZX->setSize(2,longueur,true,true);
    bpZX->clear();

    auto* dataW = bpWY->getWritePointer(0);
    auto* dataY = bpWY->getWritePointer(1);
    auto* dataZ = bpZX->getWritePointer(0);
    auto* dataX = bpZX->getWritePointer(1);

    for (float ix = float(nxmin); ix < float(nxmax) ; ++ix)
    {
      x = 2*ceil(ix/2)*p.rx+pow(-1,ix)*p.sx;

      if (!threadShouldExit())
      {
        for (float iy = -float(n)+1.f; iy < float(n) ; ++iy)
        {
          y = 2*ceil(iy/2)*p.ry+pow(-1,iy)*p.sy;
          for (float iz=-float(n)+1.f; iz<float(n); ++iz)
          {
            z = 2*ceil(iz/2)*p.rz+pow(-1,iz)*p.sz;
            dist = sqrt((x-p.lx)*(x-p.lx)+(y-p.ly)*(y-p.ly)+(z-p.lz)*(z-p.lz));
            time = dist*INV_SOUNDSPEED;
            nbounds = abs(ix)+abs(iy)+abs(iz);

            indice = int(round((time+(juce::Random::getSystemRandom().nextFloat())*p.diffusion)*p.sampleRate));
            r = pow(1-p.damp,nbounds);
            gain = (r/dist) * float( !(ix==0.f && iy==0.f && iz==0.f) || calculateDirectPath ) ;
            
            rp = sqrt((p.sx-p.lx)*(p.sx-p.lx)+(p.sy-p.ly)*(p.sy-p.ly));
            elev = atan2f(z-p.lz,rp)*EIGHTYOVERPI;

            // Azimutal angle calculation
            // In the ambisonic case, the head orientation is managed
            // at the end of the process by matrix multiplication
            theta = atan2f(y-p.ly,-x+p.lx)*EIGHTYOVERPI-90;

            // Apply filter on the grain
            lop(&inBuf[0], &outBuf[0], p.sampleRate, p.hfDamp,nbounds,1);
            // Add grains to the buffers
            costheta = juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(-theta));
            sintheta = juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*(-theta));
            cosphi = juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*(elev));
            sinphi = juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*(elev));
            addArrayToBuffer(&dataW[indice], &outBuf[0], gain);
            addArrayToBuffer(&dataY[indice], &outBuf[0], gain*sintheta*cosphi);
            addArrayToBuffer(&dataZ[indice], &outBuf[0], gain*sinphi);
            addArrayToBuffer(&dataX[indice], &outBuf[0], gain*costheta*cosphi);
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
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;
  }
}

// Basic lowpass filter applied to each grain
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
    // std::cout << "In Lop, nsamp = " << nsamp[0]<< std::endl;            
    for (int j=0; j<order-1; j++)
    {
      out[0] *= alpha;
      for (int i=1;i<NSAMP;i++)
      {
        out[i] = alpha*out[i] + alpha1*out[i-1];
      }
    }
}

// Get max (has been used for debugging puposes only)
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

void IrBoxCalculator::setBuffers(juce::AudioBuffer<float>* bWY, juce::AudioBuffer<float>* bZX)
{
  bpWY = bWY;
  bpZX = bZX;
}

void IrBoxCalculator::setCalculateDirectPath(bool c)
{
  calculateDirectPath = c;
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

  if (irp != nullptr)
  {
    std::cout << "Transferring impulse response..." << std::endl;
    irp->loadImpulseResponse(std::move(tempBuf),
                        sampleRate,
                        juce::dsp::Convolution::Stereo::yes,
                        juce::dsp::Convolution::Trim::no,
                        juce::dsp::Convolution::Normalise::no);
    hasTransferred = true;
    std::cout << "Transfer done." << std::endl;
  }
  else
  {
    std::cerr << "Error: irp pointer is null in IrTransfer::run()" << std::endl;
    hasTransferred = false;
  }
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
      boxCalculator[i].setBuffers(&boxIrBufferWY[i], &boxIrBufferZX[i]);
      boxCalculator[i].setCalculateDirectPath(false);
    }

    boxIrTransferWY.setCalculatingBool(&isCalculating[0]);
    boxIrTransferWY.setBuffer(&boxIrBufferWY[0]);
    boxIrTransferWY.setIr(&boxConvolutionWY);
    boxIrTransferWY.setThreadsNum(threadsNum);

    boxIrTransferZX.setCalculatingBool(&isCalculating[0]);
    boxIrTransferZX.setBuffer(&boxIrBufferZX[0]);
    boxIrTransferZX.setIr(&boxConvolutionZX);
    boxIrTransferZX.setThreadsNum(threadsNum);

    // Calculator for the direct path

    directCalculator.setCalculatingBool(&isCalculatingDirect);
    directCalculator.setBuffers(&directIrBufferWY, &directIrBufferZX);
    directCalculator.setCalculateDirectPath(true);

    directIrTransferWY.setCalculatingBool(&isCalculatingDirect);
    directIrTransferWY.setBuffer(&directIrBufferWY);
    directIrTransferWY.setIr(&directConvolutionWY);
    directIrTransferWY.setThreadsNum(1);

    directIrTransferZX.setCalculatingBool(&isCalculatingDirect);
    directIrTransferZX.setBuffer(&directIrBufferZX);
    directIrTransferZX.setIr(&directConvolutionZX);
    directIrTransferZX.setThreadsNum(1);

    hasInitialized = true;
    std::cout << "Has initialized" << std::endl;

}

void BoxRoomIR::prepare(juce::dsp::ProcessSpec spec)
{

    inputBufferCopyWYZX.setSize(4, spec.maximumBlockSize ,false,true);

    cout << "Actual sampleRate : " << spec.sampleRate << " Hz." << endl;

    boxIrTransferWY.setSampleRate(spec.sampleRate);
    boxConvolutionWY.reset();
    boxConvolutionWY.prepare(spec);
    boxIrTransferZX.setSampleRate(spec.sampleRate);
    boxConvolutionZX.reset();
    boxConvolutionZX.prepare(spec);

    directIrTransferWY.setSampleRate(spec.sampleRate);
    directConvolutionWY.reset();
    directConvolutionWY.prepare(spec);
    directIrTransferZX.setSampleRate(spec.sampleRate);
    directConvolutionZX.reset();
    directConvolutionZX.prepare(spec);

    // Output highpass filter to cut everything below 15Hz
    for (int i=0; i<4; i++)
    {
      filter[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate,15.f);
      filter[i].prepare(spec);  
    }

    std::cout << "BoxRoomIR::prepare has finished. \n";

}

void BoxRoomIR::calculate(IrBoxCalculatorParams& p)
{
    // std::cout << "Start calculate" << std::endl;

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

      if (boxIrTransferWY.isThreadRunning())
        {
          std::cout << "Box IR WY thread running" << endl;
          if (boxIrTransferWY.stopThread(500))
            std::cout << "Thread box IR WY stopped" << endl;
        }
      if (boxIrTransferZX.isThreadRunning())
        {
          std::cout << "Box IR ZX thread running" << endl;
          if (boxIrTransferZX.stopThread(500))
            std::cout << "Thread box IR ZX stopped" << endl;
        }

      if (directIrTransferWY.isThreadRunning())
      {
        std::cout << "Thread direct WY running" << endl;
        if (directIrTransferWY.stopThread(500))
          std::cout << "Thread direct IR WY stopped" << endl;            
      }
      
      if (directIrTransferZX.isThreadRunning())
      {
        std::cout << "Thread direct ZX running" << endl;
        if (directIrTransferZX.stopThread(500))
          std::cout << "Thread direct IR ZX stopped" << endl;            
      }

      // Set some multithread loops parameters

      int n = int(log10(2e-2)/log10(1-p.damp));
      std::cout << "n = " << n << std::endl;
      auto dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
      int longueur = int(ceil(dur*p.sampleRate)+NSAMP+int(p.sampleRate*p.diffusion));
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
      longueur = int(ceil(dur*p.sampleRate)+NSAMP+int(p.sampleRate*p.diffusion));

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

      boxIrTransferWY.startThread();
      directIrTransferWY.startThread();
      boxIrTransferZX.startThread();
      directIrTransferZX.startThread();
    }
}

bool BoxRoomIR::setIrCaclulatorsParams(IrBoxCalculatorParams& pa)
{
    // We check if a parameter has changed
    // If nothing has changed, we do nothing and return false
    // If at least one parameter has changed we update params
    // of each threaded calculator
    //
    // We do not test on headAzim because it doesn't necessitate
    // to recompute IRs when changed

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
      && juce::approximatelyEqual(p.diffusion,pa.diffusion)
      && juce::approximatelyEqual(p.sampleRate,pa.sampleRate))
      {
        p = pa;
        for (int i=0;i<threadsNum;i++)
          boxCalculator[i].setParams(pa);
        directCalculator.setParams(pa);
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
  return (boxIrTransferWY.getBufferTransferState() && boxIrTransferZX.getBufferTransferState()) ;
}

void BoxRoomIR::process(juce::AudioBuffer<float> &bufferWYZX)
{

    // cout << "In BoxRoomIR::process" << endl;

    inputBufferCopyWYZX.makeCopyOf(bufferWYZX,true);

    juce::dsp::AudioBlock<float> blockWYZX (bufferWYZX);
    juce::dsp::AudioBlock<float> blockCopyWYZX (inputBufferCopyWYZX);

    juce::dsp::AudioBlock<float> blockWY = blockWYZX.getSubsetChannelBlock(0,2);
    juce::dsp::AudioBlock<float> blockZX = blockWYZX.getSubsetChannelBlock(2,2);
    juce::dsp::AudioBlock<float> blockCopyWY = blockCopyWYZX.getSubsetChannelBlock(0,2);
    juce::dsp::AudioBlock<float> blockCopyZX = blockCopyWYZX.getSubsetChannelBlock(2,2);

    juce::dsp::AudioBlock<float> blockW = blockWYZX.getSingleChannelBlock(0);
    juce::dsp::AudioBlock<float> blockY = blockWYZX.getSingleChannelBlock(1);
    juce::dsp::AudioBlock<float> blockZ = blockWYZX.getSingleChannelBlock(2);
    juce::dsp::AudioBlock<float> blockX = blockWYZX.getSingleChannelBlock(3);

    if (boxConvolutionWY.getCurrentIRSize()>0)
    {
        boxConvolutionWY.process(juce::dsp::ProcessContextReplacing<float>(blockWY));        
    }

    if (boxConvolutionZX.getCurrentIRSize()>0)
    {
        boxConvolutionZX.process(juce::dsp::ProcessContextReplacing<float>(blockZX));        
    }

    bufferWYZX.applyGain(reflectionsLevel);
    
    if (directConvolutionWY.getCurrentIRSize()>0)
    {
        directConvolutionWY.process(juce::dsp::ProcessContextReplacing<float>(blockCopyWY));        
    }

    if (directConvolutionZX.getCurrentIRSize()>0)
    {
        directConvolutionZX.process(juce::dsp::ProcessContextReplacing<float>(blockCopyZX));        
    }

    bufferWYZX.addFrom(0,0,inputBufferCopyWYZX,0,0,inputBufferCopyWYZX.getNumSamples(),directLevel);
    bufferWYZX.addFrom(1,0,inputBufferCopyWYZX,1,0,inputBufferCopyWYZX.getNumSamples(),directLevel);
    bufferWYZX.addFrom(2,0,inputBufferCopyWYZX,2,0,inputBufferCopyWYZX.getNumSamples(),directLevel);
    bufferWYZX.addFrom(3,0,inputBufferCopyWYZX,3,0,inputBufferCopyWYZX.getNumSamples(),directLevel);

    filter[0].process(juce::dsp::ProcessContextReplacing<float>(blockW));
    filter[1].process(juce::dsp::ProcessContextReplacing<float>(blockY));
    filter[2].process(juce::dsp::ProcessContextReplacing<float>(blockZ));
    filter[3].process(juce::dsp::ProcessContextReplacing<float>(blockX));

    // We use the copy block for the rotation operation
    // As we only rotate around Z, only X and Y components are modified
    // However, the four components are needed to compute the four trigonometric operations
    // sin(X) cos(X) sin(Y) cos(Y)
    juce::dsp::AudioBlock<float> blockCopyY1 = blockCopyWYZX.getSingleChannelBlock(0);
    juce::dsp::AudioBlock<float> blockCopyY2 = blockCopyWYZX.getSingleChannelBlock(1);
    juce::dsp::AudioBlock<float> blockCopyX1 = blockCopyWYZX.getSingleChannelBlock(2);
    juce::dsp::AudioBlock<float> blockCopyX2 = blockCopyWYZX.getSingleChannelBlock(3);

    // std::cout << "Head azim" << p.headAzim << std::endl;

    blockCopyX1.replaceWithProductOf(blockX,juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*p.headAzim));
    blockCopyX2.replaceWithProductOf(blockX,juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*p.headAzim));
    blockCopyY1.replaceWithProductOf(blockY,juce::dsp::FastMathApproximations::cos(PIOVEREIGHTY*p.headAzim));
    blockCopyY2.replaceWithProductOf(blockY,-juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*p.headAzim));

    blockX.replaceWithSumOf(blockCopyX1,blockCopyY2);
    blockY.replaceWithSumOf(blockCopyX2,blockCopyY1);

    // cout << "End of BoxRoomIR::process" << endl;

}

void BoxRoomIR::exportIrToWav(juce::File file)
{

  std::unique_ptr<juce::AudioFormatWriter> writer;
  juce::FileOutputStream stream(file);

  // Mix the Ir buffers to get a single 4-channels buffer
  juce::AudioBuffer<float> fullBuffer(4, boxCalculator[0].longueur);
  std::cout << "Box buffer length : " << fullBuffer.getNumSamples() << std::endl; 
  fullBuffer.clear();

  if (getBufferTransferState())
  {
    for (int i=0;i<threadsNum;i++)
    {
    fullBuffer.addFrom(0,0,boxIrTransferWY.bp[i],0,0,boxIrBufferWY[i].getNumSamples());
    fullBuffer.addFrom(1,0,boxIrTransferWY.bp[i],1,0,boxIrBufferWY[i].getNumSamples());
    fullBuffer.addFrom(2,0,boxIrTransferZX.bp[i],0,0,boxIrBufferZX[i].getNumSamples());
    fullBuffer.addFrom(3,0,boxIrTransferZX.bp[i],1,0,boxIrBufferZX[i].getNumSamples());
    }

    fullBuffer.addFrom(0,0,directIrTransferWY.bp[0],0,0,directIrBufferWY.getNumSamples());
    fullBuffer.addFrom(1,0,directIrTransferWY.bp[0],1,0,directIrBufferWY.getNumSamples());
    fullBuffer.addFrom(2,0,directIrTransferZX.bp[0],0,0,directIrBufferZX.getNumSamples());
    fullBuffer.addFrom(3,0,directIrTransferZX.bp[0],1,0,directIrBufferZX.getNumSamples());

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    writer.reset (format.createWriterFor (new juce::FileOutputStream (file),
                                          boxIrTransferWY.getSampleRate(),
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
