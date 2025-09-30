#pragma once

#include <JuceHeader.h>

//==============================================================================
namespace Gui
{
    /** A base class for draggable points on the XY pad. */
    class DraggablePoint : public juce::Component
    {
    public:
        explicit DraggablePoint(int pointSize);
        ~DraggablePoint() override = default;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        void setColour(juce::Colour newColour);

        std::function<void(juce::Point<double>)> moveCallback;
        std::function<void()> mouseUpCallback;
        std::function<void()> mouseDownCallback;

        const int pointSize;

    private:
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;

    protected:
        juce::Colour pointColour;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DraggablePoint);
    };

    //==============================================================================
    /** A simple circular thumb. */
    class Thumb : public DraggablePoint
    {
    public:
        explicit Thumb(int thumbSize);
        void paint(juce::Graphics& g) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb);
    };

    //==============================================================================
    /** A draggable point that represents a head with an orientation. */
    class Head : public DraggablePoint
    {
    public:
        explicit Head(int headSize);

        void paint(juce::Graphics& g) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

        void setOrientation(float newOrientation);
        float getOrientation() const;

        void setShowNose(bool shouldShow);
        bool isShowingNose() const;

    private:
        float headOrientation { 0.0f };
        bool showNose { true };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Head);
    };

    //==============================================================================
    /** A generic XY Pad that can manage multiple draggable points (Thumbs or Heads). */
    class XyPad : public juce::Component, public juce::Slider::Listener
    {
    public:
        enum class Axis { X, Y, O };

        XyPad();
        ~XyPad() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        DraggablePoint* addThumb(int size, juce::Colour colour);
        Head* addHead(int size, juce::Colour colour);
        void removePoint(int pointIndex);
        DraggablePoint* getPoint(int pointIndex);
        int getNumPoints() const;
        juce::Slider* getSliderForPoint (int pointIndex, Axis axis) const;

        struct SliderBinding
        {
            juce::Slider* slider;
            int pointIndex;
            Axis axis;
        };
        const std::vector<SliderBinding>& getSliderBindings() const { return sliderBindings; }

        void registerSlider(juce::Slider* slider, int pointIndex, Axis axis);
        void deregisterSlider(juce::Slider* slider);

    private:
        void sliderValueChanged(juce::Slider* slider) override;

        DraggablePoint* addPoint (DraggablePoint* newPoint, juce::Colour colour);
        juce::OwnedArray<DraggablePoint> points;

        std::vector<SliderBinding> sliderBindings;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XyPad);
    };
}
