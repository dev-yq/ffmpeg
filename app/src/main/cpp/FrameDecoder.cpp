

#include "FrameDecoder.h"


#define  TAG "ffmpeg"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
ANativeWindow *window = 0;
FrameDecoder::FrameDecoder(){

};
FrameDecoder::~FrameDecoder() {
}
void call_video_play(AVFrame *frame) {
    if (!window) {
        return;
    }
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        return;
    }

    LOGE("绘制 宽%d,高%d", frame->width, frame->height);
    LOGE("绘制 宽%d,高%d  行字节 %d ", window_buffer.width, window_buffer.height, frame->linesize[0]);
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int dstStride = window_buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}
int FrameDecoder::init(const  char *url) {

    videoDecoder   = new FFmpegVideo();
    audioDecoder    = new FFmpegMusic();

    avformat_network_init();



    pFormatCtx  =     avformat_alloc_context();


    int    res  =  avformat_open_input(&pFormatCtx ,    url,NULL ,NULL);
    if (res!=0){
        LOGE("地址解析失败");
        return -1;
    }
    return      0;

}

void * FrameDecoder::start(void *arg) {


    FrameDecoder * frameDecoder =  (FrameDecoder*)arg;

    //获取视频流媒
   int  res   =     avformat_find_stream_info(frameDecoder->pFormatCtx  ,NULL);
    if(res<0){
        LOGE("获取信息失败");
        return reinterpret_cast<void *>(-1);
    }
    int videoindex = -1;


    int   audioIndex   =-1;

    for (int i = 0; i <frameDecoder->pFormatCtx ->nb_streams ; i++) {
        if (frameDecoder->pFormatCtx ->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;

        }

        else if (frameDecoder->pFormatCtx ->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audioIndex   = i;

        }

    }
    if(videoindex == -1||audioIndex==-1){
        LOGE("获取视频信息失败");
        return reinterpret_cast<void *>(-1);
    }
    //查找视频解码id





    //视频解码信息

    AVCodecParameters *avCodecParameters =  frameDecoder->pFormatCtx->streams[videoindex]->codecpar;
    AVCodec * vidoeCode   =    avcodec_find_decoder(avCodecParameters->codec_id);


    AVCodecContext *avCodecContext = avcodec_alloc_context3(vidoeCode);
    avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    frameDecoder->videoDecoder->setAvCodecContext(avCodecContext);
    frameDecoder-> videoDecoder->index = videoindex;
    frameDecoder->videoDecoder->time_base = frameDecoder->pFormatCtx->streams[videoindex]->time_base;
    //查找音频解码id
    if (window) {
        ANativeWindow_setBuffersGeometry(window, frameDecoder->videoDecoder->codec->width,
                                         frameDecoder->videoDecoder->codec->height,
                                         WINDOW_FORMAT_RGBA_8888);
    }
    if (avcodec_open2(avCodecContext, vidoeCode, NULL) < 0) {
        LOGE("打开失败");
        return reinterpret_cast<void *>(-1);
    }
    //音频解码信息

    AVCodecParameters *avCodecParameters1 =  frameDecoder->pFormatCtx->streams[audioIndex]->codecpar;
    AVCodec *  avCodec =  avcodec_find_decoder(avCodecParameters1->codec_id );
    AVCodecContext *avCodecContext1 = avcodec_alloc_context3(  avCodec);
    avcodec_parameters_to_context(avCodecContext1, avCodecParameters1);

    frameDecoder-> audioDecoder->index = audioIndex;
    frameDecoder-> audioDecoder->setAvCodecContext(avCodecContext1);

    frameDecoder->audioDecoder->time_base = frameDecoder->pFormatCtx->streams[audioIndex]->time_base;

    if (avcodec_open2(avCodecContext1, avCodec, NULL) < 0) {
        LOGE("打开失败");
        return reinterpret_cast<void *>(-1);
    }
    AVPacket *  vPacket   =(AVPacket *)(malloc(sizeof( AVPacket )));
    frameDecoder-> videoDecoder->setFFmepegMusic( frameDecoder-> audioDecoder);
    frameDecoder-> audioDecoder->play();
    frameDecoder-> videoDecoder->play();
    LOGE("开始解码");
    while (frameDecoder->isPlayer){
           int  res  = av_read_frame(frameDecoder->pFormatCtx ,vPacket);
           if (res==0){
               if ( frameDecoder->videoDecoder &&vPacket->stream_index ==  frameDecoder-> videoDecoder->index  &&   frameDecoder->videoDecoder->isPlay){
                   frameDecoder->videoDecoder->put(vPacket);

               } else  if (frameDecoder->audioDecoder&&vPacket->stream_index  ==  frameDecoder-> audioDecoder->index &&   frameDecoder->audioDecoder->isPlay){
                   frameDecoder->audioDecoder->put(vPacket);

               }
               av_packet_unref(vPacket);
           }
           else if(res ==AVERROR_EOF){


               while (frameDecoder->isPlayer) {
                   if (  frameDecoder->videoDecoder->queue.empty() &&   frameDecoder->videoDecoder->queue.empty()) {
                       break;
                   }
//                LOGI("等待播放完成");
                   av_usleep(10000);
               }
           }

           }

    av_packet_unref(vPacket);
    avformat_free_context(frameDecoder->pFormatCtx);

    pthread_exit(0);






}



void FrameDecoder::stop() {
    ANativeWindow_release(window);
    this->isPlayer  = false;
    this->videoDecoder->stop();

    this->audioDecoder->stop();
    avformat_free_context(pFormatCtx);
    free(videoDecoder);
    free(audioDecoder);
}



void FrameDecoder::player(JNIEnv *env ,jobject jobject1) {
    this->isPlayer  = true;
    window = ANativeWindow_fromSurface(env, jobject1);


    videoDecoder->setPlayCall(call_video_play);

    pthread_t _pthread_ptr;
    pthread_create(&_pthread_ptr, NULL , start,(void *)this);







}



