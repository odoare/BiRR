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
  logo = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    auto updateFunc = [this](){      
      if (audioProcessor.apvts.getRawParameterValue("Auto update")->load()==true){
        audioProcessor.setIrLoader();
      };
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
    addAndConnectLabel(listenerZSlider, listenerZLabel);

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
    addAndConnectLabel(sourceZSlider, sourceZLabel);

    // Damping sliders
    addController(dampingSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::green,juce::Colours::black);
    addAndConnectLabel(dampingSlider, dampingLabel);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"D",dampingSlider);
    dampingSlider.onDragEnd = updateFunc;

    addController(hfDampingSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::green,juce::Colours::black);
    addAndConnectLabel(hfDampingSlider, hfDampingLabel);
    hfDampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"HFD",hfDampingSlider);
    hfDampingSlider.onDragEnd = updateFunc;

    // Reverb type combo box
    juce::StringArray choices;
    choices.addArray(CHOICES);
    typeComboBox.addItemList(choices,1);
    typeComboBoxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,"Reverb type",typeComboBox);
    typeLabel.attachToComponent(&typeComboBox,false);
    addAndMakeVisible(typeComboBox);
    addAndMakeVisible(typeLabel);
    typeComboBox.onChange = updateFunc;

    // XY Pad
    addAndMakeVisible(xyPad2);
    xyPad2.registerSlider(&listenerXSlider, Gui::XyPad2::Axis::X1);
    xyPad2.registerSlider(&listenerYSlider, Gui::XyPad2::Axis::Y1);
    xyPad2.registerSlider(&sourceXSlider, Gui::XyPad2::Axis::X2);
    xyPad2.registerSlider(&sourceYSlider, Gui::XyPad2::Axis::Y2);
    xyPad2.thumb1.setColour(listenerColour);
    xyPad2.thumb2.setColour(sourceColour);
    xyPad2.thumb1.mouseUpCallback = updateFunc;
    xyPad2.thumb2.mouseUpCallback = updateFunc;

    addAndMakeVisible(calculateButton);
    calculateButton.setButtonText("Calculate");
    // We don't use updateFunc here because it has to update even if autoUpdate is false
    calculateButton.onClick = [this](){      
        audioProcessor.setIrLoader();
      };

    addAndMakeVisible(autoButton);
    autoButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts,"Auto update",autoButton);
    addAndMakeVisible(autoLabel);
    autoLabel.attachToComponent(&autoButton,false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (550, 480);
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
    static const float border = 0.01;
    float uxb = border*getWidth();
    float uyb = border*getHeight();
    auto ux = (1-2*border)*getWidth()/20;
    auto uy = (1-2*border)*getHeight()/18;

    auto diagonale = (getLocalBounds().getTopLeft() - getLocalBounds().getBottomRight()).toFloat();
    auto length = diagonale.getDistanceFromOrigin();
    auto perpendicular = diagonale.rotatedAboutOrigin (juce::degreesToRadians (90.0f)) / length;
    auto height = float (getWidth() * getHeight()) / length;
    auto bluegreengrey = juce::Colour::fromFloatRGBA(0.17f,0.22f,0.27f,1.0f);
    juce::ColourGradient grad (bluegreengrey.darker().darker().darker(), perpendicular * height,
                           bluegreengrey, perpendicular * -height, false);
    g.setGradientFill(grad);
    g.fillAll();

    g.setFont(18);
    g.setColour(listenerColour);
    g.drawSingleLineText("O Listener", uxb+18*ux, uyb+8*uy,juce::Justification::centred);
    g.setColour(sourceColour);
    g.drawSingleLineText("O Source", uxb+18*ux, uyb+7*uy,juce::Justification::centred);

    auto r = juce::Rectangle<float>(uxb+16*ux,uyb+14*uy,4*ux,4*uy);
    g.drawImage(logo, r);

    g.setColour(juce::Colours::grey);
    g.setFont(28);
    g.drawSingleLineText("BiRR3D", uxb+14*ux, uyb+15.5*uy,juce::Justification::centred);
    g.setFont(12);
    g.drawMultiLineText("Binaural Room Reverb 3D", uxb+12*ux, uyb+16*uy, 4*ux, juce::Justification::centred);
    g.setFont(16);
    g.drawMultiLineText("v0.1", uxb+12*ux, uyb+17*uy, 4*ux, juce::Justification::centred);
}

void ReverbAudioProcessorEditor::resized()
{
    static const float border = 0.01;
    float uxb = border*getWidth();
    float uyb = border*getHeight();
    auto ux = (1-2*border)*getWidth()/20;
    auto uy = (1-2*border)*getHeight()/18;
    
    roomXSlider.setBounds(uxb,uyb+uy,4*ux,4*uy);
    roomYSlider.setBounds(uxb+4*ux,uyb+uy,4*ux,4*uy);
    roomZSlider.setBounds(uxb+8*ux,uyb+uy,4*ux,4*uy);

    sourceZSlider.setBounds(uxb+12*ux,uyb+7*uy,2*ux,5*uy);
    listenerZSlider.setBounds(uxb+14*ux,uyb+7*uy,2*ux,5*uy);

    dampingSlider.setBounds(uxb+12*ux,uyb+uy,4*ux,4*uy);
    hfDampingSlider.setBounds(uxb+16*ux,uyb+uy,4*ux,4*uy);

    sourceXSlider.setBounds(uxb+ux,uyb+6*uy,10*ux,uy);
    sourceYSlider.setBounds(uxb,uyb+7*uy,ux,10*uy);

    listenerXSlider.setBounds(uxb+ux,uyb+17*uy,10*ux,uy);
    listenerYSlider.setBounds(uxb+11*ux,uyb+7*uy,ux,10*uy);

    xyPad2.setBounds(uxb+ux,uyb+7*uy,10*ux,10*uy);

    typeComboBox.setBounds(uxb+16*ux,uyb+10*uy,4*ux,uy);

    calculateButton.setBounds(uxb+12*ux,uyb+12.5*uy,3*ux,uy);

    autoButton.setBounds(uxb+15.5*ux,uyb+12.5*uy,4*ux,uy);
    autoLabel.setTopLeftPosition(uxb+16.5*ux,uyb+12.5*uy);
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
