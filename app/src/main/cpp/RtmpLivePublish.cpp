#include "RtmpLivePublish.h"



#define  TAG "ffmpeg"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)

RtmpLivePublish::RtmpLivePublish() {
}

RtmpLivePublish::~RtmpLivePublish() {
}

/**
 * 初始化RTMP数据，与rtmp连接
 * @param url
 */
void RtmpLivePublish::init(unsigned char * url) {



    this->rtmp_url = url ;
    rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);

    rtmp->Link.timeout =5;
    RTMP_SetupURL(rtmp, (char *)url);
    RTMP_EnableWrite(rtmp);
    int  a =  RTMP_Connect(rtmp, NULL);
    LOGI("RTMP_Connect error=%d" , a );

    //建立RTMP socket连接
    if (!a  ) {
        LOGI("RTMP_Connect error");

    } else {
        LOGI("RTMP_Connect success.");
    }
    //连接到rtmp流上
    if (!RTMP_ConnectStream(rtmp, 0)) {
        LOGI("RTMP_ConnectStream error");

    } else {
        LOGI("RTMP_ConnectStream success.");
    }
    start_time = RTMP_GetTime();


    LOGI(" start_time = %d", start_time);
}

/**
 * 发送sps pps，即发送H264SequenceHead头
 * @param sps
 * @param sps_len
 * @param pps
 * @param pps_len
 */
void RtmpLivePublish::addSequenceH264Header(unsigned char *sps, int sps_len, unsigned char *pps, int pps_len) {
    int bodySize = sps_len + pps_len + 16;

    RTMPPacket *packet = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
    RTMPPacket_Alloc(packet, bodySize);
    RTMPPacket_Reset(packet);

    unsigned char* body = (unsigned char*)packet->m_body;

    int i = 0;
    /*1:keyframe 7:AVC*/
    body[i++] = 0x17;
    /* AVC head */
    body[i++] = 0x00;

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    body[i++] = 0x01;
    body[i++] = sps[1];

    body[i++] = sps[2];
    //AVCLevelIndication sps[3]
    body[i++] = sps[3];
    //6bit的reserved为二进制位111111和2bitlengthSizeMinusOne一般为3，
    //二进制位11，合并起来为11111111，即为0xff
    body[i++] = 0xff;

    /*sps*/
    //3bit的reserved，二进制位111，5bit的numOfSequenceParameterSets，
    //sps个数，一般为1，及合起来二进制位11100001，即为0xe1
    body[i++]   = 0xe1;
    //SequenceParametersSetNALUnits（sps_size + sps）的数组
    body[i++] = (sps_len >> 8) & 0xff;
    body[i++] = sps_len & 0xff;
    memcpy(&body[i], sps, sps_len);
    i +=  sps_len;

    /*pps*/
    //numOfPictureParameterSets一般为1，即为0x01
    body[i++]   = 0x01;
    //SequenceParametersSetNALUnits（pps_size + pps）的数组
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = (pps_len) & 0xff;
    memcpy(&body[i], pps, pps_len);


    //Message Type，RTMP_PACKET_TYPE_VIDEO：0x09
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    //Payload Length
    packet->m_nBodySize = bodySize;
    //Time Stamp：4字节
    //记录了每一个tag相对于第一个tag（File Header）的相对时间。
    //以毫秒为单位。而File Header的time stamp永远为0。
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    //Channel ID，Audio和Vidio通道
    packet->m_nChannel = 0x04;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = rtmp->m_stream_id;
    //send rtmp
    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);
        LOGD("send packet sendSpsAndPps");
    }
    free(packet);
}

/**
 * 发送H264数据
 * @param buf
 * @param len
 * @param timeStamp
 */
void RtmpLivePublish::addH264Body(unsigned char *buf, int len, long timeStamp  , bool f) {

    int bodySize = len + 9;

    RTMPPacket *packet = static_cast<RTMPPacket *> (malloc(sizeof(RTMPPacket)));
    RTMPPacket_Alloc(packet, bodySize);
    RTMPPacket_Reset(packet);
    unsigned char *body = (unsigned char*)packet->m_body;
    int  i  = 0;
    if ( f) {
        //00 00 00 01
        body[i++] = 0x17;
    } else  {
        // 00 00 01
        body[i++] = 0x27;
    }

    body[i++] = 0x01;

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    //写入NALU信息，右移8位，一个字节的读取
    body[i++] = (len >> 24) & 0xff;
    body[i++] = (len >> 16) & 0xff;
    body[i++] = (len >> 8) & 0xff;
    body[i++] = (len) & 0xff;

    /*copy data*/
    memcpy(&body[i], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    //当前packet的类型：Video
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x04;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nInfoField2 = rtmp->m_stream_id;

    packet->m_nTimeStamp = RTMP_GetTime() - start_time;

    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);

    }
    free(packet);
}

int RtmpLivePublish::getSampleRateIndex(int sampleRate) {
    int sampleRateArray[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
    for( int i = 0 ; i < 13 ; ++i ) {
        if (sampleRateArray[i] == sampleRate) {
            return i;
        }
    }
    return -1 ;
}

/**
 * 发送AAC Sequence HEAD 头数据
 * @param sampleRate
 * @param channel
 * @param timestamp
 */
void RtmpLivePublish::addSequenceAacHeader(int sampleRate, int channel, int timestamp) {
    int body_size = 2 + 2;
    RTMPPacket *packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 4);
    memset(packet,0,RTMP_HEAD_SIZE);

    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    unsigned char * body = (unsigned char *)packet->m_body;
    //头信息配置
    /*AF 00 + AAC RAW data*/
    body[0] = 0xAF;
    //AACPacketType:0表示AAC sequence header
    body[1] = 0x00;

    uint16_t audioConfig = 0 ;
    //这里的2表示对应的是AAC-LC 由于是5个bit，左移11位，变为16bit，2个字节
    //与上一个1111100000000000(0xF800)，即只保留前5个bit
    audioConfig |= ((2 << 11) & 0xF800) ;

    int sampleRateIndex = getSampleRateIndex( sampleRate ) ;
    if( -1 == sampleRateIndex ) {
        free(packet);
        packet = NULL;
        LOGE("addSequenceAacHeader: no support current sampleRate[%d]" , sampleRate);
        return;
    }

    //sampleRateIndex为4，二进制位0000001000000000 & 0000011110000000(0x0780)（只保留5bit后4位）
    audioConfig |= ((sampleRateIndex << 7) & 0x0780) ;
    //sampleRateIndex为4，二进制位000000000000000 & 0000000001111000(0x78)（只保留5+4后4位）
    audioConfig |= ((channel << 3) & 0x78) ;
    //最后三个bit都为0保留最后三位111(0x07)
    audioConfig |= (0 & 0x07) ;
    //最后得到合成后的数据0001001000010000，然后分别取这两个字节

    body[2] = ( audioConfig >> 8 ) & 0xFF ;
    body[3] = ( audioConfig & 0xFF );

    //参数设置
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nTimeStamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = rtmp->m_stream_id;

    //send rtmp aac head
    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);
        //LOGD("send packet sendAacSpec");
    }
    free(packet);
}

/**
 * 发送rtmp AAC data
 * @param buf
 * @param len
 * @param timeStamp
 */
void RtmpLivePublish::addAccBody(unsigned char *buf, int len, long timeStamp) {
    int body_size = 2 + len;
    RTMPPacket * packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + len + 2);
    memset(packet, 0, RTMP_HEAD_SIZE);

    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    unsigned char * body = (unsigned char *)packet->m_body;

    //头信息配置
    /*AF 00 + AAC RAW data*/
    body[0] = 0xAF;
    //AACPacketType:1表示AAC raw
    body[1] = 0x01;
    /*spec_buf是AAC raw数据*/
    memcpy(&body[2], buf, len);
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = RTMP_GetTime() - start_time;
    //LOGI("aac m_nTimeStamp = %d", packet->m_nTimeStamp);
    packet->m_nInfoField2 = rtmp->m_stream_id;

    //send rtmp aac data
    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);
        //LOGD("send packet sendAccBody");
    }
    free(packet);
}

void RtmpLivePublish::release()
{
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
}