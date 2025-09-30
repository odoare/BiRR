#include "RoomComponent.h"
#include "../assets/defines.h"

//==============================================================================
RoomComponent::RoomComponent(juce::AudioProcessorValueTreeState& vts)
    : valueTreeState(vts)
{
    startTimerHz(20);
}

RoomComponent::~RoomComponent()
{
}

void RoomComponent::paint (juce::Graphics& g)
{
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::white);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 1.0f);

    // Pre-calculate trig functions for projection
    cosX = std::cos(viewAngleX);
    sinX = std::sin(viewAngleX);
    cosY = std::cos(viewAngleY);
    sinY = std::sin(viewAngleY);

    // Define the 8 corners of the room in 3D space (0 to 1)
    Point3D corners[8] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };

    // Project 3D corners to 2D
    Point2D projectedCorners[8];
    for (int i = 0; i < 8; ++i)
    {
        projectedCorners[i] = project(corners[i]);
    }

    g.setColour(juce::Colours::grey);

    // Draw the edges of the room
    // Bottom face
    g.drawLine(projectedCorners[0].x, projectedCorners[0].y, projectedCorners[1].x, projectedCorners[1].y);
    g.drawLine(projectedCorners[1].x, projectedCorners[1].y, projectedCorners[2].x, projectedCorners[2].y);
    g.drawLine(projectedCorners[2].x, projectedCorners[2].y, projectedCorners[3].x, projectedCorners[3].y);
    g.drawLine(projectedCorners[3].x, projectedCorners[3].y, projectedCorners[0].x, projectedCorners[0].y);

    // Top face
    g.drawLine(projectedCorners[4].x, projectedCorners[4].y, projectedCorners[5].x, projectedCorners[5].y);
    g.drawLine(projectedCorners[5].x, projectedCorners[5].y, projectedCorners[6].x, projectedCorners[6].y);
    g.drawLine(projectedCorners[6].x, projectedCorners[6].y, projectedCorners[7].x, projectedCorners[7].y);
    g.drawLine(projectedCorners[7].x, projectedCorners[7].y, projectedCorners[4].x, projectedCorners[4].y);

    // Vertical edges
    g.drawLine(projectedCorners[0].x, projectedCorners[0].y, projectedCorners[4].x, projectedCorners[4].y);
    g.drawLine(projectedCorners[1].x, projectedCorners[1].y, projectedCorners[5].x, projectedCorners[5].y);
    g.drawLine(projectedCorners[2].x, projectedCorners[2].y, projectedCorners[6].x, projectedCorners[6].y);
    g.drawLine(projectedCorners[3].x, projectedCorners[3].y, projectedCorners[7].x, projectedCorners[7].y);

    // Project source L and listener positions
    Point3D sourceLPos = { sourceLX, sourceLY, sourceLZ };
    projectedSourceL = project(sourceLPos);

    // Project source R position
    Point3D sourceRPos = { sourceRX, sourceRY, sourceRZ };
    projectedSourceR = project(sourceRPos);

    Point3D listenerPos = { listenerX, listenerY, listenerZ };
    projectedListener = project(listenerPos);

    // Draw source (L)
    g.setColour(SOURCELCOLOUR);
    g.fillEllipse(projectedSourceL.x - 5, projectedSourceL.y - 5, 10, 10);
    g.drawText("L", projectedSourceL.x - 10, projectedSourceL.y - 20, 20, 20, juce::Justification::centred);

    // Draw source (R)
    g.setColour(SOURCERCOLOUR);
    g.fillEllipse(projectedSourceR.x - 5, projectedSourceR.y - 5, 10, 10);
    g.drawText("R", projectedSourceR.x - 10, projectedSourceR.y - 20, 20, 20, juce::Justification::centred);

    // Draw listener
    g.setColour(LISTENERCOLOUR);
    g.fillEllipse(projectedListener.x - 5, projectedListener.y - 5, 10, 10);

    // Draw listener orientation arrow
    const float arrowLength = 0.1f; // Length of arrow in normalized room coordinates
    float listenerO_rad = juce::degreesToRadians(listenerO);

    // Calculate the arrow tip position in 3D space
    // The orientation is a rotation in the XY plane (around the Z axis)
    Point3D arrowTipPos = {
        listenerX - arrowLength * std::sin(listenerO_rad),
        listenerY + arrowLength * std::cos(listenerO_rad),
        listenerZ
    };
    Point2D projectedArrowTip = project(arrowTipPos);
    g.drawLine(projectedListener.x, projectedListener.y, projectedArrowTip.x, projectedArrowTip.y, 2.0f);

    // --- Draw axis indicator in the bottom-left corner that matches room orientation ---
    const float axisLength = 20.0f;
    const float axisOriginX = 25.0f;
    const float axisOriginY = getHeight() - 25.0f;

    g.setColour(juce::Colours::grey);

    // Define axis vectors and project them using the same rotation as the room.
    // The Z-axis is inverted because in graphics, the Y-axis typically points down.
    auto projectAxisVector = [&](float x, float y, float z) -> Point2D
    {
        // Rotate around Y (vertical) axis
        float tempX_rot = x * cosY - y * sinY;
        float tempY_rot = -(x * sinY + y * cosY);
 
        // Rotate around X (horizontal) axis
        float finalY = z * cosX - tempY_rot * sinX;
 
        // Invert final Y for screen coordinates (Y points down)
        // This matches the main projection logic.
        return { tempX_rot, -finalY };
    };

    Point2D pX = projectAxisVector(1.f, 0.f, 0.f); // X-axis (width)
    Point2D pY = projectAxisVector(0.f, 1.f, 0.f); // Y-axis (depth)
    Point2D pZ = projectAxisVector(0.f, 0.f, 1.f); // Z-axis (height)

    g.drawLine(axisOriginX, axisOriginY, axisOriginX + pX.x * axisLength, axisOriginY + pX.y * axisLength, 2.0f);
    g.drawText("X", axisOriginX + pX.x * (axisLength + 8.f) - 10, axisOriginY + pX.y * (axisLength + 8.f) - 10, 20, 20, juce::Justification::centred);

    g.drawLine(axisOriginX, axisOriginY, axisOriginX + pY.x * axisLength, axisOriginY + pY.y * axisLength, 2.0f);
    g.drawText("Y", axisOriginX + pY.x * (axisLength + 8.f) - 10, axisOriginY + pY.y * (axisLength + 8.f) - 10, 20, 20, juce::Justification::centred);

    g.drawLine(axisOriginX, axisOriginY, axisOriginX + pZ.x * axisLength, axisOriginY + pZ.y * axisLength, 2.0f);
    g.drawText("Z", axisOriginX + pZ.x * (axisLength + 8.f) - 10, axisOriginY + pZ.y * (axisLength + 8.f) - 10, 20, 20, juce::Justification::centred);
}

void RoomComponent::mouseDown (const juce::MouseEvent& event)
{
    //if (event.mods.isRightButtonDown()) {
        lastMousePosition = event.getPosition();
        draggedObject = None; // Ensure we aren't dragging an object
    //}
}

void RoomComponent::mouseDrag (const juce::MouseEvent& event)
{
    // if (event.mods.isRightButtonDown())
    // {
        auto currentPosition = event.getPosition();
        auto diff = currentPosition - lastMousePosition;

        // Adjust view angles for room rotation
        const float sensitivity = 0.005f;
        viewAngleY += diff.x * sensitivity;
        viewAngleX += diff.y * sensitivity;

        // Clamp the vertical rotation to prevent flipping over
        viewAngleX = juce::jlimit(-juce::MathConstants<float>::pi / 2.0f + 0.01f, juce::MathConstants<float>::pi / 2.0f - 0.01f, viewAngleX);

        lastMousePosition = currentPosition;
    // }
}

void RoomComponent::mouseUp(const juce::MouseEvent& event)
{
    if (draggedObject != None)
    {
        draggedObject = None;
    }
}

void RoomComponent::resized()
{
}

void RoomComponent::timerCallback()
{
    // Get parameters from APVTS. Note: these are normalized 0-1 values for positions.
    roomWidth = *valueTreeState.getRawParameterValue("Room Size X");
    roomDepth = *valueTreeState.getRawParameterValue("Room Size Y");
    roomHeight = *valueTreeState.getRawParameterValue("Room Size Z");

    // Source and listener positions are percentages of room dimensions
    sourceLX = *valueTreeState.getRawParameterValue("SourceLX");
    sourceLY = *valueTreeState.getRawParameterValue("SourceLY");
    sourceLZ = *valueTreeState.getRawParameterValue("SourceLZ");

    sourceRX = *valueTreeState.getRawParameterValue("SourceRX");
    sourceRY = *valueTreeState.getRawParameterValue("SourceRY");
    sourceRZ = *valueTreeState.getRawParameterValue("SourceRZ");

    listenerX = *valueTreeState.getRawParameterValue("ListenerX");
    listenerY = *valueTreeState.getRawParameterValue("ListenerY");
    listenerZ = *valueTreeState.getRawParameterValue("ListenerZ");
    listenerO = *valueTreeState.getRawParameterValue("ListenerO");

    repaint();
}

RoomComponent::Point2D RoomComponent::project(Point3D p)
{
    // Find the maximum dimension to normalize the room shape, preventing division by zero.
    float maxDim = std::max({ roomWidth, roomDepth, roomHeight, 1.0f });

    // Scale coordinates by aspect ratio to represent the room's true shape
    float x = p.x * (roomWidth / maxDim);
    float y = p.y * (roomDepth / maxDim);
    float z = p.z * (roomHeight / maxDim);

    // Center the object based on its scaled dimensions
    x -= (roomWidth / maxDim) * 0.5f;
    y -= (roomDepth / maxDim) * 0.5f;
    z -= (roomHeight / maxDim) * 0.5f;

    // Simple orthographic projection with rotation
    // Rotate around Y (vertical) axis - Yaw
    float tempX_rot = x * cosY - y * sinY;
    float tempY_rot = -(x * sinY + y * cosY);

    // Rotate around X (horizontal) axis - Pitch
    // We use tempY_rot from the previous rotation.
    float finalY = z * cosX - tempY_rot * sinX;

    // Scale and translate to fit component bounds
    const float scale = juce::jmin(getWidth(), getHeight()) * 0.8f;
    return {
        getWidth() / 2.0f + tempX_rot * scale,
        getHeight() / 2.0f - finalY * scale // Use -finalY to flip Z-axis for screen coordinates
    };
}