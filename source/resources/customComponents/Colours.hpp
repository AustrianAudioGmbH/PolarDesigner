/*
 ==============================================================================
 Author: Sebastian Grill
 
 Copyright (c) 2025 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
namespace Colours
{
    static const juce::Colour mainBackground = juce::Colour (17, 18, 19);
    static const juce::Colour mainTextColor = juce::Colour (255, 255, 255);
    static const juce::Colour mainTextDisabledColor = mainTextColor.withAlpha (0.4f);
    static const juce::Colour mainTextInactiveColor = mainTextColor.withAlpha (0.7f);
    static const juce::Colour textButtonFrameColor = juce::Colour (52, 54, 57);
    static const juce::Colour labelBackgroundColor = juce::Colour (39, 39, 44);
    static const juce::Colour multiTextButtonBackgroundColor = juce::Colour (31, 32, 38);
    static const juce::Colour groupComponentBackgroundColor = juce::Colour (24, 25, 28);
    static const juce::Colour textButtonDefaultBackgroundColor = juce::Colour (24, 25, 27);
    static const juce::Colour textButtonActiveFrameColor = juce::Colour (255, 255, 255);
    static const juce::Colour textButtonActiveRedFrameColor = juce::Colour (182, 22, 22);
    static const juce::Colour textButtonActiveBlue1FrameColor = juce::Colour (0, 49, 222);
    static const juce::Colour textButtonActiveBlue2FrameColor = juce::Colour (0, 127, 255);
    static const juce::Colour textButtonActiveBlue3FrameColor = juce::Colour (0, 87, 255);
    static const juce::Colour textButtonActiveBlue4FrameColor = juce::Colour (108, 180, 238);
    static const juce::Colour sliderHoverFrameColor = juce::Colour (93, 94, 95);
    static const juce::Colour polarVisualizerRed = juce::Colour (182, 22, 22);
    static const juce::Colour polarVisualizerOrange = juce::Colour (255, 107, 0);
    static const juce::Colour polarVisualizerYellow = juce::Colour (214, 193, 9);
    static const juce::Colour polarVisualizerGreen = juce::Colour (96, 154, 0);
    static const juce::Colour polarVisualizerGreenDark = juce::Colour (0, 157, 25);
    static const juce::Colour polarVisualizerGrid = juce::Colour (52, 54, 57);
    static const juce::Colour trimSliderMainColor = juce::Colour (92, 92, 92);
    static const juce::Colour textButtonHoverBackgroundColor =
        textButtonActiveFrameColor.withAlpha (0.3f);
    static const juce::Colour textButtonHoverRedBackgroundColor =
        textButtonActiveRedFrameColor.withAlpha (0.3f);
    static const juce::Colour textButtonPressedBackgroundColor =
        textButtonActiveFrameColor.withAlpha (0.1f);
    static const juce::Colour textButtonPressedRedBackgroundColor =
        textButtonActiveRedFrameColor.withAlpha (0.1f);
    static const juce::Colour toggleButtonActiveRedBackgroundColor =
        juce::Colour (182, 22, 22).withAlpha (0.7f);

    static const juce::Colour ClFace = juce::Colour (0xFFD8D8D8);
    static const juce::Colour ClFaceShadow = juce::Colour (0XFF272727);
    static const juce::Colour ClFaceShadowOutline = juce::Colour (0xFF212121);
    static const juce::Colour ClRotSliderArrow = juce::Colour (0xFF4A4A4A);
    static const juce::Colour ClRotSliderArrowShadow = juce::Colour (0x445D5D5D);
} // namespace Colours
} // namespace AAGuiComponents
