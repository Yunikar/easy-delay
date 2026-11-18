cmake_minimum_required(VERSION 3.22)

project(EasyDelay VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE and CMake are fetched into third_party/ by tools/fetch-deps.ps1.
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/juce-src/JUCE-master")

if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE was not found in third_party/.\n"
        "Run:  powershell -File tools/fetch-deps.ps1\n"
        "then re-run CMake.")
endif()

add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_plugin(EasyDelay
    COMPANY_NAME "YangAudio"
    BUNDLE_ID "com.yangaudio.easydelay"
    PLUGIN_MANUFACTURER_CODE Yang
    PLUGIN_CODE EzDl
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx Delay"
)

target_sources(EasyDelay PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(EasyDelay PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(EasyDelay PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Source is UTF-8; this keeps MSVC from reading it as the system code page.
if(MSVC)
    target_compile_options(EasyDelay PRIVATE /utf-8)
endif()

option(EASYDELAY_BUILD_CHECKER "Build the local VST3 loading/validation tool" ON)

if(EASYDELAY_BUILD_CHECKER)
    add_subdirectory(tools/PluginCheck)
    add_subdirectory(tools/DspProbe)
    add_subdirectory(tools/EditorPreview)
endif()


