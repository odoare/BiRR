#include "XyPad2.h"

namespace Gui
{
    Thumb::Thumb(int ts)
    {
        thumbSize = ts;
        constrainer.setMinimumOnscreenAmounts(thumbSize,thumbSize,thumbSize,thumbSize);
    }

    void Thumb::paint(juce::Graphics& g)
    {
        g.setColour(thumbColour);
        g.drawEllipse(getLocalBounds().reduced(5).toFloat(),5.f);
    }

    void Thumb::mouseDown(const juce::MouseEvent& event)
    {
        dragger.startDraggingComponent(this, event);
    }

    void Thumb::mouseDrag(const juce::MouseEvent& event)
    {
        dragger.dragComponent(this, event, &constrainer);
        if (moveCallback)
            moveCallback(getPosition().toDouble());
    }

    void Thumb::mouseUp(const juce::MouseEvent& event)
    {
        mouseUpCallback();
    }

    void Thumb::setColour(juce::Colour newColour)
    {
        thumbColour = newColour;
    }

    Head::Head(int ts)
    {
        headSize = ts;
        constrainer.setMinimumOnscreenAmounts(headSize,headSize,headSize,headSize);
    }

    void Head::paint(juce::Graphics& g)
    {
        g.setColour(headColour);
        g.fillEllipse(getLocalBounds().reduced(5).toFloat());
        g.fillEllipse(getLocalBounds().reduced(10,0).toFloat());
    }

    void Head::mouseDown(const juce::MouseEvent& event)
    {
        dragger.startDraggingComponent(this, event);
    }

    void Head::mouseDrag(const juce::MouseEvent& event)
    {
        dragger.dragComponent(this, event, &constrainer);
        if (moveCallback)
            moveCallback(getPosition().toDouble());
    }

    void Head::mouseUp(const juce::MouseEvent& event)
    {
        mouseUpCallback();
    }

    void Head::setColour(juce::Colour newColour)
    {
        headColour = newColour;
    }

    void Head::setOrientation(float o)
    {
        headOrientation = o;
    }

    XyPad2::XyPad2()
    {
        addAndMakeVisible(thumb1);
        addAndMakeVisible(thumb2);
        thumb1.moveCallback = [&](juce::Point<double> position)
        {
            const auto bounds = getLocalBounds().toDouble();
            const auto w = static_cast<double>(thumb1.thumbSize);
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
            const auto w = static_cast<double>(thumb2.thumbSize);
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
        const auto w1 = static_cast<double>(thumb1.thumbSize);
        const auto w2 = static_cast<double>(thumb2.thumbSize);

        thumb1.setBounds(getLocalBounds().withSizeKeepingCentre(thumb1.thumbSize,thumb1.thumbSize));
        thumb2.setBounds(getLocalBounds().withSizeKeepingCentre(thumb2.thumbSize,thumb2.thumbSize));

        thumb1.setTopLeftPosition(
                juce::jmap(x1Sliders[0]->getValue(), x1Sliders[0]->getMinimum(), x1Sliders[0]->getMaximum(), 0.0, bounds.getWidth() - w1),
                juce::jmap(y1Sliders[0]->getValue(), y1Sliders[0]->getMinimum(), y1Sliders[0]->getMaximum(), bounds.getHeight() - w1, 0.0)
            );
        thumb2.setTopLeftPosition(
                juce::jmap(x2Sliders[0]->getValue(), x2Sliders[0]->getMinimum(), x2Sliders[0]->getMaximum(), 0.0, bounds.getWidth() - w2),
                juce::jmap(y2Sliders[0]->getValue(), y2Sliders[0]->getMinimum(), y2Sliders[0]->getMaximum(), bounds.getHeight() - w2, 0.0)
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
        const auto w1 = static_cast<double>(thumb1.thumbSize);
        const auto w2 = static_cast<double>(thumb2.thumbSize);
        if (isX1AxisSlider)
        {
            thumb1.setTopLeftPosition(
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w1),
                thumb1.getY()
            );
        } else if (isY1AxisSlider)
        {
            thumb1.setTopLeftPosition(
                thumb1.getX(),
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w1, 0.0)
            );
        } else if (isX2AxisSlider)
        {
            thumb2.setTopLeftPosition(
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w2),
                thumb2.getY()
            );
        } else
        {
            thumb2.setTopLeftPosition(
                thumb2.getX(),
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w2, 0.0)
            );
        }
        repaint();   
    }

    XyPad2h::XyPad2h()
    {
        addAndMakeVisible(thumb1);
        addAndMakeVisible(head1);
        thumb1.moveCallback = [&](juce::Point<double> position)
        {
            const auto bounds = getLocalBounds().toDouble();
            const auto w = static_cast<double>(thumb1.thumbSize);
            for (auto* slider : x1Sliders)
            {
                slider->setValue(juce::jmap(position.getX(), 0.0, bounds.getWidth()-w, slider->getMinimum(), slider->getMaximum()));
            }
            for (auto* slider : y1Sliders)
            {
                slider->setValue(juce::jmap(position.getY(), bounds.getHeight()-w, 0.0, slider->getMinimum(), slider->getMaximum()));
            }
        };
        head1.moveCallback = [&](juce::Point<double> position)
        {
            const auto bounds = getLocalBounds().toDouble();
            const auto w = static_cast<double>(head1.headSize);
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

    void XyPad2h::paint(juce::Graphics& g)
    {
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
    }

    void XyPad2h::resized()
    {
        const auto bounds = getLocalBounds();
        const auto w1 = static_cast<double>(thumb1.thumbSize);
        const auto w2 = static_cast<double>(head1.headSize);

        thumb1.setBounds(getLocalBounds().withSizeKeepingCentre(thumb1.thumbSize,thumb1.thumbSize));
        head1.setBounds(getLocalBounds().withSizeKeepingCentre(head1.headSize,head1.headSize));

        thumb1.setTopLeftPosition(
                juce::jmap(x1Sliders[0]->getValue(), x1Sliders[0]->getMinimum(), x1Sliders[0]->getMaximum(), 0.0, bounds.getWidth() - w1),
                juce::jmap(y1Sliders[0]->getValue(), y1Sliders[0]->getMinimum(), y1Sliders[0]->getMaximum(), bounds.getHeight() - w1, 0.0)
            );
        head1.setTopLeftPosition(
                juce::jmap(x2Sliders[0]->getValue(), x2Sliders[0]->getMinimum(), x2Sliders[0]->getMaximum(), 0.0, bounds.getWidth() - w2),
                juce::jmap(y2Sliders[0]->getValue(), y2Sliders[0]->getMinimum(), y2Sliders[0]->getMaximum(), bounds.getHeight() - w2, 0.0)
            );
    }

    void XyPad2h::registerSlider(juce::Slider* slider, Axis axis)
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

    void XyPad2h::deregisterSlider(juce::Slider* slider)
    {
        slider->removeListener(this);
        x1Sliders.erase(std::remove(x1Sliders.begin(), x1Sliders.end(), slider), x1Sliders.end());
        x2Sliders.erase(std::remove(x2Sliders.begin(), x2Sliders.end(), slider), x2Sliders.end());
        y1Sliders.erase(std::remove(y1Sliders.begin(), y1Sliders.end(), slider), y1Sliders.end());
        y2Sliders.erase(std::remove(y2Sliders.begin(), y2Sliders.end(), slider), y2Sliders.end());
    }

    void XyPad2h::sliderValueChanged(juce::Slider* slider)
    {
        // Avoid loopback
        if (thumb1.isMouseOverOrDragging(false))
            return;
        if (head1.isMouseOverOrDragging(false))
            return;

        const auto isX1AxisSlider = std::find(x1Sliders.begin(), x1Sliders.end(), slider) != x1Sliders.end();
        const auto isX2AxisSlider = std::find(x2Sliders.begin(), x2Sliders.end(), slider) != x2Sliders.end();
        const auto isY1AxisSlider = std::find(y1Sliders.begin(), y1Sliders.end(), slider) != y1Sliders.end();
        
        const auto bounds = getLocalBounds().toDouble();
        const auto w1 = static_cast<double>(thumb1.thumbSize);
        const auto w2 = static_cast<double>(head1.headSize);
        if (isX1AxisSlider)
        {
            thumb1.setTopLeftPosition(
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w1),
                thumb1.getY()
            );
        } else if (isY1AxisSlider)
        {
            thumb1.setTopLeftPosition(
                thumb1.getX(),
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w1, 0.0)
            );
        } else if (isX2AxisSlider)
        {
            head1.setTopLeftPosition(
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w2),
                head1.getY()
            );
        } else
        {
            head1.setTopLeftPosition(
                head1.getX(),
                juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w2, 0.0)
            );
        }
        repaint();   
    }
}
