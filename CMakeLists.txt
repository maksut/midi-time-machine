cmake_minimum_required(VERSION 3.22)

project(MIDI_TIME_MACHINE_PROJECT VERSION 0.0.1)

# In Windows, statically link C++ runtime (/MT) for Release.
if(MSVC)
    # Debug -> MultiThreadedDebugDLL -> dynamic CRT (/MDd)
    # Everything else (inc. Release) -> MultiThreaded -> static CRT (/MT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "$<$<CONFIG:Debug>:MultiThreadedDebugDLL>$<$<NOT:$<CONFIG:Debug>>:MultiThreaded>")
endif()

add_subdirectory(JUCE)
add_subdirectory(midi-time-machine)
