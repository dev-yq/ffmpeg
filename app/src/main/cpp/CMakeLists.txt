
cmake_minimum_required(VERSION 3.4.1)
include_directories(${CMAKE_SOURCE_DIR}/ffmpeg)
include_directories(${CMAKE_SOURCE_DIR}/yuv)
include_directories(${CMAKE_SOURCE_DIR}/librtmp)
include_directories(${CMAKE_SOURCE_DIR}/aac)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}/")

# 源文件编译生成链接库
add_library(ffmpeg SHARED
        FFmpegUtils.cpp
        wavreader.c
        RtmpLivePublish.cpp
        FrameEncoder.cpp
        AudioEncoder.cpp)
add_library(avcodec  SHARED
        IMPORTED )
set_target_properties( avcodec
        PROPERTIES IMPORTED_LOCATION
        ../../../../libs/armeabi-v7a/libavcodec.so)
add_library(avdevice  SHARED
        IMPORTED )
set_target_properties(  avdevice   PROPERTIES IMPORTED_LOCATION      ../../../../libs/armeabi-v7a/libavdevice.so )
add_library(avfilter  SHARED IMPORTED )
set_target_properties(  avfilter   PROPERTIES IMPORTED_LOCATION   ../../../../libs/armeabi-v7a/libavfilter.so)
add_library(avformat  STATIC IMPORTED )
set_target_properties(avformat   PROPERTIES IMPORTED_LOCATION    ../../../../libs/armeabi-v7a/libavformat.so)

add_library(avutil  SHARED
        IMPORTED )
set_target_properties(  avutil   PROPERTIES IMPORTED_LOCATION  ../../../../libs/armeabi-v7a/libavutil.so)

add_library(postproc  SHARED
        IMPORTED )
set_target_properties(  postproc   PROPERTIES IMPORTED_LOCATION   ../../../../libs/armeabi-v7a/libpostproc.so)
add_library(swresample  SHARED
        IMPORTED )
set_target_properties( swresample  PROPERTIES IMPORTED_LOCATION  ../../../../libs/armeabi-v7a/libswresample.so)
add_library(swscale  SHARED
        IMPORTED )
set_target_properties( swscale  PROPERTIES IMPORTED_LOCATION   ../../../../libs/armeabi-v7a/libswscale.so)

add_library(x264  STATIC
        IMPORTED )

set_target_properties( x264  PROPERTIES IMPORTED_LOCATION  ../../../../libs/armeabi-v7a/libx264.a)

add_library(yuv  STATIC
        IMPORTED )

set_target_properties( yuv  PROPERTIES IMPORTED_LOCATION   ../../../../libs/armeabi-v7a/libyuv.a)
add_library(rtmp  STATIC
        IMPORTED )

set_target_properties( rtmp  PROPERTIES IMPORTED_LOCATION  ../../../../libs/armeabi-v7a/librtmp.a)
add_library(aac  STATIC
        IMPORTED )

set_target_properties( aac  PROPERTIES IMPORTED_LOCATION   ../../../../libs/armeabi-v7a/libfdk-aac.a)
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                        ffmpeg
                        avcodec
                        avdevice
                        avfilter
                        avformat
                        avutil
                        postproc
                        swresample
                        swscale
                        x264
                        yuv
                        aac
                        rtmp
                           # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )