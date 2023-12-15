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

#define INV_SOUNDSPEED 2.9412e-03
#define EIGHTYOVERPI 57.295779513
#define OMEGASTART 125663.706

//==============================================================================
ReverbAudioProcessor::ReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
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
    layout.add(std::make_unique<juce::AudioParameterFloat>("RoomY","RoomY",1.0f,10.0f,3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerX","ListenerX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ListenerY","ListenerY",0.01f,0.99f,0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceX","SourceX",0.01f,0.99f,0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SourceY","SourceY",0.01f,0.99f,0.75f));
    layout.add(std::make_unique<juce::AudioParameterInt>("N","N",1,150,20));
    layout.add(std::make_unique<juce::AudioParameterFloat>("D","D",juce::NormalisableRange<float>(0.0f,1.0f,0.001f,0.3f),0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HFD","HFD",juce::NormalisableRange<float>(0.0f,1.0f,0.001f,0.3f),0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Diffusion","Diffusion",juce::NormalisableRange<float>(0.0f,1.0f,0.001f,0.3f),0.1f));
    
    return layout;
}

// This is the function where the impulse response is calculated
void ReverbAudioProcessor::setIrLoader()
{
    float outBuf[NSAMP], outBuf2[NSAMP]; 

    auto rx = apvts.getRawParameterValue("RoomX")->load();
    auto ry = apvts.getRawParameterValue("RoomY")->load();
    auto lx = rx*(apvts.getRawParameterValue("ListenerX")->load());
    auto ly = ry*(apvts.getRawParameterValue("ListenerY")->load());
    auto sx = rx*(apvts.getRawParameterValue("SourceX")->load());
    auto sy = ry*(apvts.getRawParameterValue("SourceY")->load());
    auto n = apvts.getRawParameterValue("N")->load();
    auto dur = (n+1)*sqrt(rx*rx+ry*ry)/340;
    int longueur = int(ceil(dur*spec.sampleRate)+NSAMP);
    auto damp = apvts.getRawParameterValue("D")->load();
    auto hfDamp = apvts.getRawParameterValue("HFD")->load();
    auto diffusion = apvts.getRawParameterValue("Diffusion")->load();
    
    #ifdef DEBUG_OUTPUTS
    cout << "rx : " << rx << "\n" ;
    cout << "ry : " << ry << "\n" ;
    cout << "lx : " << lx << "\n" ;
    cout << "ly : " << ly << "\n" ;
    cout << "sx : " << sx << "\n" ;
    cout << "sy : " << sy << "\n" ;
    cout << "n : " << n << "\n" ;
    cout << "dur : " << dur << "\n" ;
    cout << "longueur : " << longueur << "\n" ;
    cout << "damp : " << damp << "\n" ;
    cout << "hfDamp : " << damp << "\n" ;
    auto start = std::chrono::high_resolution_clock::now();
    #endif

    juce::AudioBuffer<float> buf;
    buf.setSize (2, int(longueur));
    auto* dataL = buf.getWritePointer(0);
    auto* dataR = buf.getWritePointer(1);

    for (int sample=0; sample<buf.getNumSamples(); ++sample)
    {
        dataL[sample] = 0;
        dataR[sample] = 0;
    }
    
    float x,y;

    for (int ix = 0; ix < n ; ++ix)
    {
    x = 2*float(ceil(float(ix)/2))*rx+pow(-1,ix)*sx;
    for (int iy = 0; iy < n ; ++iy)
    {
        y = 2*float(ceil(float(iy)/2))*ry+pow(-1,iy)*sy;
        float dist = sqrt((x-lx)*(x-lx)+(y-ly)*(y-ly));
        float time = dist*INV_SOUNDSPEED;
        int indice = int(round(time*spec.sampleRate));
        float r = pow(1-damp,abs(ix)+abs(iy));
        float gain = r/dist;

        //Elevation angle is zero here
        int elevationIndex = 4;

        // Azimutal angle calculation
        float theta = atan2f(y-ly,-x+lx)*EIGHTYOVERPI-90;
        int azimutalIndex = proximityIndex(&azimuths[elevationIndex][0],NAZIM,theta);

        #ifdef DEBUG_OUTPUTS
        if (ix==0 and iy==0)
        {
          cout << "x = " << x << "      y = " << y << endl;
          cout << "r = " << r << "      dist = " << dist << endl;
          cout << "y-ly = " << y-ly << "         -x+lx = " << -lx+x << endl;
          cout << "Theta = " << theta << "        Azimutal index = " << azimutalIndex << endl;
        }
        #endif

        // Apply lowpass filter and add grain to buffer
        lop(&lhrtf[elevationIndex][azimutalIndex][0], &outBuf[0], getSampleRate(),hfDamp,abs(ix)+abs(iy),1);
        // alp(&outBuf[0], &outBuf2[0], getSampleRate(),diffusion,abs(ix)+abs(iy));
        addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
        lop(&rhrtf[elevationIndex][azimutalIndex][0], &outBuf[0], getSampleRate(),hfDamp,abs(ix)+abs(iy),1);
        // alp(&outBuf[0], &outBuf2[0], getSampleRate(),diffusion,abs(ix)+abs(iy));
        addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
    }
    }

    #ifdef DEBUG_OUTPUTS
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    cout << "Buffer preparation duration:" << duration.count() << "µs" << endl;
    start = std::chrono::high_resolution_clock::now();
    #endif
    
    reset();
    irLoader.loadImpulseResponse(std::move (buf),
                        spec.sampleRate,
                        juce::dsp::Convolution::Stereo::yes,
                        juce::dsp::Convolution::Trim::no,
                        juce::dsp::Convolution::Normalise::no);

    #ifdef DEBUG_OUTPUTS
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    cout << "Buffer fill duration:" << duration.count() << "µs" << endl;
    #endif
}

// Add a given array to a buffer
void ReverbAudioProcessor::addArrayToBuffer(float *bufPtr, const float *hrtfPtr, float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;

  }
}

// Compares the values in data to a float prameter value and returns the nearest index
int ReverbAudioProcessor::proximityIndex(const float *data, int length, float value)
{
  int proxIndex = 0;
  float minDistance = BIGVALUE;
  float val;
  if (value<0.f)
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
void ReverbAudioProcessor::lop(const float* in, float* out, int sampleFreq, float hfDamping, int nRebounds, int order)
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