cmake_minimum_required(VERSION 3.22)
project(MeteoraDrum VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# runtime statico su Windows: il plugin non richiede librerie Visual C++ installate
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(METEORA_FORMATS VST3 AU Standalone)
else()
    set(METEORA_FORMATS VST3 Standalone)
endif()

juce_add_plugin(MeteoraDrum
    COMPANY_NAME             "Meteora Plugins"
    BUNDLE_ID                com.meteoraplugins.meteoradrum
    PLUGIN_MANUFACTURER_CODE Mtra
    PLUGIN_CODE              MtDr
    PRODUCT_NAME             "Meteora Drum"
    FORMATS                  ${METEORA_FORMATS}
    IS_SYNTH                 TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE
    VST3_CATEGORIES          Instrument Drum
    AU_MAIN_TYPE             kAudioUnitType_MusicDevice)

target_sources(MeteoraDrum PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(MeteoraDrum PRIVATE Source)

file(GLOB METEORA_SAMPLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Samples/*.wav")
file(GLOB METEORA_SAMPLES_B CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/SamplesB/*.wav")
set(METEORA_GROOVES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Grooves/grooves.pack")
file(GLOB METEORA_GUI CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/gui/*")
juce_add_binary_data(MeteoraDrumData
    SOURCES
        ${METEORA_SAMPLES}
        ${METEORA_SAMPLES_B}
        ${METEORA_GROOVES}
        ${METEORA_GUI})

target_compile_definitions(MeteoraDrum PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MeteoraDrum
    PRIVATE
        MeteoraDrumData
        juce::juce_audio_utils
        juce::juce_audio_formats
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
