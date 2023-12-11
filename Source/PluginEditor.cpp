/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "hrtf.h"

#define INV_SOUNDSPEED 2.9412e-03

#define DEBUG_OUTPUTS

#ifdef DEBUG_OUTPUTS
#include <iostream>
#include <chrono>
using namespace std;
#endif      

//==============================================================================
BinauRoomRevAudioProcessorEditor::BinauRoomRevAudioProcessorEditor (BinauRoomRevAudioProcessor& p)
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

    NSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    NSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(NSlider);
    NSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"N",NSlider);

    dampingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    addAndMakeVisible(dampingSlider);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"D",dampingSlider);

    addAndMakeVisible(addButton);
    addButton.setButtonText("Compute IR");
    addButton.onClick = [this]()
    {
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
          y = 2*ceil(float(iy)/2)*ry+pow(-1,iy)*sy;
          float dist = sqrt((x-rx)*(x-rx)+(y-ry)*(y-ry));
          float time = dist*INV_SOUNDSPEED;
          int indice = int(round(time*audioProcessor.spec.sampleRate));
          float r = pow(1-damp,abs(ix)+abs(iy));
          float gain = r/dist;

          // For now we fix elevation and azimuth at zero (i=4, j=0)
          int i = 4;
          int j = 0;

          addArrayToBuffer(&dataL[indice], &lhrtf[i][j][0], gain);
          addArrayToBuffer(&dataR[indice], &rhrtf[i][j][0], gain);
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
    setSize (400, 500);

}

BinauRoomRevAudioProcessorEditor::~BinauRoomRevAudioProcessorEditor()
{
}

//==============================================================================
void BinauRoomRevAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void BinauRoomRevAudioProcessorEditor::resized()
{
    roomXSlider.setBounds(getWidth()/4-40,getHeight()/5-80,80,80);
    roomYSlider.setBounds(3*getWidth()/4-40,getHeight()/5-80,80,80);
    listenerXSlider.setBounds(getWidth()/4-40,2*getHeight()/5-80,80,80);
    listenerYSlider.setBounds(3*getWidth()/4-40,2*getHeight()/5-80,80,80);
    sourceXSlider.setBounds(getWidth()/4-40,3*getHeight()/5-80,80,80);
    sourceYSlider.setBounds(3*getWidth()/4-40,3*getHeight()/5-80,80,80);
    NSlider.setBounds(getWidth()/4-80,4*getHeight()/5-80,160,80);
    dampingSlider.setBounds(3*getWidth()/4-80,4*getHeight()/5-80,160,80);

    addButton.setBounds(getWidth()/4-80,5*getHeight()/5-80,160,60);
    removeButton.setBounds(3*getWidth()/4-80,5*getHeight()/5-80,160,60);

}

int BinauRoomRevAudioProcessorEditor::addArrayToBuffer(float *bufPtr, float *hrtfPtr, float gain)
{
  for (int i=0; i<NSAMP; i++)
  {
    bufPtr[i] += hrtfPtr[i]*gain;

  }
  return 0;
}
