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
    logo = juce::ImageCache::getFromMemory(BinaryData::logo686_png, BinaryData::logo686_pngSize);

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
    roomXSlider.setLookAndFeel(&knobLookAndFeel);

    addController(roomYSlider, juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Colours::teal,juce::Colours::black);
    addAndConnectLabel(roomYSlider, roomYLabel);
    roomYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomY",roomYSlider);
    roomYSlider.onDragEnd = updateFunc;
    roomYSlider.setLookAndFeel(&knobLookAndFeel);

    addController(roomZSlider, juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Colours::teal,juce::Colours::black);
    addAndConnectLabel(roomZSlider, roomZLabel);
    roomZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"RoomZ",roomZSlider);
    roomZSlider.onDragEnd = updateFunc;
    roomZSlider.setLookAndFeel(&knobLookAndFeel);

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

    addController(listenerOSlider, juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, listenerColour, juce::Colours::black);
    listenerOSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerOSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerO",listenerOSlider);
    listenerOSlider.onDragEnd = updateFunc;
    addAndConnectLabel(listenerOSlider, listenerOLabel);
    juce::Slider::RotaryParameters par;
    par.startAngleRadians = -juce::MathConstants<float>::pi;
    par.endAngleRadians = juce::MathConstants<float>::pi;
    par.stopAtEnd = true;
    listenerOSlider.setRotaryParameters(par);
    listenerOSlider.setLookAndFeel(&knobLookAndFeel);

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
    dampingSlider.setLookAndFeel(&knobLookAndFeel);

    addController(hfDampingSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::green,juce::Colours::black);
    addAndConnectLabel(hfDampingSlider, hfDampingLabel);
    hfDampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"HFD",hfDampingSlider);
    hfDampingSlider.onDragEnd = updateFunc;
    hfDampingSlider.setLookAndFeel(&knobLookAndFeel);

    addController(widthSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::orange,juce::Colours::black);
    addAndConnectLabel(widthSlider, widthLabel);
    widthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"Stereo Width",widthSlider);
    widthSlider.onDragEnd = updateFunc;
    widthSlider.setLookAndFeel(&knobLookAndFeel);

    addController(directLevelSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::orange,juce::Colours::black);
    addAndConnectLabel(directLevelSlider, directLevelLabel);
    directLevelSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"Direct Level",directLevelSlider);
    directLevelSlider.onDragEnd = updateFunc;
    directLevelSlider.setLookAndFeel(&knobLookAndFeel);

    addController(reflectionsLevelSlider, juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Colours::orange,juce::Colours::black);
    addAndConnectLabel(reflectionsLevelSlider, reflectionsLevelLabel);
    reflectionsLevelSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"Reflections Level",reflectionsLevelSlider);
    reflectionsLevelSlider.onDragEnd = updateFunc;
    reflectionsLevelSlider.setLookAndFeel(&knobLookAndFeel);

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
    xyPad2.registerSlider(&listenerXSlider, Gui::XyPad2h::Axis::X1);
    xyPad2.registerSlider(&listenerYSlider, Gui::XyPad2h::Axis::Y1);
    xyPad2.registerSlider(&sourceXSlider, Gui::XyPad2h::Axis::X2);
    xyPad2.registerSlider(&sourceYSlider, Gui::XyPad2h::Axis::Y2);
    xyPad2.registerSlider(&listenerOSlider, Gui::XyPad2h::Axis::O1);
    xyPad2.thumb1.setColour(listenerColour);
    xyPad2.thumb2.setColour(sourceColour);
    xyPad2.thumb1.mouseUpCallback = updateFunc;
    xyPad2.thumb2.mouseUpCallback = updateFunc;

    addAndMakeVisible(calculateButton);
    calculateButton.setButtonText("Update");
    // We don't use updateFunc here because it has to update even if autoUpdate is false
    calculateButton.onClick = [this](){      
        audioProcessor.setIrLoader();
      };

    addAndMakeVisible(autoButton);
    autoButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts,"Auto update",autoButton);
    addAndMakeVisible(autoLabel);
    autoLabel.attachToComponent(&autoButton,false);

    // Progress bar
    progressBar.setColour(juce::Colours::green);
    addAndMakeVisible(progressBar);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (650, 480);
    setResizable(true,true);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor()
{
  xyPad2.deregisterSlider(&listenerXSlider);
  xyPad2.deregisterSlider(&listenerYSlider);
  xyPad2.deregisterSlider(&sourceXSlider);
  xyPad2.deregisterSlider(&sourceYSlider);
}


#define NX 24
#define NY 18
#define DELTAX 0.1f
#define DELTAY 0.1f
#define BORDER 0.01f

//==============================================================================
void ReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    float uxb = BORDER*getWidth();
    float uyb = BORDER*getHeight();
    auto ux = (1-2*BORDER)*getWidth()/NX;
    auto uy = (1-2*BORDER)*getHeight()/NY;

    auto diagonale = (getLocalBounds().getTopLeft() - getLocalBounds().getBottomRight()).toFloat();
    auto length = diagonale.getDistanceFromOrigin();
    auto perpendicular = diagonale.rotatedAboutOrigin (juce::degreesToRadians (90.0f)) / length;
    auto height = float (getWidth() * getHeight()) / length;
    // auto bluegreengrey = juce::Colour::fromFloatRGBA(0.17f,0.22f,0.27f,1.0f);
    auto bluegreengrey = juce::Colour::fromFloatRGBA (0.15f, 0.15f, 0.25f, 1.0f);
    juce::ColourGradient grad (bluegreengrey.darker().darker().darker(), perpendicular * height,
                           bluegreengrey, perpendicular * -height, false);
    g.setGradientFill(grad);
    g.fillAll();

    g.setFont(18);
    g.setColour(listenerColour);
    g.drawSingleLineText("O Listener", uxb+18*ux, uyb+8*uy,juce::Justification::centred);
    g.setColour(sourceColour);
    g.drawSingleLineText("O Source", uxb+18*ux, uyb+7*uy,juce::Justification::centred);

    auto r = juce::Rectangle<float>(uxb+20.5*ux,uyb+15*uy,3*ux,3*ux);
    g.drawImage(logo, r);

    // g.setColour(juce::Colours::grey);
    // g.setFont(28);
    // g.drawSingleLineText("BiRR3D", uxb+14*ux, uyb+15.5*uy,juce::Justification::centred);
    // g.setFont(12);
    // g.drawMultiLineText("Binaural Room Reverb 3D", uxb+12*ux, uyb+16*uy, 4*ux, juce::Justification::centred);
    // g.setFont(16);
    // g.drawMultiLineText("v0.1", uxb+12*ux, uyb+17*uy, 4*ux, juce::Justification::centred);
}

#define RED reduced(DELTAX*dux,DELTAY*duy)

void ReverbAudioProcessorEditor::resized()
{
    float uxb = BORDER*getWidth();
    float uyb =BORDER*getHeight();
    auto ux = (1-2*BORDER)*getWidth()/24;
    auto uy = (1-2*BORDER)*getHeight()/18;
    auto dux=4*ux;
    auto duy=4*uy;
    
    roomXSlider.setBounds(juce::Rectangle<int>(uxb,uyb+uy,dux,duy).RED);
    roomYSlider.setBounds(juce::Rectangle<int>(uxb+4*ux,uyb+uy,dux,duy).RED);
    roomZSlider.setBounds(juce::Rectangle<int>(uxb+8*ux,uyb+uy,dux,duy).RED);

    sourceZSlider.setBounds(uxb+12*ux,uyb+7*uy,2*ux,5*uy);
    listenerZSlider.setBounds(uxb+14*ux,uyb+7*uy,2*ux,5*uy);

    dampingSlider.setBounds(juce::Rectangle<int>(uxb+12*ux,uyb+uy,dux,duy).RED);
    hfDampingSlider.setBounds(juce::Rectangle<int>(uxb+16*ux,uyb+uy,dux,duy).RED);

    sourceXSlider.setBounds(uxb+ux,uyb+6*uy,10*ux,uy);
    sourceYSlider.setBounds(uxb,uyb+7*uy,ux,10*uy);

    listenerXSlider.setBounds(uxb+ux,uyb+17*uy,10*ux,uy);
    listenerYSlider.setBounds(uxb+11*ux,uyb+7*uy,ux,10*uy);
    listenerOSlider.setBounds(uxb+16.5*ux,uyb+9.5*uy,3*ux,3*uy);

    widthSlider.setBounds(juce::Rectangle<int>(uxb+20*ux,uyb+11*uy,dux,duy).RED);

    directLevelSlider.setBounds(juce::Rectangle<int>(uxb+20*ux,uyb+uy,dux,duy).RED);
    reflectionsLevelSlider.setBounds(juce::Rectangle<int>(uxb+20*ux,uyb+6*uy,dux,duy).RED);

    xyPad2.setBounds(uxb+ux,uyb+7*uy,10*ux,10*uy);

    typeComboBox.setBounds(uxb+12*ux,uyb+13.5*uy,7*ux,uy);

    calculateButton.setBounds(uxb+12*ux,uyb+15.5*uy,3*ux,uy);

    autoButton.setBounds(uxb+15.5*ux,uyb+15.5*uy,4*ux,uy);
    autoLabel.setTopLeftPosition(uxb+16.3*ux,uyb+15.5*uy);

    progressBar.setBounds(uxb+12*ux,uyb+16.5*uy,8*ux,uy);
}

void ReverbAudioProcessorEditor::addController(juce::Slider& slider,
                                                    juce::Slider::SliderStyle style,
                                                    juce::Colour fillCol,
                                                    juce::Colour outlineCol)
{
  slider.setSliderStyle(style);
  slider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,100,20);
  slider.setTextBoxIsEditable(true);
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
