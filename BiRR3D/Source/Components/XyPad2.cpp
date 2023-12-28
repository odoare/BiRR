#include "XyPad2.h"

namespace Gui
{
    XyPad2::Thumb::Thumb()
    {
        constrainer.setMinimumOnscreenAmounts(thumbSize,thumbSize,thumbSize,thumbSize);
    }

    void XyPad2::Thumb::paint(juce::Graphics& g)
    {
        g.setColour(thumbColour);
        g.drawEllipse(getLocalBounds().reduced(5).toFloat(),5.f);
    }

    void XyPad2::Thumb::mouseDown(const juce::MouseEvent& event)
    {
        dragger.startDraggingComponent(this, event);
    }

    void XyPad2::Thumb::mouseDrag(const juce::MouseEvent& event)
    {
        dragger.dragComponent(this, event, &constrainer);
        if (moveCallback)
            moveCallback(getPosition().toDouble());
    }

    void XyPad2::Thumb::mouseUp(const juce::MouseEvent& event)
    {
        mouseUpCallback();
    }

    void XyPad2::Thumb::setColour(juce::Colour newColour)
    {
        thumbColour = newColour;
    }

    XyPad2::XyPad2()
    {
        addAndMakeVisible(thumb1);
        addAndMakeVisible(thumb2);
        thumb1.moveCallback = [&](juce::Point<double> position)
        {
            const auto bounds = getLocalBounds().toDouble();
            const auto w = static_cast<double>(thumbSize);
            for (auto* slider : x1Sliders)
            {
                slider->setValue(juce::jmap(position.getX(), 0.0, bounds.getWidth()-w, slider->getMinimum(), slider->getMaximum()));
            }
            for (auto* slider : y1Sliders)
            {
                slider->setValue(juce::jmap(position.getY(), bounds.getHeight()-w, 0.0, slider->getMinimum(), slider->getMaximum()));
            }
        };
        thumb2.moveCallback = [&](juce::Point<double> position)
        {
            const auto bounds = getLocalBounds().toDouble();
            const auto w = static_cast<double>(thumbSize);
            for (auto* slider : x2Sliders)
            {
                slider->setValue(juce::jmap(position.getX(), 0.0, bounds.getWidth()-w, slider->getMinimum(), slider->getMaximum()));

            }
            for (auto* slider : y2Sliders)
            {
                slider->setValue(juce::jmap(position.getY(), bounds.getHeight()-w, 0.0, slider->getMinimum(), slider->getMaximum()));
            }
        };
    }

    void XyPad2::paint(juce::Graphics& g)
    {
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
    }

    void XyPad2::resized()
    {
        const auto bounds = getLocalBounds();
        const auto w = static_cast<double>(thumbSize);

        thumb1.setBounds(getLocalBounds().withSizeKeepingCentre(thumbSize,thumbSize));
        thumb2.setBounds(getLocalBounds().withSizeKeepingCentre(thumbSize,thumbSize));

        thumb1.setTopLeftPosition(
                juce::jmap(x1Sliders[0]->getValue(), x1Sliders[0]->getMinimum(), x1Sliders[0]->getMaximum(), 0.0, bounds.getWidth() - w),
                juce::jmap(y1Sliders[0]->getValue(), y1Sliders[0]->getMinimum(), y1Sliders[0]->getMaximum(), bounds.getHeight() - w, 0.0)
            );
        thumb2.setTopLeftPosition(
                juce::jmap(x2Sliders[0]->getValue(), x2Sliders[0]->getMinimum(), x2Sliders[0]->getMaximum(), 0.0, bounds.getWidth() - w),
                juce::jmap(y2Sliders[0]->getValue(), y2Sliders[0]->getMinimum(), y2Sliders[0]->getMaximum(), bounds.getHeight() - w, 0.0)
            );
    }

    void XyPad2::registerSlider(juce::Slider* slider, Axis axis)
    {
        slider->addListener(this);
        if (axis == Axis::X1)
            x1Sliders.push_back(slider);
        if (axis == Axis::X2)
            x2Sliders.push_back(slider);
        if (axis == Axis::Y1)
            y1Sliders.push_back(slider);
        if (axis == Axis::Y2)
            y2Sliders.push_back(slider);
    }

    void XyPad2::deregisterSlider(juce::Slider* slider)
    {
        slider->removeListener(this);
        x1Sliders.erase(std::remove(x1Sliders.begin(), x1Sliders.end(), slider), x1Sliders.end());
        x2Sliders.erase(std::remove(x2Sliders.begin(), x2Sliders.end(), slider), x2Sliders.end());
        y1Sliders.erase(std::remove(y1Sliders.begin(), y1Sliders.end(), slider), y1Sliders.end());
        y2Sliders.erase(std::remove(y2Sliders.begin(), y2Sliders.end(), slider), y2Sliders.end());
    }

    void XyPad2::sliderValueChanged(juce::Slider* slider)
    {
        // Avoid loopback
        if (thumb1.isMouseOverOrDragging(false))
            return;
        if (thumb2.isMouseOverOrDragging(false))
            return;

        const auto isX1AxisSlider = std::find(x1Sliders.begin(), x1Sliders.end(), slider) != x1Sliders.end();
        const auto isX2AxisSlider = std::find(x2Sliders.begin(), x2Sliders.end(), slider) != x2Sliders.end();
        const auto isY1AxisSlider = std::find(y1Sliders.begin(), y1Sliders.end(), slider) != y1Sliders.end();
        
        const auto bounds = getLocalBounds().toDouble();
        const auto w = static_cast<double>(thumbSize);
        if (isX1AxisSlider)
        {
            thumb1.setTopLeftPosition(
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w),
                thumb1.getY()
            );
        } else if (isY1AxisSlider)
        {
            thumb1.setTopLeftPosition(
                thumb1.getX(),
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w, 0.0)
            );
        } else if (isX2AxisSlider)
        {
            thumb2.setTopLeftPosition(
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w),
                thumb2.getY()
            );
        } else
        {
            thumb2.setTopLeftPosition(
                thumb2.getX(),
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w, 0.0)
            );
        }
        repaint();   
    }
}
