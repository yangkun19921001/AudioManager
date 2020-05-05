#!/bin/bash

echo ">>>>>>>>> 注意：该编译环境目前只在 NDK17c + ffmpeg4.2.2 测试过 <<<<<<<<"
echo ">>>>>>>>> 注意：该编译环境目前只在 NDK17c + ffmpeg4.2.2 测试过 <<<<<<<<"
echo ">>>>>>>>> 注意：该编译环境目前只在 NDK17c + ffmpeg4.2.2 测试过 <<<<<<<<"

#NDK_ROOT 变量指向 ndk 目录
NDK_ROOT=$NDK_HOME
#指定android api版本
ANDROID_API=21


#开始编译 在下面调用传入参数即可
function build_ffmpeg
{
echo "开始编译 $PREFIX_CPU"
echo "开始编译 $PREFIX"
echo "开始编译 $TOOLCHAIN"

./configure \
--prefix=$PREFIX \
--enable-small \
--disable-programs \
--disable-avdevice \
--disable-encoders \
--disable-muxers \
--disable-filters \
--enable-cross-compile \
--cross-prefix=$CROSS_PREFIX \
--disable-shared \
--enable-static \
--sysroot=$NDK_ROOT/platforms/android-$ANDROID_API/arch-$ARCH \
--extra-cflags="$CFLAGES" \
--arch=$ARCH \
--target-os=android

#上面运行脚本生成makefile之后，使用make执行脚本
make clean
make
make install

echo "$PREFIX_CPU 编译完成"
echo "$PREFIX_CPU 编译完成"
echo "$PREFIX_CPU 编译完成"
}

#armeabi-v7a
PREFIX=./result/armeabi-v7a
TOOLCHAIN=$NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
ARCH=arm
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
CFLAGES="-isysroot $NDK_ROOT/sysroot -isystem $NDK_ROOT/sysroot/usr/include/arm-linux-androideabi -D__ANDROID_API__=$ANDROID_API -U_FILE_OFFSET_BITS  -DANDROID -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -Wa,--noexecstack -Wformat -Werror=format-security  -O0 -fPIC"

build_ffmpeg

#arm64-v8a
PREFIX=./result/arm64-v8a
TOOLCHAIN=$NDK_ROOT/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
ARCH=arm64
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
CFLAGES="-isysroot $NDK_ROOT/sysroot -isystem $NDK_ROOT/sysroot/usr/include/aarch64-linux-android -D__ANDROID_API__=$ANDROID_API -U_FILE_OFFSET_BITS  -DANDROID -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -Wa,--noexecstack -Wformat -Werror=format-security  -O0 -fPIC"

build_ffmpeg

#直接跳转到编译完成的路径
cd result/