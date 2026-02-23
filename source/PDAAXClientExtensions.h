#pragma once

#include <juce_audio_processors_headless/juce_audio_processors_headless.h>

namespace AAExtensions
{
class PolarDesignerAAXClientExtensions : public juce::AAXClientExtensions
{
    juce::String getPageFileName() const override { return "PolarDesigner3.xml"; }
};
} // namespace AAExtensions
