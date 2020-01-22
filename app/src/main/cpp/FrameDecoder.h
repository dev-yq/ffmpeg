//
// Created by Administrator on 2020/1/18 0018.
//



#ifndef FFMPEG3_FRAMEDECODER_H
#define FFMPEG3_FRAMEDECODER_H
#include "FFmpegVideo.h"
#include "FFmpegMusic.h"


extern "C"  {


#include <unistd.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <android/log.h>
#include <android/native_window_jni.h>
}

class     FrameDecoder{
private:

    static void* start (void *arg);
    AVPacket *vPacket;
    AVFormatContext	*pFormatCtx;
    AVFrame	*pFrame;
    FFmpegMusic *audioDecoder;
    FFmpegVideo * videoDecoder;
    const char  *   url;


    bool     isPlayer;
public:


    FrameDecoder();
    ~FrameDecoder();
    int    init(const char *url);
    void      stop();





    void    player(JNIEnv *env ,jobject jobject1);

    void    init();
};

#endif //FFMPEG3_FRAMEDECODER_H