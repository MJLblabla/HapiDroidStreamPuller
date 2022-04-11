
cmake_minimum_required(VERSION 3.18.1)
# Declares and names the project.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
project("libstreaming")

set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(libname libstreaming)

include_directories(
        include
        include/libyuv
        include/librtmp
        codec
        common
        util
        muxer
        output
        output/mp4
        output/rtmp
        #        output/hls
        output/srt
        output/tslib
)

link_directories(
        ${jnilibs}/${ANDROID_ABI}
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.


file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*.cpp
        ${CMAKE_SOURCE_DIR}/codec/*.cpp
        ${CMAKE_SOURCE_DIR}/common/*.cpp
        ${CMAKE_SOURCE_DIR}/util/*.cpp
        ${CMAKE_SOURCE_DIR}/muxer/*.cpp
        ${CMAKE_SOURCE_DIR}/output/*.cpp
        ${CMAKE_SOURCE_DIR}/output/mp4/*.cpp
        ${CMAKE_SOURCE_DIR}/output/rtmp/*.cpp
        #        ${CMAKE_SOURCE_DIR}/output/hls/*.cpp
        ${CMAKE_SOURCE_DIR}/output/srt/*.cpp
        ${CMAKE_SOURCE_DIR}/output/tslib/*.cpp
        )

add_library( # Sets the name of the library.
        ${libname}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files}
        )

#
set(third-party-libs_yuv
        yuv
        )
set(third-party-libs_ffmpeg
        swresample
        avutil
        )
set(third-party-libs_rtmp
        rtmp
        )
#
set(native-libs
        mediandk
        log
        )

set(third-party-libs_srt
        srt
        ssl
        )

target_link_libraries( # Specifies the target library.
        ${libname}
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${third-party-libs_srt}
        ${third-party-libs_yuv}
        ${third-party-libs_ffmpeg}
        ${third-party-libs_rtmp}
        ${native-libs}
        )
