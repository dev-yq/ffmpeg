package com.live.ffmpeg.ffmpeg;

/**
 * @作者： $User$
 * @时间： $date$
 * @描述：
 */
public class FFmpegUtils {

    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
        System.loadLibrary("avfilter");
        System.loadLibrary("avdevice");
    }
    public static native int init(int width, int height, int outWidth, int outHeight);

    public static native int release();

    public static native int yuvI420ToNV21(byte[] i420Src, byte[] nv21Src, int width, int height);

    /**
     * NV21转化为YUV420P数据
     * @param src         原始数据
     * @param width       原始数据宽度
     * @param height      原始数据高度
     * @param dst         生成数据
     * @param dst_width   生成数据宽度
     * @param dst_height  生成数据高度
     * @param mode        模式
     * @param degree      角度
     * @param isMirror    是否镜像
     * @return
     */
    public static native int compressYUV(byte[] src, int width, int height, byte[] dst, int dst_width, int dst_height, int mode, int degree, boolean isMirror);

    /**
     * YUV420P数据的裁剪
     * @param src         原始数据
     * @param width       原始数据宽度
     * @param height      原始数据高度
     * @param dst         生成数据
     * @param dst_width   生成数据宽度
     * @param dst_height  生成数据高度
     * @param left        裁剪的起始x点
     * @param top         裁剪的起始y点
     * @return
     */
    public static native int cropYUV(byte[] src, int width, int height, byte[] dst, int dst_width, int dst_height, int left, int top);

    /**
     * 编码视频数据准备工作
     * @param in_width
     * @param in_height
     * @param out_width
     * @param out_height
     * @return
     */
    public static native int encoderVideoinit(int in_width, int in_height, int out_width, int out_height);

    /**
     * 编码视频数据接口
     * @param srcFrame      原始数据(YUV420P数据)
     * @param frameSize     帧大小
     * @param fps           fps
     * @param dstFrame      编码后的数据存储
     * @param outFramewSize 编码后的数据大小
     * @return
     */
    public static native int encoderVideoEncode(byte[] srcFrame, int frameSize, int fps, byte[] dstFrame, int[] outFramewSize);

    /**
     *
     * @param sampleRate 音频采样频率
     * @param channels   音频通道
     * @param bitRate    音频bitRate
     * @return
     */
    public static native int encoderAudioInit(int sampleRate, int channels, int bitRate);

    public static native int encoderAudioEncode(byte[] srcFrame, int frameSize, byte[] dstFrame, int dstSize);

    /**
     * 初始化RMTP，建立RTMP与RTMP服务器连接
     * @param url
     * @return
     */
    public static native int initRtmpData(String url);

    /**
     * 发送SPS,PPS数据
     * @param sps       sps数据
     * @param spsLen    sps长度
     * @param pps       pps数据
     * @param ppsLen    pps长度
     * @param timeStamp 时间戳
     * @return
     */
    public static native int sendRtmpVideoSpsPPS(byte[] sps, int spsLen, byte[] pps, int ppsLen, long timeStamp);

    /**
     * 发送视频数据，再发送sps，pps之后
     * @param data
     * @param dataLen
     * @param timeStamp
     * @return
     */
    public static native int sendRtmpVideoData(byte[] data, int dataLen, long timeStamp);

    /**
     * 发送AAC Sequence HEAD 头数据
     * @param timeStamp
     * @return
     */
    public static native int sendRtmpAudioSpec(long timeStamp);

    /**
     * 发送AAC音频数据
     * @param data
     * @param dataLen
     * @param timeStamp
     * @return
     */
    public static native int sendRtmpAudioData(byte[] data, int dataLen, long timeStamp);

    /**
     * 释放RTMP连接
     * @return
     */
    public static native int releaseRtmp();





    //推流，将Y、U、V数据分开传递
    public static native  int pushCameraData(byte[] buffer,int ylen,byte[] ubuffer,int ulen,byte[] vbuffer,int vlen);

    //结束
    public static native int close();
}
