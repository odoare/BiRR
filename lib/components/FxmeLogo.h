/*
  ==============================================================================

    FxmeLogo.h
    Created: 30 May 2025 8:32:38pm
    Author:  doare

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class FxmeLogo  : public juce::Component
{
public:
    FxmeLogo(juce::String title = "Fxme", bool dtitle = true);
    ~FxmeLogo() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool drawTitle;

private:
    juce::Image logo;
    juce::String titleText;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxmeLogo)
};
