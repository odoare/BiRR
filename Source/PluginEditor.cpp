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
    roomXSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    roomXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    roomXSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::teal);
    roomXSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    roomXSlider.setColour(juce::Slider::thumbColourId, juce::Colours::teal);
    addAndMakeVisible(roomXSlider);
    roomXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomX",roomXSlider);

    addAndMakeVisible(roomXLabel);
    roomXLabel.setJustificationType(juce::Justification::centred);
    roomXLabel.attachToComponent(&roomXSlider,false);

    addAndMakeVisible(roomYLabel);
    roomYLabel.setJustificationType(juce::Justification::centred);
    roomYLabel.attachToComponent(&roomYSlider,false);

    roomYSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    roomYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    roomYSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::teal);
    roomYSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    roomYSlider.setColour(juce::Slider::thumbColourId, juce::Colours::teal);
    addAndMakeVisible(roomYSlider);
    roomYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomY",roomYSlider);

    listenerXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    listenerXSlider.setColour(juce::Slider::thumbColourId, listenerColour);
    listenerXSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    listenerXSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(listenerXSlider);
    listenerXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerX",listenerXSlider);

    listenerYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    listenerYSlider.setColour(juce::Slider::thumbColourId, listenerColour);
    listenerYSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    listenerYSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(listenerYSlider);
    listenerYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerY",listenerYSlider);

    sourceXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    sourceXSlider.setColour(juce::Slider::thumbColourId, sourceColour);
    sourceXSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    sourceXSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(sourceXSlider);
    sourceXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceX",sourceXSlider);

    sourceYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    sourceYSlider.setColour(juce::Slider::thumbColourId, sourceColour);
    sourceYSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    sourceYSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(sourceYSlider);
    sourceYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceY",sourceYSlider);

    dampingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    dampingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::green);
    dampingSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    dampingSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    addAndMakeVisible(dampingSlider);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"D",dampingSlider);

    addAndMakeVisible(dampingLabel);
    dampingLabel.setJustificationType(juce::Justification::centred);
    dampingLabel.attachToComponent(&dampingSlider,false);

    hfDampingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    hfDampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    hfDampingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::green);
    hfDampingSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    hfDampingSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    addAndMakeVisible(hfDampingSlider);
    hfDampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"HFD",hfDampingSlider);

    addAndMakeVisible(hfDampingLabel);
    hfDampingLabel.setJustificationType(juce::Justification::centred);
    hfDampingLabel.attachToComponent(&hfDampingSlider,false);


    NSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    NSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    NSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    NSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    NSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    addAndMakeVisible(NSlider);
    NSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"N",NSlider);

    addAndMakeVisible(NLabel);
    NLabel.setJustificationType(juce::Justification::centred);
    NLabel.attachToComponent(&NSlider,false);

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

          // #ifdef DEBUG_OUTPUTS
          // if (ix==0 and iy==0)
          // {
          //   cout << "x = " << x << "      y = " << y << endl;
          //   cout << "r = " << r << "      dist = " << dist << endl;
          //   cout << "y-ly = " << y-ly << "         -x+lx = " << -lx+x << endl;
          //   cout << "Theta = " << theta << "        Azimutal index = " << azimutalIndex << endl;
          // }
          // #endif

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
    setSize (500, 500);
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
    auto diagonale = (getLocalBounds().getTopLeft() - getLocalBounds().getBottomRight()).toFloat();
    auto length = diagonale.getDistanceFromOrigin();
    auto perpendicular = diagonale.rotatedAboutOrigin (juce::degreesToRadians (90.0f)) / length;
    auto height = float (getWidth() * getHeight()) / length;

    auto bluegreengrey = juce::Colour::fromFloatRGBA(0.15f,0.20f,0.25f,1.0f);

    juce::ColourGradient grad (bluegreengrey.darker().darker(), perpendicular * height,
                           bluegreengrey, perpendicular * -height, false);
    g.setGradientFill(grad);
    g.fillAll();
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ReverbAudioProcessorEditor::resized()
{
    static const float border = 0.05;
    float uxb = border*getWidth();
    float uyb = border*getHeight();
    auto ux = (1-2*border)*getWidth()/16;
    auto uy = (1-2*border)*getHeight()/16;
    
    roomXSlider.setBounds(uxb,uyb,4*ux,4*uy);
    roomYSlider.setBounds(uxb+4*ux,uyb,4*ux,4*uy);
    dampingSlider.setBounds(uxb+8*ux,uyb,4*ux,4*uy);
    hfDampingSlider.setBounds(uxb+12*ux,uyb,4*ux,4*uy);

    sourceXSlider.setBounds(uxb+ux,uyb+5*uy,9*ux,uy);
    sourceYSlider.setBounds(uxb,uyb+6*uy,ux,9*uy);

    listenerXSlider.setBounds(uxb+ux,uyb+15*uy,9*ux,uy);
    listenerYSlider.setBounds(uxb+10*ux,uyb+6*uy,ux,9*uy);

    xyPad.setBounds(uxb+ux,uyb+6*uy,9*ux,9*uy);

    NSlider.setBounds(uxb+12*ux,uyb+5*uy,4*ux,4*uy);

    addButton.setBounds(uxb+12*ux,uyb+10*uy,4*ux,2*uy);

    // roomYSlider.setBounds(b.removeFromLeft(b.proportionOfWidth(0.33f)).removeFromTop(b.proportionOfHeight(0.25f)));
    // dampingSlider.setBounds(b.removeFromLeft(b.proportionOfWidth(0.5f)).removeFromTop(b.proportionOfHeight(0.25f)));
    // hfDampingSlider.setBounds(b.removeFromTop(b.proportionOfHeight(0.25f)));

    // listenerXSlider.setBounds(b.removeFromLeft(0.1f).rem)

    // roomXSlider.setBounds(getWidth()/6-40,getHeight()/6-40,80,80);
    // roomYSlider.setBounds(3*getWidth()/6-40,getHeight()/6-40,80,80);

    // listenerXSlider.setBounds(getWidth()/6-40,2*getHeight()/6-80,80,80);
    // listenerYSlider.setBounds(3*getWidth()/6-40,2*getHeight()/6-80,80,80);
    
    // sourceXSlider.setBounds(getWidth()/6-40,3*getHeight()/6-80,80,80);
    // sourceYSlider.setBounds(3*getWidth()/6-40,3*getHeight()/6-80,80,80);
    
    // dampingSlider.setBounds(getWidth()/6-80,4*getHeight()/6-80,160,80);
    // hfDampingSlider.setBounds(3*getWidth()/6-80,4*getHeight()/6-80,160,80);

    
    // addButton.setBounds(getWidth()/6-80,6*getHeight()/6-80,160,60);
    // removeButton.setBounds(3*getWidth()/6-80,6*getHeight()/6-80,160,60);

    // xyPad.setBounds(getWidth()/6-80,2*getHeight()/6-40,300,300);

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

void ReverbAudioProcessorEditor::lop(const float* in, float* out, int sampleFreq, float hfDamping, int nRebounds, int order)
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
