cmake_minimum_required (VERSION 3.24.1)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" FORCE)

option (WITH_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
option (WITH_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)

message (STATUS "Sanitizers: ASan=${WITH_ADDRESS_SANITIZER} TSan=${WITH_THREAD_SANITIZER}")
if (WITH_ADDRESS_SANITIZER)
    if (MSVC)
        add_compile_options (/fsanitize=address)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # also enable UndefinedBehaviorSanitizer
        # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
        add_compile_options (-fsanitize=address,undefined -fno-omit-frame-pointer)
        link_libraries (-fsanitize=address)
    endif ()
    message ("Address Sanitizer enabled")
endif ()

if (WITH_THREAD_SANITIZER)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options (-fsanitize=thread -g -fno-omit-frame-pointer)
        link_libraries (-fsanitize=thread)
        message ("Thread Sanitizer enabled")
    endif ()
endif ()

execute_process (
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE AA_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE AA_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (COMMAND uname -n OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process (
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/juce
    OUTPUT_VARIABLE AA_JUCE_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message (
    STATUS
        "AA_GIT_COMMIT_HASH=${AA_GIT_COMMIT_HASH} AA_GIT_TAG=${AA_GIT_TAG} JUCE_GIT_TAG=${AA_JUCE_GIT_TAG} "
)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (PamplejuceVersion)

set (PROJECT_NAME "PolarDesigner")

set (PRODUCT_NAME "PolarDesigner")

set (COMPANY_NAME "Austrian Audio")

set (BUNDLE_ID "audio.austrian.software.plugins.polardesigner")

project (${PROJECT_NAME} VERSION ${CURRENT_VERSION})

include (JUCEDefaults)

add_subdirectory (JUCE)

# Development-specific environment settings:

# Get the hostname of the current machine
execute_process (COMMAND hostname OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)

# Developer workstation - jv's macbook pro
if (HOSTNAME STREQUAL "austrianaudio-jv-mbpro.local")
    set (AAX_SDK_PATH "/Users/jayvaughan/Documents/Development/Plugins/SDKs/aax-sdk-2-8-1/")
    set (ENV{AAX_SDK_PATH} "${AAX_SDK_PATH}")
    set (FORMATS Standalone AU VST3 AUv3)
    message (STATUS "Building on MacOS (Debug) with AAX_SDK_PATH=${AAX_SDK_PATH}")
endif ()

# MacOS VM environment on zosma.intra.austrian.audio
if (HOSTNAME STREQUAL "builder-macos-plugins.local")
    set (AAX_SDK_PATH "/Users/builder/Desktop/Internal/SDKs/aax-sdk-2-8-1/")
    set (MACOSX_DEPLOYMENT_TARGET "10.15")
    set (ENV{AAX_SDK_PATH} "${AAX_SDK_PATH}")
    set (ENV{MACOSX_DEPLOYMENT_TARGET} "${MACOSX_DEPLOYMENT_TARGET}")
    message (STATUS "Building on MacOS (Release) with AAX_SDK_PATH=${AAX_SDK_PATH}")
    message (STATUS "Building on MacOS with MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET}")
endif ()

if (HOSTNAME STREQUAL "builder-win-plugins")
    set (AAX_SDK_PATH
         "c:/Users/builder/Desktop/plugins-builder-desktop/Internal/SDKs/aax-sdk-2-8-1/"
    )
    set (ENV{AAX_SDK_PATH} "${AAX_SDK_PATH}")
    message (STATUS "Building on Windows with AAX_SDK_PATH=${AAX_SDK_PATH}")
endif ()

# If FORMATS has not already been defined (e.g. for Development machine), set it up:
if (NOT DEFINED FORMATS)
    if (DEFINED ENV{AAX_SDK_PATH})
        message (STATUS "AAX_SDK_PATH set to: $ENV{AAX_SDK_PATH}")
        juce_set_aax_sdk_path ($ENV{AAX_SDK_PATH})
        set_directory_properties (
            PROPERTIES JUCE_AAX_COPY_DIR
                       "$ENV{PD_SHARED_BUILD_DIR}/Library/Application Support/Avid/Audio/Plug-Ins"
        )
        set (FORMATS Standalone AU VST3 AUv3 AAX)
    else ()
        message (WARNING "AAX_SDK_PATH not set. Will not build AAX!")
        set (FORMATS Standalone AU VST3 AUv3)
    endif ()
endif ()

set (JUCE_BUILD_LV2 OFF CACHE BOOL "Disable LV2")
set (JUCE_BUILD_ARA OFF CACHE BOOL "Disable ARA")

# Include Melatonin Inspector only for Debug builds
if (CMAKE_BUILD_TYPE MATCHES Debug)
    include (FetchContent)
    FetchContent_Declare (
        melatonin_inspector
        GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
        GIT_TAG origin/main
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_inspector
    )
    FetchContent_MakeAvailable (melatonin_inspector)
endif ()

# Include Melatonin Perfetto only for Release builds
include (FetchContent)
FetchContent_Declare (
    melatonin_perfetto GIT_REPOSITORY https://github.com/sudara/melatonin_perfetto.git
    GIT_TAG origin/main
)
FetchContent_MakeAvailable (melatonin_perfetto)

if (APPLE)
    find_library (ACCELERATE Accelerate)

    if (NOT ACCELERATE)
        message (FATAL_ERROR "Could not find the Accelerate framework!")
    endif ()

else ()
    # fftw settings
    set (BUILD_SHARED_LIBS OFF)
    set (BUILD_TESTS OFF)

    set (ENABLE_FLOAT ON)
    set (FFTW_SINGLE ON)

    set (ENABLE_SSE ON)
    set (ENABLE_SSE2 ON)
    set (CMAKE_POLICY_DEFAULT_CMP077 NEW)

    add_subdirectory (modules/fftw)

    set_target_properties (fftw3f PROPERTIES POSITION_INDEPENDENT_CODE ON)

endif ()

juce_add_plugin (
    "${PROJECT_NAME}"
    ICON_BIG
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"
    COMPANY_NAME
    "${COMPANY_NAME}"
    BUNDLE_ID
    "${BUNDLE_ID}"
    COPY_PLUGIN_AFTER_BUILD
    ON
    PLUGIN_MANUFACTURER_CODE
    OIDA
    PLUGIN_CODE
    AAPD
    FORMATS
    "${FORMATS}"
    PRODUCT_NAME
    "${PRODUCT_NAME}${MAJOR_VERSION}"
)

# HiDPI Awareness on Windows requires the .manifest file
#if(WIN32)
#	set_target_properties("${PROJECT_NAME}" PROPERTIES
#			WIN32_EXECUTABLE TRUE
#			LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\""
#	)
#	# Embed the manifest file
#	target_sources("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.exe.manifest")
#endif()
if (WIN32)
    set_target_properties (
        "${PROJECT_NAME}" PROPERTIES WIN32_EXECUTABLE TRUE VS_DPI_AWARE "PerMonitorV2"
    )
endif ()

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    message (STATUS "Release build detected: will not copy plugins")
    set_target_properties ("${PROJECT_NAME}" PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD OFF)
else ()
    message (STATUS "Debug build detected: will copy plugins for testing")
    set_target_properties ("${PROJECT_NAME}" PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD ON)
endif ()

message (STATUS "Will copy plugins!!")
set_target_properties ("${PROJECT_NAME}" PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD OFF)

add_library (SharedCode INTERFACE)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message ("Building on MacOS (Darwin)")
    set (CMAKE_XCODE_GENERATE_SCHEME "TRUE")
    include (PamplejuceMacOS)
    set (CMAKE_OSX_ARCHITECTURES arm64 x86_64)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum version of the target platform"
                                                   FORCE
    )
    target_link_options (SharedCode INTERFACE -Wl)
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        message ("The minimum macOS version is set to " $CACHE{CMAKE_OSX_DEPLOYMENT_TARGET}.)
    endif ()
else ()
    message ("Building on Windows")
    set (FORMATS VST3)
endif ()

target_compile_features (SharedCode INTERFACE cxx_std_20)

set (SourceFiles Source/PluginEditor.h Source/PluginProcessor.h Source/PluginEditor.cpp
                 Source/PluginProcessor.cpp
)
file (GLOB_RECURSE SourceLookAndFeelFiles
      CONFIGURE_DEPENDS" ${CMAKE_CURRENT_SOURCE_DIR}/resources/lookAndFeel/*.h"
      "${CMAKE_CURRENT_SOURCE_DIR}/resources/lookAndFeel/*.cpp"
)

juce_generate_juce_header (${PROJECT_NAME})

target_sources (SharedCode INTERFACE ${SourceFiles} ${SourceLookAndFeelFiles})

target_include_directories (${PROJECT_NAME} INTERFACE ${juce_generated_headers_directory})

include (Assets)

include (XcodePrettify)

target_compile_definitions (
    SharedCode
    INTERFACE JUCE_WEB_BROWSER=0
              JUCE_USE_CURL=0
              JUCE_VST3_CAN_REPLACE_VST2=0
              JUCE_MODAL_LOOPS_PERMITTED=1
              # JUCE_DISPLAY_SPLASH_SCREEN=0
              CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
              VERSION="${CURRENT_VERSION}"
              AA_BUILD_COMMIT_HASH="${AA_GIT_COMMIT_HASH}"
              AA_BUILD_TAG="${AA_GIT_TAG}"
              AA_BUILD_JUCE_VERSION="${AA_JUCE_GIT_TAG}"
              PRODUCT_NAME_WITHOUT_VERSION="PolarDesigner"
              # For debugging Performance issues:
              $<$<CONFIG:Debug>:PERFETTO=0>
              $<$<CONFIG:Release>:PERFETTO=0>
    # Define USE_MELATONIN_INSPECTOR for Debug builds
    #	$<$<CONFIG:Debug>:USE_MELATONIN_INSPECTOR>
    # For debugging UI issues:
    #	$<$<CONFIG:Debug>:JUCE_ENABLE_REPAINT_DEBUGGING=1>
    # For debugging Leaks
    #	$<$<CONFIG:Debug>:JUCE_CHECK_MEMORY_LEAKS=1>
)

if (APPLE)
    target_compile_definitions (SharedCode INTERFACE JUCE_USE_VDSP_FRAMEWORK=1)
else ()
    target_compile_definitions (SharedCode INTERFACE JUCE_DSP_USE_STATIC_FFTW=1)
endif ()

target_link_libraries (
    SharedCode
    INTERFACE Assets
              juce_audio_basics
              juce_audio_devices
              juce_audio_formats
              juce_audio_processors
              juce_audio_utils
              juce_core
              juce_dsp
              juce_events
              juce_graphics
              juce_gui_basics
              juce_gui_extra
              juce_opengl
              juce::juce_recommended_config_flags
              juce::juce_recommended_lto_flags
              juce::juce_recommended_warning_flags
)

if (APPLE)
    target_link_libraries (SharedCode INTERFACE ${ACCELERATE})
else ()
    target_link_libraries (SharedCode INTERFACE fftw3f)
endif ()

# Link melatonin_inspector only for Debug builds
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries (SharedCode INTERFACE melatonin_inspector)
endif ()

# Link melatonin_perfetto in both Debug and Release builds (disabled in Debug with PERFETTO=0)
target_link_libraries ("${PROJECT_NAME}" PRIVATE Melatonin::Perfetto)

add_custom_target (
    host_header
    DEPENDS $<TARGET_PROPERTY:${PROJECT_NAME},JUCE_GENERATED_SOURCES_DIRECTORY>/JuceHeader.h
)

target_link_libraries ("${PROJECT_NAME}" PRIVATE SharedCode)

if ("AAX" IN_LIST FORMATS)
    message (
        STATUS
            "AAX Build: Adding custom command to copy PageTable file to .aaxplugin Resources/ folder"
    )
    add_custom_command (
        TARGET ${PROJECT_NAME}_AAX
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/extra/ProTools/PolarDesigner3.xml
            $<TARGET_FILE_DIR:${PROJECT_NAME}_AAX>/../Resources/
        COMMENT "Copying PolarDesigner3.xml to the Resources folder for macOS AAX build"
    )
endif ()
