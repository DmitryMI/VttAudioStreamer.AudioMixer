cmake_minimum_required(VERSION 3.20)

project(VttAudioStreamer.AudioMixer.Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VttAudioStreamer.AudioMixer.Test.Sources 
	"VttAudioStreamer/AudioMixer/Gstreamer/AudioMixerTest.cpp"
)

enable_testing()

add_executable(VttAudioStreamer.AudioMixer.Test ${VttAudioStreamer.AudioMixer.Test.Sources} )
target_link_libraries(VttAudioStreamer.AudioMixer.Test PRIVATE VttAudioStreamer.AudioMixer)
target_include_directories(VttAudioStreamer.AudioMixer.Test PRIVATE "./")

find_package(spdlog REQUIRED)
target_link_libraries(VttAudioStreamer.AudioMixer.Test PRIVATE spdlog::spdlog)

find_package(gtest REQUIRED)
target_link_libraries(VttAudioStreamer.AudioMixer.Test PRIVATE GTest::gtest_main GTest::gmock)

# Link the shared interface that sets /EHsc on MSVC
# Ensure compiler_flags is created in a directory processed before this test (top-level or src)
target_link_libraries(VttAudioStreamer.AudioMixer.Test PRIVATE compiler_flags)

include(GoogleTest)
gtest_discover_tests(VttAudioStreamer.AudioMixer.Test)