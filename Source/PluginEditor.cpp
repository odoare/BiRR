/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"   

//==============================================================================
ReverbAudioProcessorEditor::ReverbAudioProcessorEditor (ReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    auto updateFunc = [this](){audioProcessor.setIrLoader();};

    roomXSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    roomXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    roomXSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::teal);
    roomXSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    roomXSlider.setColour(juce::Slider::thumbColourId, juce::Colours::teal);
    addAndMakeVisible(roomXSlider);
    roomXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomX",roomXSlider);
    roomXSlider.onDragEnd = updateFunc;

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
    roomYSlider.onDragEnd = updateFunc;

    listenerXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    listenerXSlider.setColour(juce::Slider::thumbColourId, listenerColour);
    listenerXSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    listenerXSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(listenerXSlider);
    listenerXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerX",listenerXSlider);
    listenerXSlider.onDragEnd = updateFunc;

    listenerYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    listenerYSlider.setColour(juce::Slider::thumbColourId, listenerColour);
    listenerYSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    listenerYSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(listenerYSlider);
    listenerYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerY",listenerYSlider);
    listenerYSlider.onDragEnd = updateFunc;

    sourceXSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    sourceXSlider.setColour(juce::Slider::thumbColourId, sourceColour);
    sourceXSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    sourceXSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(sourceXSlider);
    sourceXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceX",sourceXSlider);
    sourceXSlider.onDragEnd = updateFunc;

    sourceYSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    sourceYSlider.setColour(juce::Slider::thumbColourId, sourceColour);
    sourceYSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    sourceYSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(sourceYSlider);
    sourceYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceY",sourceYSlider);
    sourceYSlider.onDragEnd = updateFunc;

    dampingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    dampingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::green);
    dampingSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    dampingSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    addAndMakeVisible(dampingSlider);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"D",dampingSlider);
    dampingSlider.onDragEnd = updateFunc;

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
    hfDampingSlider.onDragEnd = updateFunc;

    NSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    NSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    NSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    NSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    NSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    addAndMakeVisible(NSlider);
    NSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"N",NSlider);
    NSlider.onDragEnd = updateFunc;

    addAndMakeVisible(NLabel);
    NLabel.setJustificationType(juce::Justification::centred);
    NLabel.attachToComponent(&NSlider,false);

    addAndMakeVisible(xyPad);
    xyPad.registerSlider(&listenerXSlider, Gui::XyPad::Axis::X1);
    xyPad.registerSlider(&listenerYSlider, Gui::XyPad::Axis::Y1);
    xyPad.registerSlider(&sourceXSlider, Gui::XyPad::Axis::X2);
    xyPad.registerSlider(&sourceYSlider, Gui::XyPad::Axis::Y2);
    xyPad.thumb1.setColour(listenerColour);
    xyPad.thumb2.setColour(sourceColour);
    xyPad.thumb1.mouseUpCallback = updateFunc;
    xyPad.thumb2.mouseUpCallback = updateFunc;

    // addAndMakeVisible(addButton);
    // addButton.setButtonText("Compute IR");
    // addButton.onClick = [this]()
    // {

    //   addButton.setEnabled(false);
    //   removeButton.setEnabled(false);    

    //   audioProcessor.setIrLoader();

    //   addButton.setEnabled(true);
    //   removeButton.setEnabled(true);
    // };

    // addAndMakeVisible(removeButton);
    // removeButton.setButtonText("Reset");
    // removeButton.onClick = [this]()
    // {

    //   addButton.setEnabled(false);
    //   removeButton.setEnabled(false); 
      
    //   juce::AudioBuffer<float> buf;
    //   buf.setSize (1, 10);
      
    //   for (int channel = 0; channel < buf.getNumChannels() ; ++channel)
    //   {
    //       auto* channelData = buf.getWritePointer (channel);
    //       for (int sample=0; sample<buf.getNumSamples(); ++sample)
    //       {
    //           channelData[sample] = 0 ;
    //       }
    //       channelData[0] = 1.0f;
    //   }

    //   audioProcessor.irLoader.loadImpulseResponse (std::move (buf),
    //                       audioProcessor.spec.sampleRate,
    //                       juce::dsp::Convolution::Stereo::yes,
    //                       juce::dsp::Convolution::Trim::no,
    //                       juce::dsp::Convolution::Normalise::no);

    //   addButton.setEnabled(true);
    //   removeButton.setEnabled(true);

    // };

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

}
