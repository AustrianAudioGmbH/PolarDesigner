#pragma once

#include <juce_core/juce_core.h>

#ifdef DEBUG
    #define LOG_ERROR(message) Logger::writeToLog ("ERROR: " + juce::String (message))
    #define LOG_DEBUG(message) Logger::writeToLog ("DEBUG: " + juce::String (message))
    #define LOG_WARN(message) Logger::writeToLog ("WARNING: " + juce::String (message))
#else
    #define LOG_ERROR(message)
    #define LOG_DEBUG(message)
    #define LOG_WARN(message)
#endif
