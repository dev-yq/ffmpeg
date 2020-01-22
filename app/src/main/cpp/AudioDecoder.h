//
// Created by Administrator on 2020/1/19 0019.
//

#ifndef FFMPEG3_AUDIODECODER_H
#define FFMPEG3_AUDIODECODER_H

#include <queue>

extern  "C"{
#include <android/log.h>
#include "android/native_window.h"
#include <jni.h>
#include "pthread.h"
#include <cstdlib>
#include "libavformat/avformat.h"
#include "AudioDataProvider.h"
#include <libavcodec/avcodec.h>
#include "libswresample/swresample.h"
#include "AudioDataProvider.h"
#include <libswscale/swscale.h>
}

using namespace std;

class    AudioDecoder{


private:

    AVCodec 	*audioCode;
    AVCodecContext *audioCodecContext;
    int    index;
    uint*    timeBase;
    std::queue<AVPacket* >  queue;
    AVFrame *vFrame;
   SwrContext * swsContext = NULL;
    bool    isPlayer;
    AudioDataProvider *audioDataProvider;
    static void* start (void *arg);
public:

    AudioDecoder();
    ~AudioDecoder();
    void      setAVCodecContext(  AVCodecContext * avCodecContext);
    void   setAVCodec(AVCodec *pCodecCtx);
    void   setIndex(int    index);
    int   getIndex();
    AVCodecContext *  getAVCodecContext();
    AVCodec *    getAVCodec();


    void    put(AVPacket*  avPacket);


    void    player();



    void   stop();
};


#endif //FFMPEG3_AUDIODECODER_H