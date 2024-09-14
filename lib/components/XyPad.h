#pragma once

#include <JuceHeader.h>

namespace Gui
{
    class Thumb : public juce::Component
    {
    public:
        Thumb(int ts);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        std::function<void(juce::Point<double>)> moveCallback;
        std::function<void()> mouseUpCallback;
        void setColour(juce::Colour newColour);
        int thumbSize;

    private:
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        juce::Colour thumbColour;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb);
    };

    class Head : public juce::Component
    {
    public:
        Head(int ts);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
        std::function<void(juce::Point<double>)> moveCallback;
        std::function<void()> mouseUpCallback;
        void setColour(juce::Colour newColour);
        void setOrientation(float o);
        int thumbSize;
        float headOrientation {0.f};

    private:
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        juce::Colour headColour;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Head);
    };

    class XyPad2 : public juce::Component, juce::Slider::Listener
    {
    public:
        enum class Axis {X1, Y1, X2, Y2};
        XyPad2();
        void resized() override;
        void paint(juce::Graphics& g) override;
        void registerSlider(juce::Slider* slider, Axis axis);
        void deregisterSlider(juce::Slider* slider);
        Thumb thumb1{25}, thumb2{25};
        juce::Colour coulour1, colour2;

    private:
        void sliderValueChanged(juce::Slider* slider) override;

        std::vector<juce::Slider*> x1Sliders, x2Sliders, y1Sliders, y2Sliders;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XyPad2);
    };

    class XyPad2h : public juce::Component, juce::Slider::Listener
    {
    public:
        enum class Axis {X1, Y1, X2, Y2, O1};

        XyPad2h();
        void resized() override;
        void paint(juce::Graphics& g) override;
        void registerSlider(juce::Slider* slider, Axis axis);
        void deregisterSlider(juce::Slider* slider);
        Head thumb1{30};
        Thumb thumb2{25};
        juce::Colour coulour1, colour2;

    private:
        void sliderValueChanged(juce::Slider* slider) override;

        std::vector<juce::Slider*> x1Sliders, x2Sliders, y1Sliders, y2Sliders, o1Sliders;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XyPad2h);
    };
    
    class XyPad3h : public juce::Component, juce::Slider::Listener
    {
    public:
        enum class Axis {X1, Y1, X2, Y2, X3, Y3, O1};

        XyPad3h();
        void resized() override;
        void paint(juce::Graphics& g) override;
        void registerSlider(juce::Slider* slider, Axis axis);
        void deregisterSlider(juce::Slider* slider);
        Head thumb1{30};
        Thumb thumb2{25};
        Thumb thumb3{25};
        juce::Colour coulour1, colour2, colour3;

    private:
        void sliderValueChanged(juce::Slider* slider) override;

        std::vector<juce::Slider*> x1Sliders, x2Sliders, x3Sliders, y1Sliders, y2Sliders, y3Sliders, o1Sliders;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XyPad3h);
    };
    
}
