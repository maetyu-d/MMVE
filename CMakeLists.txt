cmake_minimum_required(VERSION 3.22)

project(MMVE VERSION 0.6.0 LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MMVE_JUCE_PATH "" CACHE PATH "Path to a local JUCE checkout")
set(MMVE_PLUGIN_FORMATS "" CACHE STRING "JUCE plugin formats to build")
set(MMVE_LEGACY_JUCE_DIR_HINT "${JUCE_DIR}")

find_package(JUCE CONFIG QUIET)

if (NOT JUCE_FOUND)
    if (MMVE_JUCE_PATH AND EXISTS "${MMVE_JUCE_PATH}/CMakeLists.txt")
        add_subdirectory("${MMVE_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
    elseif (MMVE_LEGACY_JUCE_DIR_HINT AND EXISTS "${MMVE_LEGACY_JUCE_DIR_HINT}/CMakeLists.txt")
        add_subdirectory("${MMVE_LEGACY_JUCE_DIR_HINT}" "${CMAKE_BINARY_DIR}/JUCE")
    elseif (DEFINED ENV{JUCE_DIR} AND EXISTS "$ENV{JUCE_DIR}/CMakeLists.txt")
        add_subdirectory("$ENV{JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
    elseif (EXISTS "${CMAKE_SOURCE_DIR}/JUCE/CMakeLists.txt")
        add_subdirectory("${CMAKE_SOURCE_DIR}/JUCE" "${CMAKE_BINARY_DIR}/JUCE")
    else()
        message(FATAL_ERROR "JUCE not found. Set MMVE_JUCE_PATH or the JUCE_DIR environment variable to a local JUCE checkout.")
    endif()
endif()

if (NOT MMVE_PLUGIN_FORMATS)
    if (WIN32)
        set(MMVE_PLUGIN_FORMATS VST3)
    elseif (APPLE)
        set(MMVE_PLUGIN_FORMATS AU VST3 Standalone)
    else()
        set(MMVE_PLUGIN_FORMATS VST3 Standalone)
    endif()
endif()

juce_add_plugin(MMVE
    COMPANY_NAME "matd.space"
    COMPANY_WEBSITE "https://matd.space"
    COMPANY_EMAIL "hello@matd.space"
    BUNDLE_ID "space.matd.mmve"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VERSION "0.6.0"
    BUILD_VERSION "1.0.6"
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE MatD
    PLUGIN_CODE Mmve
    FORMATS ${MMVE_PLUGIN_FORMATS}
    PRODUCT_NAME "MMVE")

target_sources(MMVE
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(MMVE
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MMVE
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(MMVE)
