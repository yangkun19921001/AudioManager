cmake_minimum_required(VERSION 3.4.1)


#设置输出的 so 目录
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

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
#FILE(GLOB FFMPEG_SHARED_LIB_PATH ${CMAKE_SOURCE_DIR}/../../../../thirdpartes/ffmpeg/${CMAKE_ANDROID_ARCH_ABI}/lib)
FILE(GLOB FFMPEG_SHARED_LIB_PATH ${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI})
#soundtouch  path
FILE(GLOB soundtouch_path ${CMAKE_SOURCE_DIR}/soundtouch/include)
FILE(GLOB soundtouch_source_path ${CMAKE_SOURCE_DIR}/soundtouch/source/*.cpp)
FILE(GLOB soundtouch_source_header ${CMAKE_SOURCE_DIR}/soundtouch/source/*.h)
#lame header
FILE(GLOB lame_path ${CMAKE_SOURCE_DIR}/lame/libmp3lame)
FILE(GLOB lame_path_header ${lame_path}/*.h)
FILE(GLOB lame_source_path ${lame_path}/*.c)
#mp3 encode
FILE(GLOB mp3_encode_path ${CMAKE_SOURCE_DIR}/encode)
FILE(GLOB mp3_encode_header_path ${mp3_encode_path}/*.h)
FILE(GLOB mp3_source_path ${mp3_encode_path}/*.cpp)


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
#lame header
include_directories(${lame_path_header}/)
#mp3 encode
include_directories(${mp3_encode_header_path})


#STATIC 库
#指定 ffmpeg 静态库文件目录
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_SHARED_LIB_PATH}/libavcodec.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_SHARED_LIB_PATH}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_SHARED_LIB_PATH}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_SHARED_LIB_PATH}/libswresample.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${FFMPEG_SHARED_LIB_PATH}/libavfilter.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FFMPEG_SHARED_LIB_PATH}/libswscale.so)


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
        ${manager_path}
        ${channel_path}
        ${util_path}
        ${soundtouch_source_path}
        ${decodec_path}
        ${lame_source_path}
        ${mp3_source_path}
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