/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BinauRoomRevAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BinauRoomRevAudioProcessorEditor (BinauRoomRevAudioProcessor&);
    ~BinauRoomRevAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BinauRoomRevAudioProcessor& audioProcessor;

    juce::TextButton addButton;
    juce::TextButton removeButton;
    // juce::AudioBuffer<float> buf{1,300};

    juce::Slider roomXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomXSliderAttachment;
    juce::Slider roomYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomYSliderAttachment;
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
    juce::Slider dampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingSliderAttachment;

    int addArrayToBuffer(float *bufPtr, float *hrtfPtr, float gain);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauRoomRevAudioProcessorEditor)
};
