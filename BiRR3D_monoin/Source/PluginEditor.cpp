/*
  ==============================================================================

    Binaural Room Reverb 3D, mono input - PluginEditor.cpp

    (c) Olivier Doaré, 2022-2025

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"   

//==============================================================================
ReverbAudioProcessorEditor::ReverbAudioProcessorEditor (ReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    auto startDrag = [this](){      
      audioProcessor.autoUpdate = false;
    };

    auto stopDrag = [this](){   
      audioProcessor.autoUpdate = autoButton.button.getToggleStateValue().getValue();
    };

    // Room size controllers

    addAndMakeVisible(roomXKnob.slider);
    roomXKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    roomXKnob.slider.onDragEnd = stopDrag;
    roomXKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(roomYKnob.slider);
    roomYKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    roomYKnob.slider.onDragEnd = stopDrag;
    roomYKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(roomZKnob.slider);
    roomZKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    roomZKnob.slider.onDragEnd = stopDrag;
    roomZKnob.slider.onDragStart = startDrag;

    // Damping sliders

    addAndMakeVisible(dampingKnob.slider);
    dampingKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    dampingKnob.slider.onDragEnd = stopDrag;
    dampingKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(hfDampingKnob.slider);
    hfDampingKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    hfDampingKnob.slider.onDragEnd = stopDrag;
    hfDampingKnob.slider.onDragStart = startDrag;

    // Width slider (not effective with binaural mode)
    addAndMakeVisible(widthKnob.slider);
    widthKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    widthKnob.slider.onDragEnd = stopDrag;
    widthKnob.slider.onDragStart = startDrag;

    // Level sliders
    addAndMakeVisible(directLevelKnob.slider);
    directLevelKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    directLevelKnob.slider.onDragEnd = stopDrag;
    directLevelKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(reflectionsLevelKnob.slider);
    reflectionsLevelKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    reflectionsLevelKnob.slider.onDragEnd = stopDrag;
    reflectionsLevelKnob.slider.onDragStart = startDrag;

    // Listener position controllers
    addController(listenerXSlider, juce::Slider::SliderStyle::LinearHorizontal, LISTENERCOLOUR, juce::Colours::black);
    listenerXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerX",listenerXSlider);
    listenerXSlider.onDragStart = startDrag;
    listenerXSlider.onDragEnd = stopDrag;

    addController(listenerYSlider, juce::Slider::SliderStyle::LinearVertical, LISTENERCOLOUR, juce::Colours::black);
    listenerYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerY",listenerYSlider);
    listenerYSlider.onDragStart = startDrag;
    listenerYSlider.onDragEnd = stopDrag;

    addController(listenerZSlider, juce::Slider::SliderStyle::LinearVertical, LISTENERCOLOUR, juce::Colours::black);
    listenerZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    listenerZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"ListenerZ",listenerZSlider);
    addAndConnectLabel(listenerZSlider, listenerZLabel);
    listenerZSlider.onDragStart = startDrag;
    listenerZSlider.onDragEnd = stopDrag;

    addAndMakeVisible(listenerOKnob.slider);
    listenerOKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    listenerOKnob.slider.onDragEnd = stopDrag;
    listenerOKnob.slider.onDragStart = startDrag;
    juce::Slider::RotaryParameters par;
    par.startAngleRadians = -juce::MathConstants<float>::pi;
    par.endAngleRadians = juce::MathConstants<float>::pi;
    par.stopAtEnd = true;
    listenerOKnob.slider.setRotaryParameters(par);

    // Source position controllers
    addController(sourceXSlider, juce::Slider::SliderStyle::LinearHorizontal, SOURCECOLOUR, juce::Colours::black);
    sourceXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceX",sourceXSlider);
    sourceXSlider.onDragStart = startDrag;
    sourceXSlider.onDragEnd = stopDrag;

    addController(sourceYSlider, juce::Slider::SliderStyle::LinearVertical, SOURCECOLOUR, juce::Colours::black);
    sourceYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceY",sourceYSlider);
    sourceYSlider.onDragStart = startDrag;
    sourceYSlider.onDragEnd = stopDrag;

    addController(sourceZSlider, juce::Slider::SliderStyle::LinearVertical, SOURCECOLOUR, juce::Colours::black);
    sourceZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceZ",sourceZSlider);
    addAndConnectLabel(sourceZSlider, sourceZLabel);
    sourceZSlider.onDragStart = startDrag;
    sourceZSlider.onDragEnd = stopDrag;

    // Reverb type combo box
    juce::StringArray choices;
    choices.addArray(CHOICES);
    typeComboBox.addItemList(choices,1);
    typeComboBoxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,"Reverb type",typeComboBox);
    typeLabel.attachToComponent(&typeComboBox,false);
    addAndMakeVisible(typeComboBox);
    addAndMakeVisible(typeLabel);

    // XY Pad
    addAndMakeVisible(xyPad2);
    xyPad2.registerSlider(&listenerXSlider, Gui::XyPad2h::Axis::X1);
    xyPad2.registerSlider(&listenerYSlider, Gui::XyPad2h::Axis::Y1);
    xyPad2.registerSlider(&sourceXSlider, Gui::XyPad2h::Axis::X2);
    xyPad2.registerSlider(&sourceYSlider, Gui::XyPad2h::Axis::Y2);
    xyPad2.registerSlider(&listenerOKnob.slider, Gui::XyPad2h::Axis::O1);
    xyPad2.thumb1.setColour(LISTENERCOLOUR);
    xyPad2.thumb2.setColour(SOURCECOLOUR);
    xyPad2.thumb1.mouseUpCallback = stopDrag;
    xyPad2.thumb2.mouseUpCallback = stopDrag;
    xyPad2.thumb1.mouseDownCallback = startDrag;
    xyPad2.thumb2.mouseDownCallback = startDrag;

    // addAndMakeVisible(calculateButton);
    // calculateButton.setButtonText("Update");
    // // We don't use updateFunc here because it has to update even if autoUpdate is false
    // calculateButton.onClick = [this](){      
    //     audioProcessor.setIrLoader();
    //   };

    addAndMakeVisible(autoButton.button);
    autoButton.button.setLookAndFeel(&fxmeLookAndFeel);
    autoButton.button.onClick = stopDrag;

    // Progress bar
    addAndMakeVisible(progressBar);

    addAndMakeVisible(logo);

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
    auto diagonale = (getLocalBounds().getTopLeft() - getLocalBounds().getBottomRight()).toFloat();
    auto length = diagonale.getDistanceFromOrigin();
    auto perpendicular = diagonale.rotatedAboutOrigin (juce::degreesToRadians (90.0f)) / length;
    auto height = float (getWidth() * getHeight()) / length;
    auto bluegreengrey = juce::Colour::fromFloatRGBA (0.15f, 0.15f, 0.25f, 1.0f);
    juce::ColourGradient grad (bluegreengrey.darker().darker().darker(), perpendicular * height,
                           bluegreengrey, perpendicular * -height, false);
    g.setGradientFill(grad);
    g.fillAll();

    // If the IR is being calculated, we disable room size sliders
    // This prevents eventual crashes when increasing room size
    // while calcultating, due to buffer resizing (I've not figured
    // out why yet).
    if (audioProcessor.roomIR.getCalculatingState())
    {
      roomXKnob.slider.setEnabled(false);
      roomYKnob.slider.setEnabled(false);
      roomZKnob.slider.setEnabled(false);
      roomXKnob.slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
      roomYKnob.slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
      roomZKnob.slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
    }
    else
    {
      roomXKnob.slider.setEnabled(true);
      roomYKnob.slider.setEnabled(true);
      roomZKnob.slider.setEnabled(true);
      roomXKnob.slider.setColour(juce::Slider::thumbColourId, FXMECOLOUR);
      roomYKnob.slider.setColour(juce::Slider::thumbColourId, FXMECOLOUR);
      roomZKnob.slider.setColour(juce::Slider::thumbColourId, FXMECOLOUR);
    }
      
}

#define RED reduced(DELTAX*dux,DELTAY*duy)

void ReverbAudioProcessorEditor::resized()
{
    using fi = juce::FlexItem;
    juce::FlexBox fbmain, fb1, fb20, fb2, fb3, fb21,
                  fb2t, fb2b, fb2tl, fb2tc, fb2tr, fb2bl, fb2br, fb31, fb32, fb311, fb3111;

    fbmain.flexDirection = juce::FlexBox::Direction::column;
    fb1.flexDirection = juce::FlexBox::Direction::row;
    fb20.flexDirection = juce::FlexBox::Direction::row;
    fb2.flexDirection = juce::FlexBox::Direction::column;
    fb2t.flexDirection = juce::FlexBox::Direction::row;
    fb2tc.flexDirection = juce::FlexBox::Direction::column;
    fb2b.flexDirection = juce::FlexBox::Direction::row;
    fb3.flexDirection = juce::FlexBox::Direction::row;
    fb21.flexDirection = juce::FlexBox::Direction::row;
    fb21.flexDirection = juce::FlexBox::Direction::row;
    fb21.flexDirection = juce::FlexBox::Direction::row;
    fb31.flexDirection = juce::FlexBox::Direction::column;
    fb32.flexDirection = juce::FlexBox::Direction::column;
    fb311.flexDirection = juce::FlexBox::Direction::row;
    fb3111.flexDirection = juce::FlexBox::Direction::column;

    fb1.items.add(fi(roomXKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(roomYKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(roomZKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(dampingKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(hfDampingKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(widthKnob.flex()).withFlex(1.f));

    fb21.items.add(fi(sourceYSlider).withFlex(0.1f));
    fb21.items.add(fi(xyPad2).withFlex(1.f));
    fb21.items.add(fi(listenerYSlider).withFlex(0.1f));

    fb2.items.add(fi(fb2t).withFlex(0.2f));
    fb2t.items.add(fi(fb2tl).withFlex(0.2f));
    fb2t.items.add(fi(fb2tc).withFlex(1.f));
    fb2t.items.add(fi(fb2tr).withFlex(0.1f));
    fb2tc.items.add(fi(sourceXSlider).withFlex(0.1f));
    fb2.items.add(fi(fb21).withFlex(1.f));
    fb2.items.add(fi(fb2b).withFlex(0.1f));
    fb2b.items.add(fi(fb2bl).withFlex(0.2f));
    fb2b.items.add(fi(listenerXSlider).withFlex(1.f));
    fb2b.items.add(fi(fb2br).withFlex(0.1f));
    
    fb3111.items.add(fi(listenerOKnob.flex()).withFlex(1.f));
    fb311.items.add(fi(fb3111).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,0.f,20.f,0.f)));
    fb311.items.add(fi(sourceZSlider).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));
    fb311.items.add(fi(listenerZSlider).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));
    fb31.items.add(fi(fb311).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,20.f,0.f,0.f)));
    fb31.items.add(fi(typeComboBox).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));

    fb31.items.add(fi(autoButton.flex()).withFlex(0.25f).withMargin(juce::FlexItem::Margin(20.f,20.f,0.f,20.f)));
    fb31.items.add(fi(progressBar).withFlex(0.18f));
    fb32.items.add(fi(directLevelKnob.flex()).withFlex(1.f).withMargin(juce::FlexItem::Margin(20.f,0.f,0.f,0.f)));
    fb32.items.add(fi(reflectionsLevelKnob.flex()).withFlex(1.f).withMargin(juce::FlexItem::Margin(20.f,0.f,0.f,0.f)));
    fb32.items.add(juce::FlexItem(logo).withFlex(0.65f).withMargin(juce::FlexItem::Margin(5.f, 5.f, 5.f, 5.f)).withAlignSelf(juce::FlexItem::AlignSelf::stretch));

    fb3.items.add(fi(fb31).withFlex(1.f).withMargin(juce::FlexItem::Margin(20.f,0.f,0.f,0.f)));
    fb3.items.add(fi(fb32).withFlex(0.5f));

    fb20.items.add(fi(fb2).withFlex(1.f).withMargin(juce::FlexItem::Margin(20.f,10.f,0.f,0.f)));
    fb20.items.add(fi(fb3).withFlex(1.f));

    fbmain.items.add(fi(fb1).withFlex(0.35f));
    fbmain.items.add(fi(fb20).withFlex(1.f));

    fb3111.performLayout(getLocalBounds());
    fb311.performLayout(getLocalBounds());
    fb31.performLayout(getLocalBounds());
    fb32.performLayout(getLocalBounds());
    fb3.performLayout(getLocalBounds());
    fb21.performLayout(getLocalBounds());
    fb2.performLayout(getLocalBounds());
    fb20.performLayout(getLocalBounds());
    fb1.performLayout(getLocalBounds());
    fbmain.performLayout(getLocalBounds());
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
