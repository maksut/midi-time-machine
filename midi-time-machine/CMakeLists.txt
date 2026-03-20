juce_add_plugin(midi-time-machine
    COMPANY_NAME MTM
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_NAME "MIDI Time Machine"
    PLUGIN_DESC "MIDI Time Machine"
    VST3_CATEGORIES Fx Instrument Tools
    BUNDLE_ID com.machine-time-midi
    AAX_IDENTIFIER com.machine-time-midi
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "midi-time-machine"
)

target_sources(midi-time-machine
    PRIVATE
        Source/MessageTracker.h
        Source/MidiQueue.h
        Source/Playback.cpp
        Source/Playback.h
        Source/Processor.cpp
        Source/Processor.h
        Source/State.h
        Source/Store.cpp
        Source/Store.h
        Source/ValueTreeLogger.h
        Source/ui/Content.h
        Source/ui/Editor.cpp
        Source/ui/Editor.h
        Source/ui/Keyboard.h
        Source/ui/MidiRoll.h
        Source/ui/Resources.cpp
        Source/ui/Resources.h
        Source/ui/RoundedComponent.h
        Source/ui/Settings.h
        Source/ui/Toolbar.h
        Source/ui/VelocityLines.h
)

target_compile_definitions(midi-time-machine
    PUBLIC
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        # So <JuceHeader.h> does not have "using namespace juce"
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, we do not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
)

target_link_libraries(midi-time-machine
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_processors_headless
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_midi_ci
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# So LinkTimeOptimisation uses all available cores
target_compile_options(midi-time-machine PRIVATE -flto=auto)
target_link_options(midi-time-machine PRIVATE -flto=auto)

# Generates <JuceHeader.h>
juce_generate_juce_header(midi-time-machine)
target_include_directories(midi-time-machine PUBLIC Source)
