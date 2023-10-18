cmake_minimum_required(VERSION 3.12)

project(PolarDesigner VERSION 2.1.1)


set(POLARDESIGNER_JUCE_PATH "${CMAKE_SOURCE_DIR}/../tracktion_engine/modules/juce" CACHE STRING "Path to JUCE library source tree (in tracktion_engine module)")

add_subdirectory(../../JUCE JUCE)

juce_set_aax_sdk_path(../SDKs/aax-sdk-2-5-1)
juce_set_vst2_sdk_path(../SDKs/vst-sdk_3.7.3_build-20_2021-08-10)

juce_add_plugin(PolarDesigner
    PLUGIN_MANUFACTURER_CODE OIDA
    PLUGIN_CODE AAPD
    PLUGIN_NAME "PolarDesigner"
    FORMATS VST3 AU Standalone AAX
    PRODUCT_NAME "PolarDesigner"
    COMPANY_NAME "Austrian Audio"
    BUNDLE_ID "audio.austrian.software.plugins.polardesigner"
)


juce_add_binary_data(PolarDesignerData
    SOURCES
        resources/lookAndFeel/BinaryFonts.cpp
        Assets/austrianAudioLogo.svg
        Assets/bandHandleKnob.svg
        Assets/bCardioidPatternIcon.svg
        Assets/cardioidPatternIcon.svg
        Assets/closePresetListIcon.svg
        Assets/diffuseField.svg
        Assets/eightPatternIcon.svg
        Assets/eqFieldCheckSign.svg
        Assets/eyeDrop.svg
        Assets/freeField.svg
        Assets/hyperCardioidPatternIcon.svg
        Assets/loadArrow.svg
        Assets/maximizeTargetIcon.svg
        Assets/maxTargetToSpillIcon.svg
        Assets/omniPatternIcon.svg
        Assets/revBCardioidPatternIcon.svg
        Assets/revCardioidPatternIcon.svg
        Assets/scrollImage.png
        Assets/superCardioidPatternIcon.svg
        Assets/terminateSpillIcon.svg
        Assets/undoPresetIcon.svg
)

juce_generate_juce_header(PolarDesigner)

target_sources(PolarDesigner
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp
        resources/lookAndFeel/BinaryFonts.cpp
)

target_compile_definitions(PolarDesigner
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PolarDesigner_Standalone
    PRIVATE
        PolarDesignerData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
)


target_link_libraries(PolarDesigner
    PRIVATE
        PolarDesignerData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
)

# Set C++ standard to C++20
target_compile_features(PolarDesigner PRIVATE cxx_std_20)

