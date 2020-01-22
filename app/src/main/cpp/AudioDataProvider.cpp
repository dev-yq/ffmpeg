//
// Created by Administrator on 2020/1/19 0019.
//

#include "AudioDataProvider.h"
#define  TAG "ffmpeg"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

SLObjectItf engineObject = NULL;
SLEngineItf engineEngine;




// output mix interfaces
SLObjectItf outputMixObject;
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
SLObjectItf bqPlayerObject = NULL;
SLPlayItf bqPlayerPlay;
SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
SLEffectSendItf bqPlayerEffectSend;
SLMuteSoloItf bqPlayerMuteSolo;
SLVolumeItf bqPlayerVolume;
SLmilliHertz bqPlayerSampleRate = 0;
short *resampleBuf = NULL;

pthread_mutex_t  audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

const SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
const int outputBufferSize = 8196;

bool stop = false;


AudioDataProvider::AudioDataProvider() {


}

AudioDataProvider::~AudioDataProvider() {


}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{


    AudioDataProvider *  audioDataProvider  =( AudioDataProvider *)context;


    while(true){




        if (!audioDataProvider->queue.empty()){


            AudioData * audioData  = audioDataProvider->queue.front();
                    uint8_t *buffer =   audioData->getData();

        int size   = audioData->getSize();

        if ( NULL != buffer && 0 != size) {
            SLresult result;
            // enqueue another buffer
            result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer, size);
            // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
            // which for this code example would indicate a programming error
            if (SL_RESULT_SUCCESS != result) {
                pthread_mutex_unlock(&audioEngineLock);
            }
            (void)result;
        } else {
            pthread_mutex_unlock(&audioEngineLock);
        }
        }




    }


}

void createAudioEngine() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);



    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);



    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);



    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);

    }

}



void createBufferQueueAudioPlayer(int sampleRate, int channel ,AudioDataProvider *audioDataProvider) {
    SLresult result;
    if (sampleRate >= 0 ) {
        bqPlayerSampleRate = sampleRate*1000 ;
    }

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    if(bqPlayerSampleRate) {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in mili second
    }
    format_pcm.numChannels = (SLuint32) channel;
    if (channel == 2) {
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }else {
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};


    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};


    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
         };
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            };
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                bqPlayerSampleRate? 2 : 3, ids, req);



    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);



    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);


    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);


    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, audioDataProvider);


    bqPlayerEffectSend = NULL;
    if( 0 == bqPlayerSampleRate) {
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                                 &bqPlayerEffectSend);

    }

#if 0
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
#endif


    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);


}


void AudioDataProvider:: initAudioPlayer(int sampleRate, int channel) {

    createAudioEngine();
    createBufferQueueAudioPlayer(sampleRate, channel ,this);
}

   void  AudioDataProvider:: startAudioPlay() {

    SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    bqPlayerCallback(bqPlayerBufferQueue,(void *) this);
}

void AudioDataProvider:: stopAudioPlay() {
    assert((*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED) == SL_RESULT_SUCCESS);
}
void AudioDataProvider::put(AudioData *audioData) {


    this->queue.push(audioData);
}