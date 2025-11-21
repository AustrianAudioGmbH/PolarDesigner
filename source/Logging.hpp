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

#include <juce_core/juce_core.h>

#ifdef DEBUG
    #define LOG_ERROR(message) juce::Logger::writeToLog ("ERROR: " + juce::String (message))
    #define LOG_DEBUG(message) juce::Logger::writeToLog ("DEBUG: " + juce::String (message))
    #define LOG_WARN(message) juce::Logger::writeToLog ("WARNING: " + juce::String (message))
#else
    #define LOG_ERROR(message)
    #define LOG_DEBUG(message)
    #define LOG_WARN(message)
#endif
