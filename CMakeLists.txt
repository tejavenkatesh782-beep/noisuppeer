cmake_minimum_required(VERSION 3.22)
project(NSOne VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

option(NSONE_COPY_AFTER_BUILD "Copy the VST3 into the system VST3 folder after build" OFF)

juce_add_plugin(NSOne
    COMPANY_NAME "NSOne Audio"
    PLUGIN_MANUFACTURER_CODE Nsoa
    PLUGIN_CODE Nso1
    FORMATS VST3
    PRODUCT_NAME "NS-ONE"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD ${NSONE_COPY_AFTER_BUILD})

target_sources(NSOne PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(NSOne PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(NSOne PRIVATE
    juce::juce_audio_utils
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
