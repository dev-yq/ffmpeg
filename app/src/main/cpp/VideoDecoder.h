//
// Created by Administrator on 2020/1/19 0019.
//

#ifndef FFMPEG3_VIDEODECODER_H
#define FFMPEG3_VIDEODECODER_H





#include "pthread.h"
#include <queue>
#include <cstdlib>
#include<mutex>
extern "C"  {
#include <android/native_window_jni.h>
#include <android/log.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include "jni.h"
}


using namespace std;
    class     VideoDecoder{
    private:
         const  AVCodec* pCodecCtx;
        AVCodecContext *avCodecContext;
        uint*    timeBase;
        AVFrame *vFrame;
        ANativeWindow* nativeWindow;
        ANativeWindow_Buffer windowBuffer;
        SwsContext *swsContext;
        int   width  ,height;
        int   index;
        uint8_t *dst_data[4];
        int dst_linesize[4];
        static void* start (void *arg);

        bool    isPlayer;
        std::queue<AVPacket* > queue;
    public:


        VideoDecoder();
        ~VideoDecoder();



        void      setAVCodecContext(  AVCodecContext * avCodecContext);


        void   setAVCodec(AVCodec *pCodecCtx);


        AVCodecContext *  getAVCodecContext();
        AVCodec *    getAVCodec();

        void   setIndex(int    index);



        int   getIndex();

        int  init();

        void    put(AVPacket*  avPacket);

        void    player();




        void decoder(  AVPacket*  avPacket);

        void     inits(JNIEnv *env ,jobject jobject1);
        void    stop();





    };


#endif //FFMPEG3_VIDEODECODER_H

