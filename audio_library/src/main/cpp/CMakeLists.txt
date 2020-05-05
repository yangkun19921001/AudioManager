cmake_minimum_required(VERSION 3.4.1)

# native c++ 代码
FILE(GLOB yaudio_path ${CMAKE_SOURCE_DIR}/*.cpp)
# channel
FILE(GLOB channel_path ${CMAKE_SOURCE_DIR}/channel/*.cpp)
# callback path
FILE(GLOB callback_path ${CMAKE_SOURCE_DIR}/callback/*.cpp)
# decode path
FILE(GLOB decodec_path ${CMAKE_SOURCE_DIR}/decode/*.cpp)
# manager path
FILE(GLOB manager_path ${CMAKE_SOURCE_DIR}/manager/*.cpp)
# util path
FILE(GLOB util_path ${CMAKE_SOURCE_DIR}/util/*.cpp)
#ffmpeg path
FILE(GLOB FFMPEG_HEADER_PATH ${CMAKE_SOURCE_DIR}/../../../../thirdpartes/ffmpeg/${CMAKE_ANDROID_ARCH_ABI}/include)
FILE(GLOB FFMPEG_STATIC_LIB_PATH ${CMAKE_SOURCE_DIR}/../../../../thirdpartes/ffmpeg/${CMAKE_ANDROID_ARCH_ABI}/lib)
#soundtouch  path
FILE(GLOB soundtouch_path ${CMAKE_SOURCE_DIR}/soundtouch/include)
FILE(GLOB soundtouch_source_path ${CMAKE_SOURCE_DIR}/soundtouch/source/*.cpp)
FILE(GLOB soundtouch_source_header ${CMAKE_SOURCE_DIR}/soundtouch/source/*.h)


# FFMPEG HEAD
include_directories(${FFMPEG_HEADER_PATH})
#回调  head
include_directories(${CMAKE_SOURCE_DIR}/callback)
#解码 head
include_directories(${CMAKE_SOURCE_DIR}/decode)
#工具类 head
include_directories(${CMAKE_SOURCE_DIR}/util)
#数据渲染通道
include_directories(${CMAKE_SOURCE_DIR}/channel)
#管理
include_directories(${CMAKE_SOURCE_DIR}/manager)
#变调变速
include_directories(${soundtouch_header_path}/)
include_directories(${soundtouch_source_header}/)


#STATIC 库
#指定 ffmpeg 静态库文件目录
add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_STATIC_LIB_PATH}/libavcodec.a)

add_library(avformat STATIC IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_STATIC_LIB_PATH}/libavformat.a)

add_library(avutil STATIC IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_STATIC_LIB_PATH}/libavutil.a)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_STATIC_LIB_PATH}/libswresample.a)

add_library(avfilter STATIC IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${FFMPEG_STATIC_LIB_PATH}/libavfilter.a)

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FFMPEG_STATIC_LIB_PATH}/libswscale.a)


#include xlog
set(XLOG_HEADER ${CMAKE_SOURCE_DIR}/xlogger/include)
include_directories(${XLOG_HEADER})
# load xlog so
set(XLOG_LIB ${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libmarsxlog.so)
add_library(
        xlogger
        SHARED
        IMPORTED)
set_target_properties(xlogger PROPERTIES IMPORTED_LOCATION ${XLOG_LIB})


add_library(
        audio
        SHARED
        ${yaudio_path}
        ${callback_path}
        ${decodec_path}
        ${manager_path}
        ${channel_path}
        ${util_path}
        ${soundtouch_source_path}

)

target_link_libraries(
        audio
        android

        #PCM 数据渲染
        OpenSLES

        z
        #log---
        xlogger
        log
        #log---
        #ffmpeg----
        # 写了此命令不用在乎添加 ffmpeg lib 顺序问题导致应用崩溃
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        #ffmpeg----
)