
#include <unistd.h>
#include "FrameDecoder.h"


#define  TAG "ffmpeg"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

FrameDecoder::FrameDecoder(){

};
FrameDecoder::~FrameDecoder() {
}

int FrameDecoder::init(const  char *url) {
    this->url    =url;
    videoDecoder   = new VideoDecoder();

    audioDecoder    = new AudioDecoder();
    init();

    return      0;

}
void FrameDecoder::init() {
    //创建播放线程
    pthread_t _pthread_ptr;
    pthread_create(&_pthread_ptr, NULL , start,(void *)this);

}
void * FrameDecoder::start(void *arg) {

    const char  *   url   = "rtmp://118.190.54.75:1935/hls";
    avformat_network_init();
    FrameDecoder * frameDecoder =  (FrameDecoder*)arg;
    frameDecoder->pFormatCtx  =     avformat_alloc_context();


    int    res  =  avformat_open_input(&frameDecoder->pFormatCtx ,    url,NULL ,NULL);
    if (res!=0){
        LOGE("地址解析失败");
        return reinterpret_cast<void *>(-1);
    }
    //获取视频流媒
    res   =     avformat_find_stream_info(frameDecoder->pFormatCtx  ,NULL);
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
    AVCodecID  codeId =    frameDecoder->pFormatCtx ->streams[videoindex]->codecpar->codec_id;






    //视频解码信息

    AVCodecParameters *avCodecParameters =  frameDecoder->pFormatCtx->streams[videoindex]->codecpar;
    AVCodec * vidoeCode   =    avcodec_find_decoder(avCodecParameters->codec_id);
    AVCodecContext *avCodecContext = avcodec_alloc_context3(vidoeCode);
    avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    frameDecoder->videoDecoder->setAVCodec(vidoeCode);
    frameDecoder->videoDecoder->setAVCodecContext(avCodecContext);
    frameDecoder->videoDecoder->setIndex(videoindex);
    frameDecoder->videoDecoder->init();

    //查找音频解码id
    AVCodecID    audioCodeId    =    frameDecoder->pFormatCtx ->streams[audioIndex]->codecpar->codec_id;

    //音频解码信息
    frameDecoder->audioDecoder->setAVCodec( avcodec_find_decoder(audioCodeId ));
    AVCodecParameters *avCodecParameters1 =  frameDecoder->pFormatCtx->streams[audioIndex]->codecpar;

    AVCodecContext *avCodecContext1 = avcodec_alloc_context3(    frameDecoder->audioDecoder->getAVCodec());
    avcodec_parameters_to_context(avCodecContext1, avCodecParameters1);
    frameDecoder-> audioDecoder->setAVCodecContext(avCodecContext1);
    frameDecoder->audioDecoder->setIndex(audioIndex);

    AVPacket *  vPacket   =(AVPacket *)(malloc(sizeof( AVPacket )));

    LOGE("开始解码");
    while (av_read_frame(frameDecoder->pFormatCtx ,vPacket)==0){
            if (vPacket->stream_index == frameDecoder->videoDecoder->getIndex()){
            frameDecoder->videoDecoder->decoder(vPacket);


            usleep(1000*16);

        } else  if (vPacket->stream_index  == frameDecoder->audioDecoder->getIndex()
        && frameDecoder->audioDecoder->getIndex()!=-1){
//            frameDecoder->audioDecoder->put(vPacket);

        }




    }
    av_packet_unref(vPacket);


    return 0;

}



void FrameDecoder::stop() {

    this->videoDecoder->stop();

    this->audioDecoder->stop();
    avformat_free_context(pFormatCtx);
    free(videoDecoder);
    free(audioDecoder);
}



void FrameDecoder::player(JNIEnv *env ,jobject jobject1) {
    this->videoDecoder->inits(env ,jobject1);
    this->videoDecoder->player();
    this->audioDecoder->player();






}



