//
// Created by Administrator on 2020/1/10 0010.
//

#ifndef FFMPEG3_FFMPEGUTILS_H
#define FFMPEG3_FFMPEGUTILS_H
#include <jni.h>
#include <string>
extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_init(JNIEnv *env, jclass type, jint width, jint height,
                                             jint outWidth, jint outHeight);

extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_release(JNIEnv *env, jclass type);
extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_yuvI420ToNV21(JNIEnv *env, jclass type, jbyteArray i420Src_,
                                                      jbyteArray nv21Src_, jint width,
                                                      jint height) ;

extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_compressYUV(JNIEnv *env, jclass type, jbyteArray src_,
                                                    jint width, jint height, jbyteArray dst_,
                                                    jint dst_width, jint dst_height, jint mode,
                                                    jint degree, jboolean isMirror) ;
extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_cropYUV(JNIEnv *env, jclass type, jbyteArray src_,
                                                jint width, jint height, jbyteArray dst_,
                                                jint dst_width, jint dst_height, jint left,
                                                jint top) ;

extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_encoderVideoinit(JNIEnv *env, jclass type, jint in_width,
                                                         jint in_height, jint out_width,
                                                         jint out_height) ;

extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_encoderVideoEncode(JNIEnv *env, jclass type,
                                                           jbyteArray srcFrame_, jint frameSize,
                                                           jint fps, jbyteArray dstFrame_,
                                                           jintArray outFramewSize_);
extern "C"

JNIEXPORT jint JNICALL

Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_encoderAudioInit(JNIEnv *env, jclass type, jint sampleRate,
        jint channels, jint bitRate) ;

extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_encoderAudioEncode(JNIEnv *env, jclass type,
        jbyteArray srcFrame_, jint frameSize,jbyteArray dstFrame_, jint dstSize) ;
extern "C"

JNIEXPORT jint JNICALL

Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_initRtmpData(JNIEnv *env, jclass type, jstring url_) ;
extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_sendRtmpVideoSpsPPS(JNIEnv *env, jclass type,
                                                            jbyteArray sps_, jint spsLen,
                                                            jbyteArray pps_, jint ppsLen,
                                                            jlong timeStamp);



extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_sendRtmpVideoData(JNIEnv *env, jclass type,
                                                          jbyteArray data_, jint dataLen,
                                                          jlong timeStamp) ;
extern "C"

JNIEXPORT jint JNICALL

Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_sendRtmpAudioSpec(JNIEnv *env, jclass type,
        jlong timeStamp);
extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_sendRtmpAudioData(JNIEnv *env, jclass type,
                                                                  jbyteArray data_, jint dataLen,
                                                                  jlong timeStamp) ;
extern "C"
JNIEXPORT jint JNICALL
Java_com_live_ffmpeg_ffmpeg_FFmpegUtils_releaseRtmp(JNIEnv *env, jclass type) ;
#endif //FFMPEG3_FFMPEGUTILS_H

