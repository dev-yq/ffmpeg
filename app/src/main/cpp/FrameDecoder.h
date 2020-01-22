//
// Created by Administrator on 2020/1/18 0018.
//



#ifndef FFMPEG3_FRAMEDECODER_H
#define FFMPEG3_FRAMEDECODER_H
#include "VideoDecoder.h"



extern "C"  {

#include "AudioDecoder.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <android/log.h>

}

class     FrameDecoder{
private:

    static void* start (void *arg);
    AVPacket *vPacket;
    AVFormatContext	*pFormatCtx;
    AVFrame	*pFrame;
    AudioDecoder *audioDecoder;
    VideoDecoder * videoDecoder;
    const char  *   url;
public:


    FrameDecoder();
    ~FrameDecoder();
    int    init(const char *url);
    void      stop();





    void    player(JNIEnv *env ,jobject jobject1);

    void    init();
};

#endif //FFMPEG3_FRAMEDECODER_H