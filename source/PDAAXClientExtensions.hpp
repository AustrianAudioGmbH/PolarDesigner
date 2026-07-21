#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace AAExtensions
{
struct PolarDesignerAAXClientExtensions : public juce::AAXClientExtensions
{
    juce::String getPageFileName() const override { return "PolarDesigner3.xml"; }
};
} // namespace AAExtensions
