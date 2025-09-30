#include "XyPad.h"

namespace Gui
{
    //==============================================================================
    DraggablePoint::DraggablePoint(int ts) : pointSize(ts)
    {
        constrainer.setMinimumOnscreenAmounts(pointSize, pointSize, pointSize, pointSize);
    }

    void DraggablePoint::mouseDown(const juce::MouseEvent& event)
    {
        dragger.startDraggingComponent(this, event);
        if (mouseDownCallback)
            mouseDownCallback();
    }

    void DraggablePoint::mouseDrag(const juce::MouseEvent& event)
    {
        dragger.dragComponent(this, event, &constrainer);
        if (moveCallback)
            moveCallback(getPosition().toDouble());
    }

    void DraggablePoint::mouseUp(const juce::MouseEvent& event)
    {
        if (mouseUpCallback)
            mouseUpCallback();
    }

    void DraggablePoint::setColour(juce::Colour newColour)
    {
        pointColour = newColour;
        repaint();
    }

    //==============================================================================
    Thumb::Thumb(int thumbSize) : DraggablePoint(thumbSize) {}

    void Thumb::paint(juce::Graphics& g)
    {
        g.setColour(pointColour);
        g.drawEllipse(getLocalBounds().reduced(5).toFloat(), 5.f);
    }

    //==============================================================================
    Head::Head(int headSize) : DraggablePoint(headSize) {}

    void Head::paint(juce::Graphics& g)
    {
        const float thickness = 5.f;

        g.setColour(pointColour);
        g.fillEllipse(getLocalBounds().reduced(5).toFloat());

        if (showNose)
        {
            juce::Path nose;
            juce::Rectangle<float> rect(0.f, 0.f, thickness, .5f * (float)pointSize);
            nose.addRectangle(rect);
            g.fillPath(nose, juce::AffineTransform::translation(-thickness / 2.f, 0.f)
                             .rotated(headOrientation + juce::MathConstants<float>::pi)
                             .translated((float)pointSize / 2.f, (float)pointSize / 2.f));
        }
    }

    void Head::mouseDoubleClick(const juce::MouseEvent& event)
    {
        setOrientation(0.f);
        if (moveCallback)
            moveCallback(getPosition().toDouble());
        repaint();
    }

    void Head::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
    {
        float newOrientation = headOrientation + wheel.deltaY * 0.5f; // Adjust sensitivity
        newOrientation = juce::jlimit(-juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, newOrientation);
        setOrientation(newOrientation);
        if (moveCallback)
            moveCallback(getPosition().toDouble());
    }

    void Head::setOrientation(float newOrientation)
    {
        if (headOrientation != newOrientation)
        {
            headOrientation = newOrientation;
            repaint();
        }
    }

    float Head::getOrientation() const
    {
        return headOrientation;
    }

    void Head::setShowNose(bool shouldShow)
    {
        showNose = shouldShow;
        repaint();
    }

    bool Head::isShowingNose() const
    {
        return showNose;
    }

    //==============================================================================
    namespace
    {
        // Private helper to avoid duplicating the callback logic
        void setupCallbacks (XyPad* pad, DraggablePoint* point)
        {
            const int pointIndex = pad->getNumPoints() - 1;
            point->moveCallback = [pad, pointIndex] (juce::Point<double> position)
            {
                if (auto* p = pad->getPoint (pointIndex))
                {
                    const auto bounds = pad->getLocalBounds().toDouble();
                    const auto w = static_cast<double> (p->pointSize);

                    for (const auto& binding : pad->getSliderBindings())
                    {
                        if (binding.pointIndex == pointIndex)
                        {
                            if (binding.axis == XyPad::Axis::X)
                                binding.slider->setValue (juce::jmap (position.getX(), 0.0, bounds.getWidth() - w, binding.slider->getMinimum(), binding.slider->getMaximum()));
                            else if (binding.axis == XyPad::Axis::Y)
                                binding.slider->setValue (juce::jmap (position.getY(), bounds.getHeight() - w, 0.0, binding.slider->getMinimum(), binding.slider->getMaximum()));
                            else if (binding.axis == XyPad::Axis::O)
                            {
                            if (auto* head = dynamic_cast<Head*> (p))
                                binding.slider->setValue (juce::jmap (static_cast<double>(head->getOrientation()),
                                                               -juce::MathConstants<double>::pi, juce::MathConstants<double>::pi,
                                                               binding.slider->getMinimum(), binding.slider->getMaximum()));
                            }
                        }
                    }
                }
            };
        }
    }

    //==============================================================================
    XyPad::XyPad()
    {
    }

    XyPad::~XyPad()
    {
        for (const auto& binding : sliderBindings)
        {
            if (binding.slider != nullptr)
                binding.slider->removeListener(this);
        }
    }

    void XyPad::paint(juce::Graphics& g)
    {
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
    }

    void XyPad::resized()
    {
        for (int i = 0; i < points.size(); ++i)
        {
            auto* point = points.getUnchecked(i);
            point->setBounds(getLocalBounds().withSizeKeepingCentre(point->pointSize, point->pointSize));

            const auto bounds = getLocalBounds().toDouble();
            const auto w = static_cast<double>(point->pointSize);

            double x = 0.0, y = 0.0;
            bool xSet = false, ySet = false;

            for (const auto& binding : sliderBindings)
            {
                if (binding.pointIndex == i)
                {
                    if (binding.axis == Axis::X && !xSet)
                    {
                        x = juce::jmap(binding.slider->getValue(), binding.slider->getMinimum(), binding.slider->getMaximum(), 0.0, bounds.getWidth() - w);
                        xSet = true;
                    }
                    else if (binding.axis == Axis::Y && !ySet)
                    {
                        y = juce::jmap(binding.slider->getValue(), binding.slider->getMinimum(), binding.slider->getMaximum(), bounds.getHeight() - w, 0.0);
                        ySet = true;
                    }
                }
            }
            point->setTopLeftPosition((int)x, (int)y);
        }
    }

    DraggablePoint* XyPad::addPoint (DraggablePoint* newPoint, juce::Colour colour)
    {
        points.add (newPoint);
        newPoint->setColour (colour);
        addAndMakeVisible (newPoint);
        setupCallbacks (this, newPoint);
        resized();
        return newPoint;
    }

    DraggablePoint* XyPad::addThumb (int size, juce::Colour colour)
    {
        return addPoint (new Thumb (size), colour);
    }

    Head* XyPad::addHead(int size, juce::Colour colour)
    {
        return static_cast<Head*> (addPoint (new Head (size), colour));
    }

    void XyPad::removePoint(int pointIndex)
    {
        if (juce::isPositiveAndBelow(pointIndex, points.size()))
        {
            // First, remove all slider bindings for this point
            sliderBindings.erase(std::remove_if(sliderBindings.begin(), sliderBindings.end(),
                [pointIndex](const SliderBinding& b) { return b.pointIndex == pointIndex; }),
                sliderBindings.end());

            // Then, update indices of subsequent points
            for (auto& binding : sliderBindings)
            {
                if (binding.pointIndex > pointIndex)
                    binding.pointIndex--;
            }

            points.remove(pointIndex);
            resized();
        }
    }

    DraggablePoint* XyPad::getPoint(int pointIndex)
    {
        return points[pointIndex];
    }

    juce::Slider* XyPad::getSliderForPoint (int pointIndex, Axis axis) const
    {
        for (const auto& binding : sliderBindings)
        {
            if (binding.pointIndex == pointIndex && binding.axis == axis)
            {
                return binding.slider;
            }
        }
        return nullptr;
    }

    int XyPad::getNumPoints() const
    {
        return points.size();
    }

    void XyPad::registerSlider(juce::Slider* slider, int pointIndex, Axis axis)
    {
        jassert(isPositiveAndBelow(pointIndex, points.size()));
        slider->addListener(this);
        sliderBindings.push_back({ slider, pointIndex, axis });
    }

    void XyPad::deregisterSlider(juce::Slider* slider)
    {
        slider->removeListener(this);
        sliderBindings.erase(std::remove_if(sliderBindings.begin(), sliderBindings.end(),
            [slider](const SliderBinding& b) { return b.slider == slider; }),
            sliderBindings.end());
    }

    void XyPad::sliderValueChanged(juce::Slider* slider)
    {
        for (const auto& binding : sliderBindings)
        {
            if (binding.slider == slider)
            {
                auto* point = points[binding.pointIndex];
                if (point == nullptr || point->isMouseOverOrDragging(false))
                    return;

                const auto bounds = getLocalBounds().toDouble();
                const auto w = static_cast<double>(point->pointSize);

                switch (binding.axis)
                {
                    case Axis::X:
                        point->setTopLeftPosition(
                            (int)juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w),
                            point->getY());
                        break;
                    case Axis::Y:
                        point->setTopLeftPosition(
                            point->getX(),
                            (int)juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w, 0.0));
                        break;
                    case Axis::O:
                        if (auto* head = dynamic_cast<Head*>(point))
                        {
                            head->setOrientation((float)juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(),
                                                                   -juce::MathConstants<double>::pi, juce::MathConstants<double>::pi));
                        }
                        break;
                }
                repaint();
                return; // Found it, no need to continue
            }
        }
    }
}
