//
// Created by Administrator on 2020/1/19 0019.
//




#include "AudioDecoder.h"
#define  TAG "ffmpeg"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__);
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

//音频解码
AudioDecoder::AudioDecoder() {


}

AudioDecoder::~AudioDecoder() {

}

void AudioDecoder::setAVCodecContext(AVCodecContext *avCodecContext) {



    this->audioCodecContext   = avCodecContext;
}

void AudioDecoder::setAVCodec(AVCodec *pCodecCtx) {

    this->audioCode  =  pCodecCtx;
}


AVCodecContext* AudioDecoder::getAVCodecContext() {


    return    this->audioCodecContext;


}



AVCodec* AudioDecoder::getAVCodec() {


    return     this->audioCode;
}





int AudioDecoder::getIndex() {


    return    this->index;


}



void AudioDecoder::setIndex(int index) {

    this->index=   index;

}

void AudioDecoder::put(AVPacket *avPacket) {

    this->queue.push(avPacket);
}


void AudioDecoder::player() {
    vFrame   =av_frame_alloc();
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    int64_t in_ch_layout = audioCodecContext->channel_layout;
    enum AVSampleFormat in_sample_fmt = audioCodecContext->sample_fmt;
    int in_sample_rate = audioCodecContext->sample_rate;
   int  out_sample_rate = 44100;
   swsContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt,
                                    out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    audioDataProvider     =  new AudioDataProvider();

    audioDataProvider->initAudioPlayer(44100 ,2);

    audioDataProvider->startAudioPlay();






    isPlayer  = true;
    pthread_t _pthread_ptr;
    pthread_create(&_pthread_ptr, NULL , start,(void *)this);
}


//解码线程

void* AudioDecoder::start(void *arg) {

    AudioDecoder * videoDecoder   = (AudioDecoder*)arg;

    while (videoDecoder->isPlayer) {

        if (!videoDecoder->queue.empty()){
            //音频解码
            AVPacket *avPacket = videoDecoder->queue.front();
            int ret = avcodec_send_packet(videoDecoder->audioCodecContext, avPacket);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            } else {
                ret = avcodec_receive_frame(videoDecoder->audioCodecContext, videoDecoder->vFrame);

                if (ret < 0 && ret != AVERROR_EOF) {

                } else {
                    int swrInitRes = swr_init(videoDecoder->swsContext);
                    uint8_t * resampleOutBuffer = (uint8_t *) malloc(videoDecoder->audioCodecContext->frame_size * 2 * 2);

                    int  dataSize = swr_convert(videoDecoder->swsContext, &resampleOutBuffer, videoDecoder->vFrame->nb_samples,
                                                (const uint8_t **) videoDecoder->vFrame->data, videoDecoder->vFrame->nb_samples);
                    dataSize = dataSize * 2 * 2;
                    AudioData *audioData =  new AudioData();

                    audioData->setSize(dataSize);

                    audioData->setData(resampleOutBuffer);
                    videoDecoder->audioDataProvider->put(audioData);
                }
                av_packet_unref(avPacket);
            }

        }


    }



    av_free(videoDecoder->vFrame);
    avcodec_close(videoDecoder->audioCodecContext);

    return   0;


}


void AudioDecoder::stop() {

    isPlayer  = false;
    av_free(vFrame);
    av_free(this->audioCode);


    avcodec_close(audioCodecContext);
}