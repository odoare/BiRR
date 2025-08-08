/*
  ==============================================================================

    Binaural Room Reverb 3D - PluginEditor.h

    (c) Olivier Doaré, 2022-2025

  ==============================================================================
*/


#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../lib/components/XyPad.h"
#include "../../lib/components/FxmeLookAndFeel.h"
#include "../../lib/components/HorizontalBar.h"
#include "../../lib/components/FxmeLogo.h"
#include "../../lib/assets/defines.h"

//==============================================================================
/**
*/
class ReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ReverbAudioProcessorEditor (ReverbAudioProcessor&);
    ~ReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ReverbAudioProcessor& audioProcessor;

    fxme::FxmeLookAndFeel fxmeLookAndFeel;

    juce::TextButton addButton;
    juce::TextButton removeButton;

    // juce::TextButton exportIrButton;
    fxme::FxmeButton exportIrButton{audioProcessor.apvts,"",FXMECOLOUR};

    std::unique_ptr<juce::FileChooser> myChooser;
    void saveWaveFile();

    fxme::FxmeKnob roomXKnob{audioProcessor.apvts,"Room Size X",ROOMCOLOUR};
    fxme::FxmeKnob roomYKnob{audioProcessor.apvts,"Room Size Y",ROOMCOLOUR};
    fxme::FxmeKnob roomZKnob{audioProcessor.apvts,"Room Size Z",ROOMCOLOUR};
    fxme::FxmeKnob dampingKnob{audioProcessor.apvts,"Damping",DAMPINGCOLOUR};
    fxme::FxmeKnob hfDampingKnob{audioProcessor.apvts,"HF Damping",DAMPINGCOLOUR};
    fxme::FxmeKnob diffusionKnob{audioProcessor.apvts,"Diffusion",DAMPINGCOLOUR};
    fxme::FxmeKnob directLevelKnob{audioProcessor.apvts,"Direct Level",VOLCOLOUR};
    fxme::FxmeKnob reflectionsLevelKnob{audioProcessor.apvts,"Reflections Level",VOLCOLOUR};

    fxme::FxmeKnob listenerOKnob{audioProcessor.apvts,"ListenerO",LISTENERCOLOUR};

    juce::Slider listenerXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> listenerXSliderAttachment;
    juce::Slider listenerYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> listenerYSliderAttachment;
    juce::Slider listenerZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> listenerZSliderAttachment;
    juce::Label listenerZLabel{"listenerZLabel", "Z"};
    
    juce::Slider sourceLXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceLXSliderAttachment;
    juce::Slider sourceLYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceLYSliderAttachment;
    juce::Slider sourceLZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceLZSliderAttachment;
    juce::Label sourceLZLabel{"sourceLZLabel", "Z"};

    juce::Slider sourceRXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceRXSliderAttachment;
    juce::Slider sourceRYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceRYSliderAttachment;
    juce::Slider sourceRZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceRZSliderAttachment;
    juce::Label sourceRZLabel{"sourceRZLabel", "Z"};

    Gui::XyPad3h xyPad3;

    juce::TextButton calculateButton;

    fxme::FxmeButton autoButton{audioProcessor.apvts,"Update",FXMECOLOUR};

    FxmeLogo logo{"", false};

    void addController(juce::Slider&, juce::Slider::SliderStyle, juce::Colour, juce::Colour);
    void addAndConnectLabel(juce::Slider&, juce::Label&);

    FxmeKnobLookAndFeel knobLookAndFeel;

    Gui::HorizontalBar progressBarL{[&]() { return audioProcessor.roomIRL.getProgress(); }};
    Gui::HorizontalBar progressBarR{[&]() { return audioProcessor.roomIRR.getProgress(); }};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioProcessorEditor)
};
