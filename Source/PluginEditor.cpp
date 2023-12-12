/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "hrtf.h"

#define INV_SOUNDSPEED 2.9412e-03
#define EIGHTYOVERPI 57.295779513
#define OMEGASTART 125663.706

#define DEBUG_OUTPUTS

#ifdef DEBUG_OUTPUTS
#include <iostream>
#include <chrono>
using namespace std;
#endif      

//==============================================================================
ReverbAudioProcessorEditor::ReverbAudioProcessorEditor (ReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    roomXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    roomXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(roomXSlider);
    roomXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomX",roomXSlider);

    roomYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    roomYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(roomYSlider);
    roomYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomY",roomYSlider);

    listenerXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(listenerXSlider);
    listenerXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerX",listenerXSlider);

    listenerYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(listenerYSlider);
    listenerYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerY",listenerYSlider);

    sourceXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(sourceXSlider);
    sourceXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceX",sourceXSlider);

    sourceYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(sourceYSlider);
    sourceYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceY",sourceYSlider);

    dampingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(dampingSlider);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"D",dampingSlider);

    hfDampingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    hfDampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(hfDampingSlider);
    hfDampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"HFD",hfDampingSlider);

    NSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    NSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(NSlider);
    NSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"N",NSlider);

    addAndMakeVisible(xyPad);
    xyPad.registerSlider(&listenerXSlider, Gui::XyPad::Axis::X1);
    xyPad.registerSlider(&listenerYSlider, Gui::XyPad::Axis::Y1);
    xyPad.registerSlider(&sourceXSlider, Gui::XyPad::Axis::X2);
    xyPad.registerSlider(&sourceYSlider, Gui::XyPad::Axis::Y2);
    addAndMakeVisible(addButton);
    addButton.setButtonText("Compute IR");
    addButton.onClick = [this]()
    {

      float outBuf[NSAMP]; 

      addButton.setEnabled(false);
      removeButton.setEnabled(false);    

      auto rx = audioProcessor.apvts.getRawParameterValue("RoomX")->load();
      auto ry = audioProcessor.apvts.getRawParameterValue("RoomY")->load();
      auto lx = rx*(audioProcessor.apvts.getRawParameterValue("ListenerX")->load());
      auto ly = ry*(audioProcessor.apvts.getRawParameterValue("ListenerY")->load());
      auto sx = rx*(audioProcessor.apvts.getRawParameterValue("SourceX")->load());
      auto sy = ry*(audioProcessor.apvts.getRawParameterValue("SourceY")->load());
      auto n = audioProcessor.apvts.getRawParameterValue("N")->load();
      auto dur = (n+1)*sqrt(rx*rx+ry*ry)/340;
      int longueur = int(ceil(dur*audioProcessor.spec.sampleRate)+NSAMP);
      auto damp = audioProcessor.apvts.getRawParameterValue("D")->load();
      auto hfDamp = audioProcessor.apvts.getRawParameterValue("HFD")->load();
      
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
          int indice = int(round(time*audioProcessor.spec.sampleRate));
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

          // // For now we fix elevation and azimuth at zero (i=4, j=0)
          // int i = 4;
          // int j = 0;

          // Apply lowpass filter
          lop(&lhrtf[elevationIndex][azimutalIndex][0], &outBuf[0], audioProcessor.getSampleRate(),hfDamp,abs(ix)+abs(iy),1);
          addArrayToBuffer(&dataL[indice], &outBuf[0], gain);
          lop(&rhrtf[elevationIndex][azimutalIndex][0], &outBuf[0], audioProcessor.getSampleRate(),hfDamp,abs(ix)+abs(iy),1);
          addArrayToBuffer(&dataR[indice], &outBuf[0], gain);
        }
      }

      #ifdef DEBUG_OUTPUTS
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      cout << "Buffer preparation duration:" << duration.count() << "µs" << endl;
      start = std::chrono::high_resolution_clock::now();
      #endif
      
      audioProcessor.irLoader.reset();
      audioProcessor.irLoader.loadImpulseResponse (std::move (buf),
                          audioProcessor.spec.sampleRate,
                          juce::dsp::Convolution::Stereo::yes,
                          juce::dsp::Convolution::Trim::no,
                          juce::dsp::Convolution::Normalise::no);

      #ifdef DEBUG_OUTPUTS
      stop = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      cout << "Buffer fill duration:" << duration.count() << "µs" << endl;
      #endif

      addButton.setEnabled(true);
      removeButton.setEnabled(true);
    };

    addAndMakeVisible(removeButton);
    removeButton.setButtonText("Reset");
    removeButton.onClick = [this]()
    {

      addButton.setEnabled(false);
      removeButton.setEnabled(false); 
      
      juce::AudioBuffer<float> buf;
      buf.setSize (1, 10);
      
      for (int channel = 0; channel < buf.getNumChannels() ; ++channel)
      {
          auto* channelData = buf.getWritePointer (channel);
          for (int sample=0; sample<buf.getNumSamples(); ++sample)
          {
              channelData[sample] = 0 ;
          }
          channelData[0] = 1.0f;
      }

      audioProcessor.irLoader.loadImpulseResponse (std::move (buf),
                          audioProcessor.spec.sampleRate,
                          juce::dsp::Convolution::Stereo::yes,
                          juce::dsp::Convolution::Trim::no,
                          juce::dsp::Convolution::Normalise::no);

      addButton.setEnabled(true);
      removeButton.setEnabled(true);

    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 500);
    setResizable(true,true);

}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor()
{
  xyPad.deregisterSlider(&listenerXSlider);
  xyPad.deregisterSlider(&listenerYSlider);
  xyPad.deregisterSlider(&sourceXSlider);
  xyPad.deregisterSlider(&sourceYSlider);
}

//==============================================================================
void ReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ReverbAudioProcessorEditor::resized()
{
    roomXSlider.setBounds(getWidth()/6-40,getHeight()/6-80,80,80);
    roomYSlider.setBounds(3*getWidth()/6-40,getHeight()/6-80,80,80);

    listenerXSlider.setBounds(getWidth()/6-40,2*getHeight()/6-80,80,80);
    listenerYSlider.setBounds(3*getWidth()/6-40,2*getHeight()/6-80,80,80);
    
    sourceXSlider.setBounds(getWidth()/6-40,3*getHeight()/6-80,80,80);
    sourceYSlider.setBounds(3*getWidth()/6-40,3*getHeight()/6-80,80,80);
    
    dampingSlider.setBounds(getWidth()/6-80,4*getHeight()/6-80,160,80);
    hfDampingSlider.setBounds(3*getWidth()/6-80,4*getHeight()/6-80,160,80);

    NSlider.setBounds(getWidth()/6-80,5*getHeight()/6-80,160,80);

    addButton.setBounds(getWidth()/6-80,6*getHeight()/6-80,160,60);
    removeButton.setBounds(3*getWidth()/6-80,6*getHeight()/6-80,160,60);

    xyPad.setBounds(4*getWidth()/6-20,getHeight()/6-20,200,200);

}

void ReverbAudioProcessorEditor::addArrayToBuffer(float *bufPtr, float *hrtfPtr, float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;

  }
}

int ReverbAudioProcessorEditor::proximityIndex(const float *data, int length, float value)
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

void ReverbAudioProcessorEditor::lop(float *in, float *out, int sampleFreq, float hfDamping, int nRebounds, int order)
{
    float om = OMEGASTART*(exp(-hfDamping*nRebounds));
    float alpha1 = exp(-om/sampleFreq);
    float alpha = 1 - alpha1;
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
