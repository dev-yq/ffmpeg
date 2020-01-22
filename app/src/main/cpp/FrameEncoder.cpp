#include <android/log.h>
#include <cstdint>
#include <cstring>
#include "FrameEncoder.h"


extern "C" {
#include "ffmpeg/libswscale/swscale.h"
#include "yuv/yuv2rgb.h"
}

#define  TAG "RiemannLee"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

//供测试文件使用,测试的时候打开
//#define ENCODE_OUT_FILE_1
//供测试文件使用
//#define ENCODE_OUT_FILE_2

FrameEncoder::FrameEncoder() : in_width(0), in_height(0), out_width(
        0), out_height(0), fps(0){

#ifdef ENCODE_OUT_FILE_1
    const char *outfile1 = "/sdcard/2222.h264";
    out1 = fopen(outfile1, "wb");
#endif

#ifdef ENCODE_OUT_FILE_2
    const char *outfile2 = "/sdcard/3333.h264";
    out2 = fopen(outfile2, "wb");
#endif
}

FrameEncoder::~FrameEncoder() {
}

bool FrameEncoder::open() {
    int r = 0;


    // write headers

    if (r < 0) {
        LOGI("x264_encoder_headers() failed");
        return false;
    }
    return true;
}

int FrameEncoder::encodeFrame(char* inBytes, int frameSize, int pts,
                               char* outBytes, int *outFrameSize) {


    int i420_y_size = 640 * 480;
    int i420_u_size = (640 >> 1) * (480 >> 1);
    int i420_v_size = frameSize-i420_y_size -i420_u_size;
    uint8_t *i420_y_data = (uint8_t *)inBytes;
    uint8_t *i420_u_data = (uint8_t *)inBytes + i420_y_size;
    uint8_t *i420_v_data = (uint8_t *)inBytes + i420_y_size + i420_u_size;
    x264_param_t *params   = setParams();
    x264_t *encoder = x264_encoder_open(params);
    x264_picture_t* pic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    x264_picture_t* pic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    x264_picture_init(pic_out);
    x264_picture_alloc(pic_in, params->i_csp,params->i_width,params->i_height);

    memcpy(pic_in->img.plane[0], i420_y_data, i420_y_size);
    memcpy(pic_in->img.plane[1], i420_u_data, i420_u_size);
    memcpy(pic_in->img.plane[2], i420_v_data, i420_v_size);
//
    pic_in->i_pts = pts;
    int num_nals=0;
    x264_nal_t* nals;
    if (fps==1)
    {
        x264_encoder_headers(encoder, &nals, &num_nals);
    }
    int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, pic_in,pic_out);


    if (frame_size) {
        int have_copy = 0;
        for (int i = 0; i < num_nals; i++) {
            outFrameSize[i] = nals[i].i_payload;
            memcpy(outBytes + have_copy, nals[i].p_payload, nals[i].i_payload);
            have_copy += nals[i].i_payload;
        }


    }
    
    x264_picture_clean(pic_in);

    x264_encoder_close(encoder);

    free(pic_in);

    free(pic_out);
    free(params);

    return num_nals;

}

bool FrameEncoder::close() {





#ifdef ENCODE_OUT_FILE_1
    if (out1) {
        fclose(out1);
    }
#endif
#ifdef ENCODE_OUT_FILE_2
    if (out2) {
        fclose(out2);
    }
#endif

    return true;
}

  x264_param_t*  FrameEncoder::setParams() {

    x264_param_t* params = (x264_param_t*)malloc(sizeof(x264_param_t));
      x264_param_default(params);
    x264_param_default_preset(params, "ultrafast", "zerolatency");
    //I帧间隔
    params->i_csp = X264_CSP_I420;

    params->i_width = 480;
    params->i_height = 640;
    params->i_threads = 1;
    params->i_fps_num =30;//getFps();
    params->i_fps_den = 1;
    // Intra refres:
    params->i_keyint_max = 20;
      params->b_vfr_input = 0;



      params->analyse.b_transform_8x8 = 1;
      params->rc.f_aq_strength = 1.5;

      params->rc.i_aq_mode = 0;
      params->rc.f_qcompress = 0.0;
      params->rc.f_ip_factor = 0.5;
      params->rc.f_rate_tolerance = 0.1;

      params->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
      params->analyse.i_me_method = X264_ME_DIA;
      params->analyse.i_me_range = 16;
      params->analyse.i_subpel_refine = 2;
      // pParameter->analyse.i_noise_reduction = 1;

      params->i_log_level     = X264_LOG_DEBUG;
    params->i_keyint_min = 1;
    params->rc.i_bitrate = 200 ;
    params->rc.i_rc_method       = X264_RC_ABR;
    x264_param_apply_profile(params, "baseline");
    return params;
}













