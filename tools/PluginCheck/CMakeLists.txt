juce_add_gui_app(EasyDelayCheck
    PRODUCT_NAME "EasyDelayCheck"
    COMPANY_NAME "YangAudio"
)

target_sources(EasyDelayCheck PRIVATE main.cpp)

target_include_directories(EasyDelayCheck PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../Source")

target_compile_definitions(EasyDelayCheck PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_AU=0
)

target_link_libraries(EasyDelayCheck PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# The plugin is a code dependency so building the checker first builds the VST3.
add_dependencies(EasyDelayCheck EasyDelay_VST3)

