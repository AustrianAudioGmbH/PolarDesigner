
#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE ("Plugin instance", "[instance]")
{
    PolarDesignerAudioProcessor testPlugin;

    SECTION ("name")
    {
        CHECK_THAT (testPlugin.getName().toStdString(), Catch::Matchers::Equals ("PolarDesigner3"));
    }
}
