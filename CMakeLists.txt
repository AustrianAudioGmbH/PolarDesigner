cmake_minimum_required (VERSION 3.24.1)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

option (WITH_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
option (WITH_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)

option (INSTALL_AFTER_BUILD "Let JUCE automatically install the plugin after building" ON)

set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS")
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum version of the target platform")

message (STATUS "Sanitizers: ASan=${WITH_ADDRESS_SANITIZER} TSan=${WITH_THREAD_SANITIZER}")
if (WITH_ADDRESS_SANITIZER)
    if (MSVC)
        add_compile_options (/fsanitize=address)
    else ()
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            set (
                sanitizer_flags
                -fno-omit-frame-pointer
                -g
                -fno-sanitize-recover=all
                -fsanitize=address
                -fsanitize=undefined
                -fsanitize=shift
                -fsanitize=shift-exponent
                -fsanitize=shift-base
                -fsanitize=float-divide-by-zero
                -fsanitize=integer-divide-by-zero
                -fsanitize=unreachable
                -fsanitize=vla-bound
                -fsanitize=null
                -fsanitize=return
                -fsanitize=signed-integer-overflow
                -fsanitize=bounds
                -fsanitize=bounds-strict
                -fsanitize=alignment
                -fsanitize=object-size
                -fsanitize=float-cast-overflow
                -fsanitize=nonnull-attribute
                -fsanitize=bool
                -fsanitize=enum
                -fsanitize=vptr
                -fsanitize=pointer-overflow
                -fsanitize=builtin
                -fsanitize=pointer-compare
                -fsanitize=pointer-subtract
                -fsanitize=leak
            )

        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set (
                sanitizer_flags
                -fno-sanitize-recover=all
                -fsanitize=alignment
                -fsanitize=bool
                -fsanitize=bounds
                -fsanitize=enum
                -fsanitize=vptr
                -fsanitize=integer-divide-by-zero
                -fsanitize=float-divide-by-zero
                -fsanitize=float-cast-overflow
                -fsanitize=nonnull-attribute
                -fsanitize=nullability-arg
                -fsanitize=nullability-assign
                -fsanitize=returns-nonnull-attribute
                -fsanitize=nullability-return
                -fsanitize=null
                -fsanitize=shift
                -fsanitize=signed-integer-overflow
                -fsanitize=unreachable
                -fsanitize=vla-bound
                -fsanitize=address
            )

        endif ()

        add_compile_options ("$<$<COMPILE_LANGUAGE:CXX>:${sanitizer_flags}>")
        add_link_options (${sanitizer_flags})
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

# If FORMATS has not already been defined (e.g. for Development machine), set it up:
if (NOT DEFINED FORMATS)
    set (FORMATS Standalone VST3)

    if (DEFINED AAX_SDK_PATH)
        message (STATUS "AAX_SDK_PATH set to: ${AAX_SDK_PATH}")
        juce_set_aax_sdk_path (${AAX_SDK_PATH})
        set_directory_properties (
            PROPERTIES JUCE_AAX_COPY_DIR
                       "$ENV{HOME}/Library/Application Support/Avid/Audio/Plug-Ins"
        )

        set (FORMATS ${FORMATS} AAX)
    else ()
        message (WARNING "AAX_SDK_PATH not set. Will not build AAX!")
    endif ()

    if (APPLE)
        set (FORMATS ${FORMATS} AU AUv3)
    endif ()

endif ()

set (JUCE_BUILD_LV2 OFF CACHE BOOL "Disable LV2")
set (JUCE_BUILD_ARA OFF CACHE BOOL "Disable ARA")

# Include Melatonin Inspector only for Debug builds
include (FetchContent)
FetchContent_Declare (
    melatonin_inspector
    GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
    GIT_TAG origin/main
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_inspector
)
FetchContent_MakeAvailable (melatonin_inspector)

# Include Melatonin Perfetto only for Release builds
# include (FetchContent)
# FetchContent_Declare (
#     melatonin_perfetto GIT_REPOSITORY https://github.com/sudara/melatonin_perfetto.git
#     GIT_TAG origin/main
# )
# FetchContent_MakeAvailable (melatonin_perfetto)

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
    ${INSTALL_AFTER_BUILD}
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

add_library (SharedCode INTERFACE)

if (APPLE)
    message ("Building on MacOS")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        message ("The minimum macOS version is set to " $CACHE{CMAKE_OSX_DEPLOYMENT_TARGET}.)
    endif ()
elseif (UNIX AND NOT APPLE)
    message ("Building on Linux")
elseif (WIN32)
    message ("Building on Windows")
    set (FORMATS VST3)
else ()
    message (FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

target_compile_features (SharedCode INTERFACE cxx_std_20)

file (GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h"
      "${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

target_sources (SharedCode INTERFACE ${SourceFiles})

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
              melatonin_inspector
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

include (Tests)

target_compile_definitions (Tests PRIVATE POLARDESIGNER_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
