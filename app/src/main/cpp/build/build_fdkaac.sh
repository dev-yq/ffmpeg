make clean
#你自己的NDK路径.
export NDK=/usr/ffmpeg/android-ndk-r20
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64/
API=29
CFLAGS="-fPIC"
FLAGS="--enable-static   --host=$HOST   --disable-asm "
export CXX="${CXX}  --sysroot=${SYSROOT}"
export LDFLAGS=" -nostdlib -Bdynamic -Wl,--whole-archive -Wl,--no-undefined -Wl,-z,noexecstack  -Wl,-z,nocopyreloc -Wl,-soname,/system/lib/libz.so -Wl,-rpath-link=$SYSROOT/usr/lib ,-dynamic-linker=/system/bin/linker -L$NDK/sources/cxx-stl/gnu-libstdc++/libs/armeabi -L$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86/arm-linux-androideabi/lib -L$SYSROOT/usr/lib  -lc -lgcc -lm -ldl   $CFLAGS "
export CXXFLAGS=$CFLAGS
export CFLAGS=$CFLAGS
export CC="${CC}  --sysroot=${SYSROOT}"
export AR="${CROSS_PREFIX}ar"
export LD="${CROSS_PREFIX}ld"
export AS="${CROSS_PREFIX}gcc"
function build_android
{
./configure $FLAGS \
--enable-pic \
--enable-strip \
--host="arm-linux-androideabi"
--prefix=$PREFIX
$ADDITIONAL_CONFIGURE_FLAG
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
PREFIX=/usr/ffmpeg/ffmpeg-4.2.2/android/armv7-a

build_android
