/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/XyPad.h"


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
    // juce::AudioBuffer<float> buf{1,300};

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
    juce::Slider sourceXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceXSliderAttachment;
    juce::Slider sourceYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sourceYSliderAttachment;

    juce::Slider NSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> NSliderAttachment;
    juce::Label NLabel{"NLabel", "Number of rebounds"};

    juce::Slider dampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingSliderAttachment;
    juce::Label dampingLabel{"dampingLabel", "Wall absorbtion"};

    juce::Slider hfDampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hfDampingSliderAttachment;
    juce::Label hfDampingLabel{"hfDampingLabel", "HF Wall absorbtion"};

    Gui::XyPad xyPad;

    void addArrayToBuffer(float *bufPtr, float *hrtfPtr, float gain);
    int proximityIndex(const float *data, int length, float value);
    void lop(const float* in, float* out, int sampleFreq, float hfDamping, int nRebounds, int order);

    const juce::Colour listenerColour = juce::Colours::blue;
    const juce::Colour sourceColour = juce::Colours::red;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioProcessorEditor)
};
