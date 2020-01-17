# 清空上次的编译
make clean
#你自己的NDK路径.
export NDK=/usr/ffmpeg/android-ndk-r20
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64/
API=29
function build_android
{  ./configure \
    --prefix=$PREFIX \
    -disable-shared \
   --enable-static \
    --enable-pic \
--enable-neon \
--disable-cli \
    --cross-prefix=$CROSS_PREFIX \
   
    --arch=$ARCH \
    --cpu=$CPU \
    --cc=$CC
    --cxx=$CXX
    --enable-cross-compile \
    --sysroot=$SYSROOT \
   --extra-cflags="-Os -fpic" \
	--extra-ldflags=""
make clean
make install
}
#armv7-a
ARCH=arm
CPU=armv7-a
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
SYSROOT=$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
build_android  
