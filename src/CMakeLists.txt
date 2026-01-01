set(
	VttAudioStreamer.AudioMixer.Sources
	"VttAudioStreamer/AudioMixer/Gstreamer/AudioMixer.cpp"
)

add_library(VttAudioStreamer.AudioMixer ${VttAudioStreamer.AudioMixer.Sources})
target_include_directories(VttAudioStreamer.AudioMixer PUBLIC "../public")
target_include_directories(VttAudioStreamer.AudioMixer PUBLIC "./")
set_property(TARGET VttAudioStreamer.AudioMixer PROPERTY CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(Boost REQUIRED COMPONENTS Spirit)
# target_link_libraries(VttAudioStreamer.AudioMixer PUBLIC Boost::spirit)

find_package(spdlog REQUIRED)
target_link_libraries(VttAudioStreamer.AudioMixer PUBLIC spdlog::spdlog)

add_library(compiler_flags INTERFACE)
target_compile_options(compiler_flags INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)

target_link_libraries(VttAudioStreamer.AudioMixer PUBLIC compiler_flags)

