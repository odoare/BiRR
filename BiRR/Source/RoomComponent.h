#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class RoomComponent  : public juce::Component, public juce::Timer
{
public:
    RoomComponent(ReverbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts);
    ~RoomComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

private:
    struct Point3D { float x, y, z; };
    struct Point2D 
    { 
        float x, y; 
        operator juce::Point<float>() const { return {x, y}; }
    };

    Point2D project(Point3D p);

    ReverbAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    float roomWidth, roomDepth, roomHeight;
    float sourceLX, sourceLY, sourceLZ;
    float sourceRX, sourceRY, sourceRZ;
    float listenerX, listenerY, listenerZ, listenerO;

    float viewAngleX = 0.4f; // rad
    float viewAngleY = 0.6f; // rad

    float cosX, sinX, cosY, sinY;

    juce::Point<int> lastMousePosition;

    enum DraggedObject
    {
        None,
        SourceL,
        SourceR,
        Listener
    } draggedObject { None };

    Point2D projectedSourceL, projectedSourceR, projectedListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomComponent)
};