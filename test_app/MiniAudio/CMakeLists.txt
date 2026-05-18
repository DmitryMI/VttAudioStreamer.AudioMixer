cmake_minimum_required(VERSION 3.10)

project(MiniAudioTestApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect test app source files
set(MiniAudioTestApp.Sources
    "main.cpp"
)

# Create executable
add_executable(MiniAudioTestApp ${MiniAudioTestApp.Sources})

find_package(spdlog REQUIRED)
target_link_libraries(MiniAudioTestApp PRIVATE spdlog::spdlog)

# Find and link miniaudio for audio output
find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
if(NOT MINIAUDIO_INCLUDE_DIRS)
    message(FATAL_ERROR "miniaudio.h not found. Please install miniaudio via vcpkg.")
endif()
target_include_directories(MiniAudioTestApp PRIVATE ${MINIAUDIO_INCLUDE_DIRS})

# Link compiler flags (MSVC /EHsc)
target_link_libraries(MiniAudioTestApp PRIVATE compiler_flags)

# Platform-specific audio libraries
if(WIN32)
    target_link_libraries(MiniAudioTestApp PRIVATE winmm ole32)
elseif(APPLE)
    target_link_libraries(MiniAudioTestApp PRIVATE "-framework CoreAudio" "-framework AudioToolbox" "-framework AudioUnit")
elseif(UNIX)
    find_package(ALSA REQUIRED)
    target_link_libraries(MiniAudioTestApp PRIVATE ALSA::ALSA)
    # Also try PulseAudio if available
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PULSEAUDIO QUIET libpulse)
        if(PULSEAUDIO_FOUND)
            target_include_directories(MiniAudioTestApp PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
            target_link_directories(MiniAudioTestApp PRIVATE ${PULSEAUDIO_LIBRARY_DIRS})
            target_link_libraries(MiniAudioTestApp PRIVATE ${PULSEAUDIO_LIBRARIES})
        endif()
    endif()
endif()

# Set output directory
set_target_properties(MiniAudioTestApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
