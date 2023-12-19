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
    auto updateFunc = [this](){
      calculatingLabel.setVisible(true);
      calculatingLabel.repaint();
      repaint();
      this->repaint();
      audioProcessor.setIrLoader();
      calculatingLabel.setVisible(false);
      };

    // Room size controllers
    addController(roomXSlider, juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Colours::teal,juce::Colours::black);
    addAndConnectLabel(roomXSlider, roomXLabel);
    roomXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomX",roomXSlider);
    roomXSlider.onDragEnd = updateFunc;

    addController(roomYSlider, juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Colours::teal,juce::Colours::black);
    addAndConnectLabel(roomYSlider, roomYLabel);
    roomYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomY",roomYSlider);
    roomYSlider.onDragEnd = updateFunc;

    addController(roomZSlider, juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Colours::teal,juce::Colours::black);
    addAndConnectLabel(roomZSlider, roomZLabel);
    roomZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomZ",roomZSlider);
    roomZSlider.onDragEnd = updateFunc;

    // Listener position controllers
    addController(listenerXSlider, juce::Slider::SliderStyle::LinearHorizontal, listenerColour, juce::Colours::black);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerX",listenerXSlider);
    listenerXSlider.onDragEnd = updateFunc;

    addController(listenerYSlider, juce::Slider::SliderStyle::LinearVertical, listenerColour, juce::Colours::black);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerY",listenerYSlider);
    listenerYSlider.onDragEnd = updateFunc;

    addController(listenerZSlider, juce::Slider::SliderStyle::LinearVertical, listenerColour, juce::Colours::black);
    listenerZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerZ",listenerZSlider);
    listenerZSlider.onDragEnd = updateFunc;

    // Source position controllers
    addController(sourceXSlider, juce::Slider::SliderStyle::LinearHorizontal, sourceColour, juce::Colours::black);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceX",sourceXSlider);
    sourceXSlider.onDragEnd = updateFunc;

    addController(sourceYSlider, juce::Slider::SliderStyle::LinearVertical, sourceColour, juce::Colours::black);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceY",sourceYSlider);
    sourceYSlider.onDragEnd = updateFunc;

    addController(sourceZSlider, juce::Slider::SliderStyle::LinearVertical, sourceColour, juce::Colours::black);
    sourceZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceZ",sourceZSlider);
    sourceZSlider.onDragEnd = updateFunc;


    // Damping sliders
    addController(dampingSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::green,juce::Colours::black);
    addAndConnectLabel(dampingSlider, dampingLabel);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"D",dampingSlider);
    dampingSlider.onDragEnd = updateFunc;

    addController(hfDampingSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::green,juce::Colours::black);
    addAndConnectLabel(hfDampingSlider, hfDampingLabel);
    hfDampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"HFD",hfDampingSlider);
    hfDampingSlider.onDragEnd = updateFunc;

    //addAndMakeVisible(calculatingLabel);
    //calculatingLabel.setVisible(false);
    
    addAndMakeVisible(xyPad2);
    xyPad2.registerSlider(&listenerXSlider, Gui::XyPad2::Axis::X1);
    xyPad2.registerSlider(&listenerYSlider, Gui::XyPad2::Axis::Y1);
    xyPad2.registerSlider(&sourceXSlider, Gui::XyPad2::Axis::X2);
    xyPad2.registerSlider(&sourceYSlider, Gui::XyPad2::Axis::Y2);
    xyPad2.thumb1.setColour(listenerColour);
    xyPad2.thumb2.setColour(sourceColour);
    xyPad2.thumb1.mouseUpCallback = updateFunc;
    xyPad2.thumb2.mouseUpCallback = updateFunc;

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 500);
    setResizable(true,true);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor()
{
  xyPad2.deregisterSlider(&listenerXSlider);
  xyPad2.deregisterSlider(&listenerYSlider);
  xyPad2.deregisterSlider(&sourceXSlider);
  xyPad2.deregisterSlider(&sourceYSlider);
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
    roomZSlider.setBounds(uxb+8*ux,uyb,4*ux,4*uy);

    sourceZSlider.setBounds(uxb+12*ux,uyb+11*uy,2*ux,4*uy);
    listenerZSlider.setBounds(uxb+14*ux,uyb+11*uy,2*ux,4*uy);

    dampingSlider.setBounds(uxb+12*ux,uyb,4*ux,4*uy);
    hfDampingSlider.setBounds(uxb+12*ux,uyb+6*uy,4*ux,4*uy);

    sourceXSlider.setBounds(uxb+ux,uyb+5*uy,9*ux,uy);
    sourceYSlider.setBounds(uxb,uyb+6*uy,ux,9*uy);

    listenerXSlider.setBounds(uxb+ux,uyb+15*uy,9*ux,uy);
    listenerYSlider.setBounds(uxb+10*ux,uyb+6*uy,ux,9*uy);

    xyPad2.setBounds(uxb+ux,uyb+6*uy,9*ux,9*uy);

    calculatingLabel.setBounds(uxb+12*ux,uyb+15*uy,4*ux,uy);

    // diffusionSlider.setBounds(uxb+12*ux,uyb+5*uy,4*ux,4*uy);

    // NSlider.setBounds(uxb+12*ux,uyb+13.5*uy,4*ux,4*uy);

    //addButton.setBounds(uxb+12*ux,uyb+10*uy,4*ux,2*uy);

}

void ReverbAudioProcessorEditor::addController(juce::Slider& slider,
                                                    juce::Slider::SliderStyle style,
                                                    juce::Colour fillCol,
                                                    juce::Colour outlineCol)
{
  slider.setSliderStyle(style);
  slider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
  slider.setColour(juce::Slider::thumbColourId, fillCol);
  slider.setColour(juce::Slider::rotarySliderFillColourId, fillCol);
  slider.setColour(juce::Slider::trackColourId, outlineCol);
  slider.setColour(juce::Slider::backgroundColourId, outlineCol);
  slider.setColour(juce::Slider::rotarySliderOutlineColourId, outlineCol);
  addAndMakeVisible(slider);
}

void ReverbAudioProcessorEditor::addAndConnectLabel(juce::Slider& slider,
                                                juce::Label& label)
{
  addAndMakeVisible(label);
  label.setJustificationType(juce::Justification::centred);
  label.attachToComponent(&slider,false);
}
