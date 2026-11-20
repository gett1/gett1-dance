cmake_minimum_required(VERSION 3.22)
set(gett1_dance_version 0.4.1)
if(APPLE)
    set(gett1_dance_version 0.4.4)
endif()
project(Gett1Dance VERSION ${gett1_dance_version} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GETT1_JUCE_PATH "D:/JUCE" CACHE PATH "JUCE source directory")
option(GETT1_MOTION_TEST_ONLY "Build timing and choreography tests without JUCE" OFF)
include(CTest)
add_executable(Gett1MotionTests Tests/MotionTests.cpp)
target_include_directories(Gett1MotionTests PRIVATE Source)
add_test(NAME Motion COMMAND Gett1MotionTests)
add_executable(Gett1OverlayTests Tests/OverlayTests.cpp)
target_include_directories(Gett1OverlayTests PRIVATE Source)
add_test(NAME OverlayLifetime COMMAND Gett1OverlayTests)
if(GETT1_MOTION_TEST_ONLY)
    return()
endif()
if(NOT EXISTS "${GETT1_JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Set GETT1_JUCE_PATH to your JUCE directory, e.g. D:/JUCE")
endif()
add_subdirectory("${GETT1_JUCE_PATH}" juce)
set(gett1_dance_formats VST3)
if(APPLE)
    list(APPEND gett1_dance_formats AU)
endif()
juce_add_plugin(Gett1Dance
    COMPANY_NAME "gett1"
    BUNDLE_ID "audio.gett1.dance"
    PLUGIN_MANUFACTURER_CODE Gttt
    PLUGIN_CODE Gtdn
    PRODUCT_NAME "gett1 Dance"
    FORMATS ${gett1_dance_formats}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Tools)
juce_add_binary_data(Gett1DanceTypography
    HEADER_NAME DanceTypefaceData.h
    NAMESPACE DanceTypefaceData
    SOURCES Assets/PixelifySans.ttf)
set_target_properties(Gett1DanceTypography PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_sources(Gett1Dance PRIVATE Source/Plugin.cpp Source/Plugin.h Source/Motion.h Source/Meter.h Source/Cat.h Source/OverlayLifetime.h Source/HostForeground.h Source/Theme.h)
if(APPLE)
    enable_language(OBJCXX)
    target_sources(Gett1Dance PRIVATE Source/MacHostWindow.h Source/MacHostWindow.mm)
    set_target_properties(Gett1Dance PROPERTIES OBJCXX_STANDARD 17 OBJCXX_STANDARD_REQUIRED ON)
endif()
target_compile_definitions(Gett1Dance PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Gett1Dance PRIVATE Gett1DanceTypography juce::juce_audio_processors juce::juce_gui_basics
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
