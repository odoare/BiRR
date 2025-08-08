/*
  ==============================================================================

    Binaural Room Reverb 3D - PluginEditor.cpp

    (c) Olivier Doaré, 2022-2025

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"   

//==============================================================================
ReverbAudioProcessorEditor::ReverbAudioProcessorEditor (ReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // logo = juce::ImageCache::getFromMemory(BinaryData::logo686_png, BinaryData::logo686_pngSize);

    auto startDrag = [this](){      
      audioProcessor.autoUpdate = false;
    };

    auto stopDrag = [this](){   
      audioProcessor.autoUpdate = autoButton.button.getToggleStateValue().getValue();
    };

    auto exportFile = [this](){   
      saveWaveFile();
      autoButton.button.setState(juce::Button::ButtonState::buttonNormal);
    };

    addAndMakeVisible (exportIrButton.button);
    exportIrButton.button.setLookAndFeel(&fxmeLookAndFeel);
    exportIrButton.button.setButtonText("Export IR");
    exportIrButton.button.onClick = exportFile;
    autoButton.button.setState(juce::Button::ButtonState::buttonNormal);

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

    // Left source position controllers
    addController(sourceLXSlider, juce::Slider::SliderStyle::LinearHorizontal, SOURCELCOLOUR, juce::Colours::black);
    sourceLXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceLXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceLX",sourceLXSlider);
    sourceLXSlider.onDragStart = startDrag;
    sourceLXSlider.onDragEnd = stopDrag;

    addController(sourceLYSlider, juce::Slider::SliderStyle::LinearVertical, SOURCELCOLOUR, juce::Colours::black);
    sourceLYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceLYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceLY",sourceLYSlider);
    sourceLYSlider.onDragStart = startDrag;
    sourceLYSlider.onDragEnd = stopDrag;

    addController(sourceLZSlider, juce::Slider::SliderStyle::LinearVertical, SOURCELCOLOUR, juce::Colours::black);
    sourceLZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceLZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceLZ",sourceLZSlider);
    addAndConnectLabel(sourceLZSlider, sourceLZLabel);
    sourceLZSlider.onDragStart = startDrag;
    sourceLZSlider.onDragEnd = stopDrag;

    // Right source position controllers
    addController(sourceRXSlider, juce::Slider::SliderStyle::LinearHorizontal, SOURCERCOLOUR, juce::Colours::black);
    sourceRXSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceRXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceRX",sourceRXSlider);
    sourceRXSlider.onDragStart = startDrag;
    sourceRXSlider.onDragEnd = stopDrag;

    addController(sourceRYSlider, juce::Slider::SliderStyle::LinearVertical, SOURCERCOLOUR, juce::Colours::black);
    sourceRYSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceRYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceRY",sourceRYSlider);
    sourceRYSlider.onDragStart = startDrag;
    sourceRYSlider.onDragEnd = stopDrag;

    addController(sourceRZSlider, juce::Slider::SliderStyle::LinearVertical, SOURCERCOLOUR, juce::Colours::black);
    sourceRZSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    sourceRZSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SourceRZ",sourceRZSlider);
    sourceRZSlider.onDragStart = startDrag;
    sourceRZSlider.onDragEnd = stopDrag;
    addAndConnectLabel(sourceRZSlider, sourceRZLabel);

    // Reverb type combo box
    juce::StringArray choices;
    choices.addArray(CHOICES);
    typeComboBox.addItemList(choices,1);
    typeComboBoxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,"Reverb type",typeComboBox);
    typeLabel.attachToComponent(&typeComboBox,false);
    addAndMakeVisible(typeComboBox);
    addAndMakeVisible(typeLabel);

    // XY Pad
    addAndMakeVisible(xyPad3);
    xyPad3.registerSlider(&listenerXSlider, Gui::XyPad3h::Axis::X1);
    xyPad3.registerSlider(&listenerYSlider, Gui::XyPad3h::Axis::Y1);
    xyPad3.registerSlider(&sourceLXSlider, Gui::XyPad3h::Axis::X2);
    xyPad3.registerSlider(&sourceLYSlider, Gui::XyPad3h::Axis::Y2);
    xyPad3.registerSlider(&sourceRXSlider, Gui::XyPad3h::Axis::X3);
    xyPad3.registerSlider(&sourceRYSlider, Gui::XyPad3h::Axis::Y3);
    xyPad3.registerSlider(&listenerOKnob.slider, Gui::XyPad3h::Axis::O1);
    xyPad3.thumb1.setColour(LISTENERCOLOUR);
    xyPad3.thumb2.setColour(SOURCELCOLOUR);
    xyPad3.thumb3.setColour(SOURCERCOLOUR);
    xyPad3.thumb1.mouseUpCallback = stopDrag;
    xyPad3.thumb2.mouseUpCallback = stopDrag;
    xyPad3.thumb3.mouseUpCallback = stopDrag;
    xyPad3.thumb1.mouseDownCallback = startDrag;
    xyPad3.thumb2.mouseDownCallback = startDrag;
    xyPad3.thumb3.mouseDownCallback = startDrag;

    addAndMakeVisible(autoButton.button);
    autoButton.button.setLookAndFeel(&fxmeLookAndFeel);
    autoButton.button.onClick = stopDrag;

    // Progress bar
    addAndMakeVisible(progressBarL);
    addAndMakeVisible(progressBarR);

    addAndMakeVisible(logo);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (650, 480);
    setResizable(true,true);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor()
{
  xyPad3.deregisterSlider(&listenerXSlider);
  xyPad3.deregisterSlider(&listenerYSlider);
  xyPad3.deregisterSlider(&sourceLXSlider);
  xyPad3.deregisterSlider(&sourceLYSlider);
  xyPad3.deregisterSlider(&sourceRXSlider);
  xyPad3.deregisterSlider(&sourceRYSlider);
}

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
    if (audioProcessor.roomIRL.getCalculatingState() || audioProcessor.roomIRR.getCalculatingState() )
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
                  fb2t, fb2b, fb2tl, fb2tc, fb2tr, fb2bl, fb2br, fb31, fb32, fb311, fb312, fb3111;

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
    fb312.flexDirection = juce::FlexBox::Direction::row;
    fb3111.flexDirection = juce::FlexBox::Direction::column;

    fb1.items.add(fi(roomXKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(roomYKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(roomZKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(dampingKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(hfDampingKnob.flex()).withFlex(1.f));
    fb1.items.add(fi(widthKnob.flex()).withFlex(1.f));

    fb21.items.add(fi(sourceLYSlider).withFlex(0.1f));
    fb21.items.add(fi(sourceRYSlider).withFlex(0.1f));
    fb21.items.add(fi(xyPad3).withFlex(1.f));
    fb21.items.add(fi(listenerYSlider).withFlex(0.1f));

    fb2.items.add(fi(fb2t).withFlex(0.2f));
    fb2t.items.add(fi(fb2tl).withFlex(0.2f));
    fb2t.items.add(fi(fb2tc).withFlex(1.f));
    fb2t.items.add(fi(fb2tr).withFlex(0.1f));
    fb2tc.items.add(fi(sourceRXSlider).withFlex(0.1f));
    fb2tc.items.add(fi(sourceLXSlider).withFlex(0.1f));
    fb2.items.add(fi(fb21).withFlex(1.f));
    fb2.items.add(fi(fb2b).withFlex(0.1f));
    fb2b.items.add(fi(fb2bl).withFlex(0.2f));
    fb2b.items.add(fi(listenerXSlider).withFlex(1.f));
    fb2b.items.add(fi(fb2br).withFlex(0.1f));
    
    fb3111.items.add(fi(listenerOKnob.flex()).withFlex(1.f));
    fb311.items.add(fi(fb3111).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,0.f,20.f,0.f)));
    fb311.items.add(fi(sourceLZSlider).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));
    fb311.items.add(fi(sourceRZSlider).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));
    fb311.items.add(fi(listenerZSlider).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));
    fb31.items.add(fi(fb311).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,20.f,0.f,0.f)));
    fb31.items.add(fi(typeComboBox).withFlex(0.2f).withMargin(juce::FlexItem::Margin(25.f,0.f,0.f,0.f)));

    fb312.items.add(fi(autoButton.flex()).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,0.f,0.f,0.f)));
    fb312.items.add(fi(exportIrButton.flex()).withFlex(0.75f).withMargin(juce::FlexItem::Margin(0.f,0.f,0.f,0.f)));

    fb31.items.add(fi(fb312).withFlex(0.25f).withMargin(juce::FlexItem::Margin(20.f,20.f,0.f,20.f)));
    fb31.items.add(fi(progressBarL).withFlex(0.18f));
    fb31.items.add(fi(progressBarR).withFlex(0.18f));
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

void ReverbAudioProcessorEditor::saveWaveFile()
{
    myChooser = std::make_unique<juce::FileChooser> ("Please select the wav you want to save...",
                                            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                                            "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::saveMode;
    myChooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& chooser)
    {
      std::cout << "In launchAsync..." << std::endl;
      juce::File wavFile (chooser.getResult()); 
      std::cout << "You choosed 💾 " << wavFile.getFullPathName() << std::endl;
      std::cout << "File name without extension: " << wavFile.getFileNameWithoutExtension() << std::endl;
      std::cout << "File extension: " << wavFile.getFileExtension() << std::endl;
      std::cout << "Parent directory: " << wavFile.getParentDirectory().getFullPathName() << std::endl;
      
      auto fname = wavFile.getParentDirectory().getFullPathName()
                    + "/"
                    + wavFile.getFileNameWithoutExtension();
      wavFile = juce::File(fname+ "_L" + ".wav") ;
      audioProcessor.roomIRL.exportIrToWav(wavFile);
      wavFile = juce::File(fname+ "_R" + ".wav") ;
      audioProcessor.roomIRR.exportIrToWav(wavFile);
    });
}
