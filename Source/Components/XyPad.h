#pragma once

#include <JuceHeader.h>

namespace Gui
{
    class XyPad : public juce::Component, juce::Slider::Listener
    {
    public:
        enum class Axis {X1, Y1, X2, Y2};
        class Thumb : public juce::Component
        {
        public:
            Thumb();
            void paint(juce::Graphics& g) override;
            void mouseDown(const juce::MouseEvent& event) override;
            void mouseDrag(const juce::MouseEvent& event) override;
            void mouseUp(const juce::MouseEvent& event) override;
            std::function<void(juce::Point<double>)> moveCallback;
            std::function<void()> mouseUpCallback;
            void setColour(juce::Colour newColour);
            
        private:
            juce::ComponentDragger dragger;
            juce::ComponentBoundsConstrainer constrainer;
            juce::Colour thumbColour;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb);
        };

        XyPad();
        void resized() override;
        void paint(juce::Graphics& g) override;
        void registerSlider(juce::Slider* slider, Axis axis);
        void deregisterSlider(juce::Slider* slider);
        Thumb thumb1, thumb2;
        juce::Colour coulour1, colour2;

    private:
        void sliderValueChanged(juce::Slider* slider) override;
        static constexpr int thumbSize = 20;

        std::vector<juce::Slider*> x1Sliders, x2Sliders, y1Sliders, y2Sliders;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XyPad);
    };
    
}