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
    : AudioProcessorEditor (&p), audioProcessor (p), roomComponent(p.apvts)
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
    addAndMakeVisible (autoButton.button);
    autoButton.button.setLookAndFeel(&fxmeLookAndFeel);
    autoButton.button.setState(juce::Button::ButtonState::buttonNormal);

    // Room size controllers

    addAndMakeVisible(roomXKnob);
    roomXKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    roomXKnob.slider.onDragEnd = stopDrag;
    roomXKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(roomYKnob);
    roomYKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    roomYKnob.slider.onDragEnd = stopDrag;
    roomYKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(roomZKnob);
    roomZKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    roomZKnob.slider.onDragEnd = stopDrag;
    roomZKnob.slider.onDragStart = startDrag;

    // Damping sliders

    addAndMakeVisible(dampingKnob);
    dampingKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    dampingKnob.slider.onDragEnd = stopDrag;
    dampingKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(hfDampingKnob);
    hfDampingKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    hfDampingKnob.slider.onDragEnd = stopDrag;
    hfDampingKnob.slider.onDragStart = startDrag;

    // Width slider (not effective with binaural mode)
    addAndMakeVisible(widthKnob);
    widthKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    widthKnob.slider.onDragEnd = stopDrag;
    widthKnob.slider.onDragStart = startDrag;

    // Level sliders
    addAndMakeVisible(directLevelKnob);
    directLevelKnob.slider.setLookAndFeel(&fxmeLookAndFeel);
    directLevelKnob.slider.onDragEnd = stopDrag;
    directLevelKnob.slider.onDragStart = startDrag;

    addAndMakeVisible(reflectionsLevelKnob);
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

    addAndMakeVisible(listenerOKnob);
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
    typeLabel.attachToComponent(&typeComboBox,true);
    addAndMakeVisible(typeComboBox);
    addAndMakeVisible(typeLabel);

    // Dimension combo box
    juce::StringArray dimensionChoices;
    dimensionChoices.addArray(DIMENSIONS);
    dimensionComboBox.addItemList(dimensionChoices, 1);
    dimensionComboBoxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "Dimension", dimensionComboBox);
    dimensionLabel.attachToComponent(&dimensionComboBox, true);
    addAndMakeVisible(dimensionComboBox);

    // XY Pad
    addAndMakeVisible(xyPad);
    auto* listenerHeadXY = xyPad.addHead(30, LISTENERCOLOUR);
    auto* sourceLThumbXY = xyPad.addThumb(30, SOURCELCOLOUR);
    auto* sourceRThumbXY = xyPad.addThumb(30, SOURCERCOLOUR);

    xyPad.registerSlider(&listenerXSlider, 0, Gui::XyPad::Axis::X);
    xyPad.registerSlider(&listenerYSlider, 0, Gui::XyPad::Axis::Y);
    xyPad.registerSlider(&listenerOKnob.slider, 0, Gui::XyPad::Axis::O);
    xyPad.registerSlider(&sourceLXSlider, 1, Gui::XyPad::Axis::X);
    xyPad.registerSlider(&sourceLYSlider, 1, Gui::XyPad::Axis::Y);
    xyPad.registerSlider(&sourceRXSlider, 2, Gui::XyPad::Axis::X);
    xyPad.registerSlider(&sourceRYSlider, 2, Gui::XyPad::Axis::Y);

    listenerHeadXY->mouseUpCallback = stopDrag;
    sourceLThumbXY->mouseUpCallback = stopDrag;
    sourceRThumbXY->mouseUpCallback = stopDrag;
    listenerHeadXY->mouseDownCallback = startDrag;
    sourceLThumbXY->mouseDownCallback = startDrag;
    sourceRThumbXY->mouseDownCallback = startDrag;
    listenerHeadXY->setShowNose(true);

    // XZ Pad
    addAndMakeVisible(xzPad);
    auto* listenerHeadXZ = xzPad.addHead(30, LISTENERCOLOUR);
    auto* sourceLThumbXZ = xzPad.addThumb(30, SOURCELCOLOUR);
    auto* sourceRThumbXZ = xzPad.addThumb(30, SOURCERCOLOUR);

    xzPad.registerSlider(&listenerXSlider, 0, Gui::XyPad::Axis::X);
    xzPad.registerSlider(&listenerZSlider, 0, Gui::XyPad::Axis::Y);
    xzPad.registerSlider(&sourceLXSlider, 1, Gui::XyPad::Axis::X);
    xzPad.registerSlider(&sourceLZSlider, 1, Gui::XyPad::Axis::Y);
    xzPad.registerSlider(&sourceRXSlider, 2, Gui::XyPad::Axis::X);
    xzPad.registerSlider(&sourceRZSlider, 2, Gui::XyPad::Axis::Y);

    listenerHeadXZ->mouseUpCallback = stopDrag;
    sourceLThumbXZ->mouseUpCallback = stopDrag;
    sourceRThumbXZ->mouseUpCallback = stopDrag;
    listenerHeadXZ->mouseDownCallback = startDrag;
    sourceLThumbXZ->mouseDownCallback = startDrag;
    sourceRThumbXZ->mouseDownCallback = startDrag;
    listenerHeadXZ->setShowNose(false);

    addAndMakeVisible(xPadLabel);
    xPadLabel.setText("X", juce::dontSendNotification);
    xPadLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(yPadLabel);
    yPadLabel.setText("Y", juce::dontSendNotification);
    yPadLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(zPadLabel);
    zPadLabel.setText("Z", juce::dontSendNotification);
    zPadLabel.setJustificationType(juce::Justification::centred);


    // Progress bar
    addAndMakeVisible(progressBarL);
    addAndMakeVisible(progressBarR);

    addAndMakeVisible(roomComponent);

    addAndMakeVisible(logo);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (640, 600);
    setResizable(true,true);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor()
{
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
    
    // If dimension==2, we don't need Z components
    int dim = audioProcessor.apvts.getRawParameterValue("Dimension")->load();
    if (dim == 0)
    {
      listenerZSlider.setEnabled(false);
      listenerZSlider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
      roomZKnob.slider.setEnabled(false);
      roomZKnob.slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
      xzPad.setEnabled(false);
      xzPadIsColourSet = false; // Mark colors as unset
      for (int i = 0; i < xzPad.getNumPoints(); ++i)
      {
          if (auto* point = xzPad.getPoint(i))
              point->setColour(juce::Colours::darkgrey);
      }
    }
    else
    {
      // Only enable roomZKnob if dim is 1 (3D)
      // This logic is already present further down, so we don't duplicate it here.
      listenerZSlider.setEnabled(true);
      listenerZSlider.setColour(juce::Slider::thumbColourId, LISTENERCOLOUR);
      xzPad.setEnabled(true);
    }

    // Restore thumb colours when re-enabling xzPad
    if (dim == 1 && !xzPadIsColourSet)
        setXzPadThumbColours();

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
      roomXKnob.slider.setColour(juce::Slider::thumbColourId, FXMECOLOUR);
      roomYKnob.slider.setColour(juce::Slider::thumbColourId, FXMECOLOUR);
      
      //std::cout << "Dim :" << dim << std::endl;
      // We run on room Z knob only if 3D
      if (dim==1)
      {
        roomZKnob.slider.setColour(juce::Slider::thumbColourId, FXMECOLOUR);
        roomZKnob.slider.setEnabled(true);
      }
    }
    if (audioProcessor.apvts.getRawParameterValue("Reverb type")->load() == 3)
    {
      widthKnob.slider.setEnabled(false);
      widthKnob.slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
    }
    else
    {
      widthKnob.slider.setEnabled(true);
      widthKnob.slider.setColour(juce::Slider::thumbColourId, DAMPINGCOLOUR);
    }
}

void ReverbAudioProcessorEditor::resized()
{
    using fi = juce::FlexItem;
    juce::FlexBox fb1, fb2;

    fb1.flexDirection = juce::FlexBox::Direction::column;
    fb2.flexDirection = juce::FlexBox::Direction::row;

    juce::FlexBox padsBox;
    padsBox.flexDirection = juce::FlexBox::Direction::column;
    juce::FlexBox xyPadBox, xzPadBox;
    xyPadBox.flexDirection = juce::FlexBox::Direction::row;
    xzPadBox.flexDirection = juce::FlexBox::Direction::row;
    xyPadBox.items.add(fi(yPadLabel).withFlex(0.1f));
    xyPadBox.items.add(fi(xyPad).withFlex(1.0f));
    xzPadBox.items.add(fi(zPadLabel).withFlex(0.1f));
    xzPadBox.items.add(fi(xzPad).withFlex(1.0f));
    padsBox.items.add(fi(xyPadBox).withFlex(1.f).withMargin(juce::FlexItem::Margin(10.f,10.f,10.f,0.f)));
    padsBox.items.add(fi(xzPadBox).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,10.f,0.f,0.f)));
    padsBox.items.add(fi(xPadLabel).withFlex(.1f));

    padsBox.items.add(fi(progressBarL).withFlex(0.113f).withMargin(juce::FlexItem::Margin(-5.f,0.f,-5.f,0.f)));
    padsBox.items.add(fi(progressBarR).withFlex(0.11f).withMargin(juce::FlexItem::Margin(-5.f,0.f,-5.f,0.f)));    

    juce::FlexBox fbRoom;
    fbRoom.flexDirection = juce::FlexBox::Direction::row;
    fbRoom.items.add(fi(roomXKnob).withFlex(1.f));
    fbRoom.items.add(fi(roomYKnob).withFlex(1.f));
    fbRoom.items.add(fi(roomZKnob).withFlex(1.f));
    
    juce::FlexBox fbDamp;
    fbDamp.flexDirection = juce::FlexBox::Direction::row;
    fbDamp.items.add(fi(dampingKnob).withFlex(1.f));
    fbDamp.items.add(fi(hfDampingKnob).withFlex(1.f));
    fbDamp.items.add(fi(widthKnob).withFlex(1.f));
    
    juce::FlexBox fbKn;
    fbKn.flexDirection = juce::FlexBox::Direction::row;
    fbKn.items.add(fi(listenerOKnob).withFlex(1.f));
    fbKn.items.add(fi(directLevelKnob).withFlex(1.f));
    fbKn.items.add(fi(reflectionsLevelKnob).withFlex(1.f));

    juce::FlexBox fbButs;
    fbButs.flexDirection = juce::FlexBox::Direction::row;
    fbButs.items.add(fi(typeComboBox).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,0.f,0.f,40.f)));
    fbButs.items.add(fi(dimensionComboBox).withFlex(0.5f).withMargin(juce::FlexItem::Margin(0.f,10.f,0.f,40.f)));

    juce::FlexBox fbBottom;
    fbBottom.flexDirection = juce::FlexBox::Direction::row;
    fbBottom.items.add(fi(roomComponent).withFlex(1.f).withMargin(juce::FlexItem::Margin(10.f,10.f,10.f,10.f)));

    juce::FlexBox fbButs2;
    fbButs2.flexDirection = juce::FlexBox::Direction::column;
    fbButs2.items.add(fi(autoButton.flex()).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,5.f,0.f,5.f)));
    fbButs2.items.add(fi(exportIrButton.flex()).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f,5.f,0.f,5.f)));
    fbButs2.items.add(juce::FlexItem(logo).withFlex(1.f).withMargin(juce::FlexItem::Margin(5.f, 5.f, 5.f, 5.f)).withAlignSelf(juce::FlexItem::AlignSelf::stretch));

    fbBottom.items.add(fi(fbButs2).withFlex(0.4f).withMargin(juce::FlexItem::Margin(0.f,0.f,0.f,0.f)));

    fb1.items.add(fi(fbBottom).withFlex(0.8f).withMargin(juce::FlexItem::Margin(0.f,0.f,0.f,0.f)));
    fb1.items.add(fi(fbButs).withFlex(0.1f).withMargin(juce::FlexItem::Margin(0.f,0.f,10.f,0.f)));
    fb1.items.add(fi(fbRoom).withFlex(0.5f).withMargin(juce::FlexItem::Margin(0.f,0.f,10.f,0.f)));
    fb1.items.add(fi(fbDamp).withFlex(0.5f).withMargin(juce::FlexItem::Margin(0.f,0.f,10.f,0.f)));
    fb1.items.add(fi(fbKn).withFlex(0.5f).withMargin(juce::FlexItem::Margin(0.f,0.f,10.f,0.f)));

    fb2.items.add(fi(padsBox).withFlex(.8f));
    fb2.items.add(fi(fb1).withFlex(1.f));
    fb2.performLayout(getLocalBounds());
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
