set(
	VttAudioStreamer.AudioMixer.Sources
	"VttAudioStreamer/AudioMixer/NoCodec/AudioMixer.cpp"
)

add_library(VttAudioStreamer.AudioMixer ${VttAudioStreamer.AudioMixer.Sources})
target_include_directories(VttAudioStreamer.AudioMixer PUBLIC "../public")
target_include_directories(VttAudioStreamer.AudioMixer PUBLIC "./")
set_property(TARGET VttAudioStreamer.AudioMixer PROPERTY CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GStreamer
# find_package(PkgConfig REQUIRED)
# pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
# pkg_search_module(gstreamer-sdp REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
# pkg_search_module(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)

# spdlog for logging
find_package(spdlog REQUIRED)
target_link_libraries(VttAudioStreamer.AudioMixer PUBLIC 
	spdlog::spdlog
)

add_library(compiler_flags INTERFACE)
target_compile_options(compiler_flags INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)

target_link_libraries(VttAudioStreamer.AudioMixer PUBLIC compiler_flags)

