#ifndef RIEMANNLEELIVEPROJECT_FRAMEENCODER_H
#define RIEMANNLEELIVEPROJECT_FRAMEENCODER_H


#include <cstdio>
#include "yuv/x264.h"

extern "C" {

#include "ffmpeg/libavutil/pixfmt.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include <yuv2rgb.h>
}

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

class FrameEncoder {

private:
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    /* e.g. 25, 60, etc.. */
    int fps;
    int bitrate;
    int i_threads;
    int i_vbv_buffer_size;
    int i_slice_max_size;
    int b_frame_frq;




public:
    FrameEncoder();
    ~FrameEncoder();
    /* open for encoding */
    bool open();
    /* encode the given data */
    int encodeFrame(char* inBytes, int frameSize, int pts, char* outBytes, int *outFrameSize);
    /* close the encoder and file, frees all memory */
    bool close();
    /* validates if all params are set correctly, like width,height, etc.. */
    bool validateSettings();
    /* sets the x264 params */
    x264_param_t * setParams();

    void setInHeight(int inHeight);

    void setInWidth(int inWidth);


    void setOutHeight(int outHeight);

    void setOutWidth(int outWidth);
    int getBitrate() const;
    void setBitrate(int bitrate);

};


#endif //RIEMANNLEELIVEPROJECT_FRAMEENCODER_H
