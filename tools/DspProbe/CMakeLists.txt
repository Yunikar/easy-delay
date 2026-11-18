juce_add_console_app(EasyDelayProbe PRODUCT_NAME "EasyDelayProbe")

target_sources(EasyDelayProbe PRIVATE
    main.cpp
    ../../Source/PluginProcessor.cpp
    ../../Source/PluginEditor.cpp
)

target_include_directories(EasyDelayProbe PRIVATE ../../Source)

target_compile_definitions(EasyDelayProbe PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JucePlugin_Name="Easy Delay"
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_EditorRequiresKeyboardFocus=0
    JucePlugin_VersionString="1.0.0"
    JucePlugin_VersionCode=0x10000
    JucePlugin_ManufacturerCode=0x59616e67
    JucePlugin_PluginCode=0x457a446c
)

target_link_libraries(EasyDelayProbe PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
)
