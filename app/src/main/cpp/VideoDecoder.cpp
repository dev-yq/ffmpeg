//
// Created by Administrator on 2020/1/19 0019.
//

#include <unistd.h>
#include "VideoDecoder.h"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "err_typecheck_member_reference_struct_union"
#define  TAG "ffmpeg"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

//视频解码
VideoDecoder::VideoDecoder() {}
VideoDecoder::~VideoDecoder() {
}
void VideoDecoder::setAVCodec(AVCodec *pCodecCtx) {
    this->pCodecCtx   =pCodecCtx;
}



void VideoDecoder::setAVCodecContext(AVCodecContext *avCodecContext) {
    this->avCodecContext    = avCodecContext;
}

void VideoDecoder::inits(JNIEnv *env, jobject jobject1) {
    nativeWindow = ANativeWindow_fromSurface(env, jobject1);
}

void VideoDecoder::decoder( AVPacket*  avPacket) {
    if (isPlayer){
            //获取视频消息
            int   ret =  avcodec_send_packet(avCodecContext   ,avPacket);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF){}
            else{
                ret = avcodec_receive_frame(avCodecContext , vFrame);
                if (ret < 0 && ret != AVERROR_EOF){
                }
                else{
                    sws_scale(swsContext, (const uint8_t* const*)vFrame->data,
                             vFrame->linesize, 0, avCodecContext->height,
                              dst_data,    dst_linesize);
                    ANativeWindow_lock(nativeWindow,&windowBuffer, 0);
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;//
                    int  dstStride = windowBuffer.stride * 4;//
                    uint8_t *src =    dst_data[0];
                    int srcStride =    dst_linesize[0];
                    int h;//
                    for (h = 0; h < avCodecContext->height; h++) {//
                        memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                    }
                    ANativeWindow_unlockAndPost(nativeWindow);
                }
            }

    }



}

int VideoDecoder::init() {
  int   res =   avcodec_open2(avCodecContext ,pCodecCtx ,NULL);
    if (res<0){
        LOGE("打开解码码器失败");
        return    -1;
    }
    //视频宽高
    width     = avCodecContext->width;
    height  =  avCodecContext->height;
    //分配视频帧内存
    vFrame     =av_frame_alloc();
    return   0;
}



void VideoDecoder::setIndex(int index) {
    this->index =  index;
}

int VideoDecoder::getIndex() {
    return    this->index;
}


void VideoDecoder::put(AVPacket *avPacket) {
    this->queue.push(avPacket);
}



//开始播放
void VideoDecoder::player() {
    isPlayer    = true;
    swsContext = sws_getContext(width, height, avCodecContext->pix_fmt,
                                width, height,   AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);

    av_image_alloc(dst_data, dst_linesize, width, height,AV_PIX_FMT_RGBA, 1);
    ANativeWindow_setBuffersGeometry(nativeWindow,height,width,WINDOW_FORMAT_RGBA_8888);

}

void* VideoDecoder::start(void *arg) {
    VideoDecoder * videoDecoder   = (VideoDecoder*)arg;
    while (videoDecoder->isPlayer){

        }

    return   0;
}

void VideoDecoder::stop() {
    isPlayer  = false;
    sws_freeContext(this->swsContext);

    ANativeWindow_release(nativeWindow);
    av_free(vFrame);
    av_free(&pCodecCtx);
    avcodec_close(this->avCodecContext);


}


#pragma clang diagnostic pop