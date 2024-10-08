/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../lib/components/XyPad.h"
#include "../../lib/components/FxmeLookAndFeel.h"
#include "../../lib/components/HorizontalBar.h"

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

    juce::TextButton addButton;
    juce::TextButton removeButton;

    juce::Slider roomXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomXSliderAttachment;
    juce::Label roomXLabel{"roomXLabel", "X Size (m)"};

    juce::Slider roomYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomYSliderAttachment;
    juce::Label roomYLabel{"roomYLabel", "Y Size (m)"};

    juce::Slider listenerXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> listenerXSliderAttachment;
    juce::Slider listenerYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> listenerYSliderAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> listenerOSliderAttachment;
    juce::Slider listenerOSlider;
    juce::Label listenerOLabel{"listenerOLabel", "Head azimuth"};

    juce::Slider sourceLXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceLXSliderAttachment;
    juce::Slider sourceLYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceLYSliderAttachment;

    juce::Slider sourceRXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceRXSliderAttachment;
    juce::Slider sourceRYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceRYSliderAttachment;

    juce::Slider dampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingSliderAttachment;
    juce::Label dampingLabel{"dampingLabel", "Wall absorbtion"};

    juce::Slider hfDampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hfDampingSliderAttachment;
    juce::Label hfDampingLabel{"hfDampingLabel", "HF Wall absorbtion"};

    juce::ComboBox typeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeComboBoxAttachment;
    juce::Label typeLabel{"typeLabel", "Microphones"};
    
    juce::Slider widthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthSliderAttachment;
    juce::Label widthLabel{"widthLabel", "Stereo Width"};

    juce::Slider directLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> directLevelSliderAttachment;
    juce::Label directLevelLabel{"directLevelLabel", "Direct Level"};

    juce::Slider reflectionsLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reflectionsLevelSliderAttachment;
    juce::Label reflectionsLevelLabel{"reflectionsLevelLabel", "Reflections Level"};

    Gui::XyPad3h xyPad3;

    juce::TextButton calculateButton;
    juce::ToggleButton autoButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoButtonAttachment;
    juce::Label autoLabel{"autoLabel", "Update"};
    
    const juce::Colour listenerColour = juce::Colours::darkviolet;
    const juce::Colour sourceLColour = juce::Colours::blue;
    const juce::Colour sourceRColour = juce::Colours::red;

    void addController(juce::Slider&, juce::Slider::SliderStyle, juce::Colour, juce::Colour);
    void addAndConnectLabel(juce::Slider&, juce::Label&);

    FxmeKnobLookAndFeel knobLookAndFeel;

    Gui::HorizontalBar progressBarL{[&]() { return audioProcessor.roomIRL.getProgress(); }};
    Gui::HorizontalBar progressBarR{[&]() { return audioProcessor.roomIRR.getProgress(); }};

    juce::Image logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioProcessorEditor)
};
