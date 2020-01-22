
#ifndef FFMPEG3_AUDIODATAPROVIDER_H
#define FFMPEG3_AUDIODATAPROVIDER_H
#include <stdint.h>

#include<queue>

extern  "C"{
#include <jni.h>
    #include "AudioData.hpp"
    #include <android/log.h>
    #include "aac/debug.h"
    #include <SLES/OpenSLES.h>
    #include <pthread.h>
    #include <assert.h>
    #include <SLES/OpenSLES_Android.h>

}
using   namespace std;
class  AudioDataProvider {
public:
    AudioDataProvider ();
    ~AudioDataProvider() ;
    std::queue<AudioData *>  queue;
    void initAudioPlayer(int sampleRate, int channel);
    void startAudioPlay();
    void stopAudioPlay();



    bool   isPlayer   = false;
    int   size  =10;
     void  put(AudioData * audioData);
};


#endif //FFMPEG3_AUDIODATAPROVIDER_H
