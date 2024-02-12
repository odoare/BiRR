/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "hrtf.h"

#define DEBUG_OUTPUTS

#ifdef DEBUG_OUTPUTS
#include <iostream>
#include <chrono>
using namespace std;
#endif

#define INV_SOUNDSPEED 2.9412e-03f
#define PIOVEREIGHTY 1.745329252e-02f
#define EIGHTYOVERPI 57.295779513f
#define OMEGASTART 125663.706f
#define SIGMA_DELTAT 1e-3f

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

    irLoader.reset();
    irLoader.prepare(spec);
}

void ReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // if (calculator.isCalculating)
    //   cout << "Calculator is calculating" << endl;
    // if (!calculator.isCalculating)
    //   cout << "Calculator is not calculating" << endl;
    // if (!calculator.isCalculating && !calculator.bufferTransferred)
    // {
    //   cout << "Load buffer..." << endl;
    //   irLoader.loadImpulseResponse(std::move (calculator.buf),
    //                     spec.sampleRate,
    //                     juce::dsp::Convolution::Stereo::yes,
    //                     juce::dsp::Convolution::Trim::no,
    //                     juce::dsp::Convolution::Normalise::no);
    //   calculator.bufferTransferred = true;
    // }

    juce::dsp::AudioBlock<float> block {buffer};
    if (irLoader.getCurrentIRSize()>0)
    {
        irLoader.process(juce::dsp::ProcessContextReplacing<float>(block));        
    }
}

//==============================================================================
bool ReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReverbAudioProcessor::createEditor()
{
    return new ReverbAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
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
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomX","RoomX",1.0f,10.0f,3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomY","RoomY",1.0f,10.0f,4.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomZ","RoomZ",1.0f,10.0f,2.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerX","ListenerX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerY","ListenerY",0.01f,0.99f,0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerZ","ListenerZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerO","ListenerO",juce::NormalisableRange<float>(-180.f,180.f,1.f,1.f),0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceX","SourceX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceY","SourceY",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceZ","SourceZ",0.01f,0.99f,0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("D","D",juce::NormalisableRange<float>(0.02f,0.7f,0.001f,0.3f),0.1f));
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
    std::cout << "In setIrLoader" << endl;

    irCalculator::irCalculatorParams p;

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
    p.directLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Direct Level")->load());
    p.reflectionsLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Reflections Level")->load());
    p.sampleRate = spec.sampleRate;

    std::cout << "Set parameters" << endl;
    calculator.setParams(p);

    calculator.setConvPointer(&irLoader);

    // Ask running thread to stop
    if (calculator.isThreadRunning())
    {
      std::cout << "Thread running" << endl;
      if (calculator.stopThread(100))
        std::cout << "Thread stopped" << endl;
    }

    std::cout << "Start thread" << endl;
    calculator.startThread();
    
    // reset();
    // irLoader.loadImpulseResponse(std::move (buf),
    //                     spec.sampleRate,
    //                     juce::dsp::Convolution::Stereo::yes,
    //                     juce::dsp::Convolution::Trim::no,
    //                     juce::dsp::Convolution::Normalise::no);

    // #ifdef DEBUG_OUTPUTS
    // stop = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    // cout << "Buffer fill duration:" << duration.count() << "µs" << endl;
    // #endif
}

// Add a given array to a buffer
void ReverbAudioProcessor::addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;
  }
}

// Compares the values in data to a float prameter value and returns the nearest index
int ReverbAudioProcessor::proximityIndex(const float *data, const int length, const float value, const bool wrap)
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
void ReverbAudioProcessor::lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order)
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

// ======================================================================

// This is the function where the impulse response is calculated
void irCalculator::run()
{

    isCalculating = true;
    progress = 0.f;

    std::cout << "In irCalculator::run()" << endl;
    static float outBuf[NSAMP], inBuf[NSAMP];

    // inBuf is the buffer used for the non-binaural method
    inBuf[10] = 1.f;

    int n = int(log10(2e-2)/log10(1-p.damp));
    auto dur = (n+1)*sqrt(p.rx*p.rx+p.ry*p.ry+p.rz*p.rz)/340;
    int longueur = int(ceil(dur*p.sampleRate)+NSAMP+int(p.sampleRate*SIGMA_DELTAT));

    // #ifdef DEBUG_OUTPUTS
    // cout << "rx : " << p.rx << "\n" ;
    // cout << "ry : " << p.ry << "\n" ;
    // cout << "rz : " << p.rz << "\n" ;
    // cout << "lx : " << p.lx << "\n" ;
    // cout << "ly : " << p.ly << "\n" ;
    // cout << "lz : " << p.lz << "\n" ;
    // cout << "sx : " << p.sx << "\n" ;
    // cout << "sy : " << p.sy << "\n" ;
    // cout << "sz : " << p.sz << "\n" ;
    // cout << "n : " << n << "\n" ;
    // cout << "dur : " << dur << "\n" ;
    // cout << "longueur : " << longueur << "\n" ;
    // cout << "damp : " << p.damp << "\n" ;
    // cout << "hfDamp : " << p.damp << "\n" ;
    // cout << "type : " << p.type << "\n" ;
    // // auto start = std::chrono::high_resolution_clock::now();
    // #endif

    // cout << "Set buffer size" << endl;
    buf.setSize (2, int(longueur),false,true);


    // cout << "Start buffer fill..." << endl;

    buf.clear();
    auto* dataL = buf.getWritePointer(0);
    auto* dataR = buf.getWritePointer(1);

    // for (int sample=0; sample<buf.getNumSamples(); ++sample)
    // {
    //     dataL[sample] = 0;
    //     dataR[sample] = 0;
    // }
    
    float x,y,z;

    // cout << "Start Loop..." << endl;

    for (int ix = -n+1; ix < n ; ++ix)
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
    
            // #ifdef DEBUG_OUTPUTS
            // if (ix==0 && iy==0 && iz==0)
            // {
            //   cout << "x = " << x << "      y = " << y << "      z = " << z << endl;
            //   cout << "r = " << r << "      dist = " << dist << endl;
            //   cout << "y-ly = " << y-ly << "         -x+lx = " << -lx+x << endl;
            //   // cout << "elev = " << elev << "          elevationIndex = " << elevationIndex << endl;
            //   // cout << "Theta = " << theta << "        Azimutal index = " << azimutalIndex << endl;
            // }
            // #endif
            
            // XY
            if (p.type==0){
              // Apply lowpass filter and add grain to 
              // cout << "XY  ";
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
              // cout << "MS  ";   
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
              // cout << "MS2     ";
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
              // cout << "Binau  ";         
              int elevationIndex = proximityIndex(&elevations[0],NELEV,elev,false);
              int azimutalIndex = proximityIndex(&azimuths[elevationIndex][0],NAZIM,theta,true);
              // Apply lowpass filter and add grain to buffer
              lop(&lhrtfn[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
              lop(&rhrtfn[elevationIndex][azimutalIndex][0], &outBuf[0], p.sampleRate, p.hfDamp,abs(ix)+abs(iy),1);
              addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
            }
            // else if (type==0){
            //   // Apply lowpass filter and add grain to buffer
            //   auto panGain = abs(-juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*(0.5f*theta-45)));
            //   lop(&inBuf[0], &outBuf[0], getSampleRate(),hfDamp,abs(ix)+abs(iy),1);
            //   addArrayToBuffer(&dataL[indice], &outBuf[0], gain*panGain);
            //   panGain = abs(juce::dsp::FastMathApproximations::sin(PIOVEREIGHTY*(0.5*theta+45)));
            //   lop(&inBuf[0], &outBuf[0], getSampleRate(),hfDamp,abs(ix)+abs(iy),1);
            //   addArrayToBuffer(&dataR[indice], &outBuf[0], gain*panGain);
            // }
          }
        }
        progress = float(ix+n-1)/float(2*n-1);
        // cout << progress << endl;
      }
      else return;
    }

    // cout << "Array fill finished" << endl;

    // #ifdef DEBUG_OUTPUTS
    // auto stop = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    // cout << "Buffer preparation duration:" << duration.count() << "µs" << endl;
    // start = std::chrono::high_resolution_clock::now();
    // #endif
    
    // reset();
    // cout << "Load impulse response" << endl;

    bufferTransferred = false;
    isCalculating = false;
    irp->loadImpulseResponse(std::move (buf),
                        p.sampleRate,
                        juce::dsp::Convolution::Stereo::yes,
                        juce::dsp::Convolution::Trim::no,
                        juce::dsp::Convolution::Normalise::no);

    bufferTransferred = true;

    std::cout << "Finished buffer filling" << endl;

    // for (int i=0; i<buf.getNumSamples(); i++)
    // {
    //   cout << dataR[i] << "    ";
    // }


    // #ifdef DEBUG_OUTPUTS
    // stop = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    // cout << "Buffer fill duration:" << duration.count() << "µs" << endl;
    // #endif
}

// Add a given array to a buffer
void irCalculator::addArrayToBuffer(float *bufPtr, const float *hrtfPtr, const float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;
  }
}

// Compares the values in data to a float prameter value and returns the nearest index
int irCalculator::proximityIndex(const float *data, const int length, const float value, const bool wrap)
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
void irCalculator::lop(const float* in, float* out, const int sampleFreq, const float hfDamping, const int nRebounds, const int order)
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

void irCalculator::setConvPointer(juce::dsp::Convolution* ip)
{
  irp = ip;
}

void irCalculator::setParams(irCalculatorParams params)
{
  p = params;
}

float irCalculator::getProgress()
{
  if (!isCalculating && bufferTransferred)
    return 1.0f;
  else
    return progress;
}

irCalculator::irCalculator() : juce::Thread("test")
{

}
