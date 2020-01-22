//
// Created by Administrator on 2017/11/20.
//



#include "FFmpegMusic.h"
#define  TAG "ffmpeg"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
//播放线程
void *MusicPlay(void *args){
    FFmpegMusic *musicplay = (FFmpegMusic *) args;
           int  code =  musicplay->CreatePlayer();
    pthread_exit(0);//退出线程
}
//得到pcm数据
int getPcm(FFmpegMusic *agrs){
    AVPacket *avPacket = (AVPacket *) av_mallocz(sizeof(AVPacket));
    AVFrame *avFrame = av_frame_alloc();
    int size;
    int gotframe;
    LOGE("准备解码");
    if (agrs->isPlay){
        size=0;
        agrs->get(avPacket);
        //时间矫正
        if (avPacket->pts != AV_NOPTS_VALUE) {
            agrs->clock = av_q2d(agrs->time_base) * avPacket->pts;
        }

        LOGE("解码");

        int  ret = 0;
        ret = avcodec_send_packet(agrs->codec ,avPacket);
        if (ret < 0){
            LOGE("格式错误");
            return   -1;
        }
        ret =  avcodec_receive_frame(agrs->codec,avFrame);

        if (ret < 0 ||ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            LOGE("格式错误");
            return   -1;
        }



            swr_convert(agrs->swrContext, &agrs->out_buffer, 44100 * 2, (const uint8_t **) avFrame->data, avFrame->nb_samples);
//                缓冲区的大小
            size = av_samples_get_buffer_size(NULL, agrs->out_channer_nb, avFrame->nb_samples,
                                              AV_SAMPLE_FMT_S16, 1);


    }
    av_free(avPacket);
    av_frame_free(&avFrame);
    return size;

}
//回调函数
void  playCallback(SLAndroidSimpleBufferQueueItf bq, void *context){
    //得到pcm数据
    LOGE("回调pcm数据=================");
    FFmpegMusic *musicplay = (FFmpegMusic *) context;
    int datasize = getPcm(musicplay);
    if(datasize>0){
        //第一针所需要时间采样字节/采样率
        double time = datasize/(44100*2*2);
        //
        musicplay->clock=time+musicplay->clock;
        LOGE("当前一帧声音时间%f   播放时间%f",time,musicplay->clock);

        (*bq)->Enqueue(bq,musicplay->out_buffer,datasize);
        LOGE("播放 %d ",musicplay->queue.size());
    }
}

//初始化ffmpeg
int createFFmpeg(FFmpegMusic *agrs){
    LOGE("初始化ffmpeg");
    agrs->swrContext = swr_alloc();

    int length=0;
    int got_frame;
//    44100*2
    agrs->out_buffer = (uint8_t *) av_mallocz(44100 * 2);
    uint64_t  out_ch_layout=AV_CH_LAYOUT_STEREO;
//    输出采样位数  16位
    enum AVSampleFormat out_formart=AV_SAMPLE_FMT_S16;
//输出的采样率必须与输入相同
    int out_sample_rate = agrs->codec->sample_rate;
    swr_alloc_set_opts( agrs->swrContext, out_ch_layout, out_formart, out_sample_rate,
                        agrs->codec->channel_layout, agrs->codec->sample_fmt,  agrs->codec->sample_rate, 0,
                        NULL);
    swr_init( agrs->swrContext);
//    获取通道数  2
    agrs->out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    LOGE("------>通道数%d  ", agrs->out_channer_nb);
    return 0;
}


FFmpegMusic::FFmpegMusic() {
    clock =0;
//初始化锁
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
}

FFmpegMusic::~FFmpegMusic() {
    if (out_buffer) {
        free(out_buffer);
    }
    for (int i = 0; i < queue.size(); ++i) {
        AVPacket *pkt = queue.front();
        queue.erase(queue.begin());
        LOGE("销毁音频帧%d",queue.size());
        av_free(pkt);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void FFmpegMusic::setAvCodecContext(AVCodecContext *avCodecContext) {
    codec = avCodecContext;
    createFFmpeg(this);
}
//将packet压入队列,生产者
int FFmpegMusic::put(AVPacket *avPacket) {
    LOGE("插入队列");
    AVPacket *avPacket1 = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //克隆
    if(av_packet_ref(avPacket1,avPacket)){
        //克隆失败
        return 0;
    }
    //push的时候需要锁住，有数据的时候再解锁
    pthread_mutex_lock(&mutex);
    queue.push_back(avPacket1);//将packet压入队列
    //压入过后发出消息并且解锁
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 1;
}
//将packet弹出队列
int FFmpegMusic::get(AVPacket *avPacket) {
    LOGE("取出队列");
    pthread_mutex_lock(&mutex);
    while (isPlay){
        if(!queue.empty()&&isPause){
            LOGE("ispause %d",isPause);
            //如果队列中有数据可以拿出来
            if(av_packet_ref(avPacket,queue.front())){
                break;
            }
            //取成功了，弹出队列，销毁packet
            AVPacket *packet2 = queue.front();
            queue.erase(queue.begin());
            av_free(packet2);
            break;
        } else{
            LOGE("音频执行wait");
            LOGE("ispause %d",isPause);
            pthread_cond_wait(&cond,&mutex);

        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

void FFmpegMusic::play() {
        isPause=1;
        isPlay=1;
        pthread_create(&playId, NULL, MusicPlay, this);//开启begin线程


}

void FFmpegMusic::stop() {
    LOGE("声音暂停");
    //因为可能卡在 deQueue
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(playId, 0);
    if (playItf) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
        playItf = 0;
    }
    if (playerObject) {
        (*playerObject)->Destroy(playerObject);
        playerObject = 0;

    }

    if (mixObject) {
        (*mixObject)->Destroy(mixObject);
        mixObject = 0;
    }

    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineItf = 0;
    }
    if (swrContext)
        swr_free(&swrContext);
    if (this->codec) {
        if (avcodec_is_open(this->codec))
            avcodec_close(this->codec);
        avcodec_free_context(&this->codec);
        this->codec = 0;
    }
    LOGE("AUDIO clear");
}

int FFmpegMusic::CreatePlayer() {

    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    //获取引擎接口
//    SLEngineItf engineItf;
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);
    //通过引擎接口获取输出混音
//    SLObjectItf mixObject;
    (*engineItf)->CreateOutputMix(engineItf, &mixObject, 0, 0, 0);
    (*mixObject)->Realize(mixObject, SL_BOOLEAN_FALSE);

    //设置播放器参数
    SLDataLocator_AndroidSimpleBufferQueue
            android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    //pcm格式
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,
                            2,//两声道
                            SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//
                            SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource slDataSource = {&android_queue, &pcm};

    //输出管道
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mixObject};
    SLDataSink audioSnk = {&outputMix, NULL};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    //通过引擎接口，创建并且初始化播放器对象
//    SLObjectItf playerObject;
    (*engineItf)->CreateAudioPlayer(engineItf, &playerObject, &slDataSource, &audioSnk, 1, ids,
                                    req);
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);

    //获取播放接口
//    SLPlayItf playItf;
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playItf);
    //获取缓冲接口
//    SLAndroidSimpleBufferQueueItf bufferQueueItf;
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueueItf);

    //注册缓冲回调
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, playCallback, this);

    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);

    playCallback(bufferQueueItf, this);
    return 1;
}

void FFmpegMusic::pause() {
    if(isPause==1){
        isPause=0;
    } else{
        isPause=1;
        pthread_cond_signal(&cond);
    }
}

