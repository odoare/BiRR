/*
  ==============================================================================

    VerticalMeter.h
    Created: 29 Nov 2023 4:38:30pm
    Author:  doare

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <iostream>

namespace Gui
{
  class HorizontalBar : public juce::Component, public juce::Timer
  {
  public:
    HorizontalBar(std::function<float()>&& valueFunction) : valueSupplier(std::move(valueFunction))
    {
      startTimerHz(5);
      barColour=juce::Colours::green;
    }

    void paint(juce::Graphics& g) override
    {
        const auto level = valueSupplier();

        const auto bounds = getLocalBounds().reduced(10).toFloat();
        const auto h = bounds.getWidth();
        const auto newWidth = juce::jmap(level,0.f, 1.0f, 0.0f, h);
        const auto x = bounds.getTopLeft().getX();
        auto bounds2 = bounds;
        bounds2.setRight(newWidth);

        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds,5.0f);

        if (juce::approximatelyEqual(level,1.f))
            g.setColour(juce::Colours::green);
        else
            g.setColour(juce::Colours::red);
        g.fillRoundedRectangle(bounds2,5.0f);
    }

    void timerCallback() override
    {
      repaint();
    }

    void setColour(juce::Colour newColour)
    {
      barColour = newColour;
    }

  private:
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
    juce::Colour barColour;

  };
}
