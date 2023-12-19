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

    roomYSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    roomYSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    roomYSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::teal);
    roomYSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    roomYSlider.setColour(juce::Slider::thumbColourId, juce::Colours::teal);
    addAndMakeVisible(roomYSlider);
    roomYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomY",roomYSlider);
    roomYSlider.onDragEnd = updateFunc;

    addAndMakeVisible(roomYLabel);
    roomYLabel.setJustificationType(juce::Justification::centred);
    roomYLabel.attachToComponent(&roomYSlider,false);

    roomZSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    roomZSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    roomZSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::teal);
    roomZSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    roomZSlider.setColour(juce::Slider::thumbColourId, juce::Colours::teal);
    addAndMakeVisible(roomZSlider);
    roomZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomZ",roomZSlider);
    roomZSlider.onDragEnd = updateFunc;

    addAndMakeVisible(roomZLabel);
    roomZLabel.setJustificationType(juce::Justification::centred);
    roomZLabel.attachToComponent(&roomZSlider,false);

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

    listenerZSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    listenerZSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    listenerZSlider.setColour(juce::Slider::thumbColourId, listenerColour);
    listenerZSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    listenerZSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    listenerZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(listenerZSlider);
    listenerZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerZ",listenerZSlider);
    listenerZSlider.onDragEnd = updateFunc;

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

    sourceZSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    sourceZSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    sourceZSlider.setColour(juce::Slider::thumbColourId, sourceColour);
    sourceZSlider.setColour(juce::Slider::trackColourId, juce::Colours::black);
    sourceZSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    sourceZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(sourceZSlider);
    sourceZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceZ",sourceZSlider);
    sourceZSlider.onDragEnd = updateFunc;

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

    addAndMakeVisible(calculatingLabel);
    //calculatingLabel.setVisible(false);
    
    // NSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    // NSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    // NSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    // NSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    // NSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    // addAndMakeVisible(NSlider);
    // NSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"N",NSlider);
    // NSlider.onDragEnd = updateFunc;

    // addAndMakeVisible(NLabel);
    // NLabel.setJustificationType(juce::Justification::centred);
    // NLabel.attachToComponent(&NSlider,false);

    // diffusionSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    // diffusionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
    // diffusionSlider.setColour(juce::Slider::thumbColourId, juce::Colours::green);
    // diffusionSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    // diffusionSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    // addAndMakeVisible(diffusionSlider);
    // diffusionSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"Diffusion",diffusionSlider);

    // addAndMakeVisible(diffusionLabel);
    // diffusionLabel.setJustificationType(juce::Justification::centred);
    // diffusionLabel.attachToComponent(&diffusionSlider,false);
    // diffusionSlider.onDragEnd = updateFunc;

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
