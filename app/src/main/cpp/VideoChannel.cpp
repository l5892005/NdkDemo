//
// Created by Administrator on 2020/5/22.
//

#include <cstring>
#include "VideoChannel.h"
#include "include/x264.h"
#include "librtmp/rtmp.h"
#include "macro.h"
//发送关键帧
void VideoChannel::sendFrame(int type, uint8_t *payload, int i_payload) {
    // 如果边界分割符是0x00 0x00 0x00 0x01 就-4
    if (payload[2] == 0x00) {
        i_payload -= 4;
        payload += 4;
    } else {
        // 如果边界分割符是 0x00 0x00 0x01 就-3
        i_payload -= 3;
        payload += 3;
    }
    //看表
// 编码格式：固定头+类型+4字节数据长度+h264裸数据
// 1 + 4 + 4 + 内容 = 9 + 内容
    int bodySize = 9 + i_payload;
    RTMPPacket *packet = new RTMPPacket;
    //
    RTMPPacket_Alloc(packet, bodySize);

    // 关键帧或非关键帧数据

    // 固定头：关键帧为0x17 非关键帧为0x27
    packet->m_body[0] = 0x27;
    if (type == NAL_SLICE_IDR) {
        packet->m_body[0] = 0x17;
        LOGE("关键帧");
    }
    // 类型为4个字节，0x01 0x00 0x00 0x00
    packet->m_body[1] = 0x01;
    // 时间戳
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;
    // 数据长度 int 4个字节 依次取每个字节
    packet->m_body[5] = (i_payload >> 24) & 0xff; // 取第一个8位
    packet->m_body[6] = (i_payload >> 16) & 0xff; // 取第二个8位
    packet->m_body[7] = (i_payload >> 8) & 0xff;  // 取第三个8位
    packet->m_body[8] = (i_payload) & 0xff;       // 取第四个8位
    //图片数据，h264裸数据
    memcpy(&packet->m_body[9], payload, i_payload);

    // 关键帧或非关键帧数据完毕

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    videoCallback(packet);
}
void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;
    ySize = width * height;
    uvSize = ySize / 4;
    //初始化x264的编码器
    //编码
    x264_param_t param;
    //初始化编码器 编码速度最快  编码质量零延时
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    //base_line 3.2编码负责度
    param.i_level_idc = 32;
    //服务器支持I420 输入数据格式 NV21
    param.i_csp = X264_CSP_I420;

    param.i_width = width;
    param.i_height = height;
    //无b帧 首开
    param.i_bframe = 0;
    //参数i_rc_method表示码率控制，CQP(恒定质量).CRF(恒定码率)，ABR（平均码率）
    param.rc.i_rc_method = X264_RC_ABR;
    //码率
    param.rc.i_bitrate = bitrate / 1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    //设置了i_vbv_max_bitrate必须设置此参数，码c率控制区大小，单位kbps
    param.rc.i_vbv_buffer_size = bitrate / 1000;

    param.i_fps_num = fps;
    param.i_fps_den = 1;
    //帧率 不是直接设置，需要换算出来
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;
    //用fps而不是时间错来计算帧间距离
    param.b_vfr_input = 0;
    //帧距离 2s一个关键帧
    param.i_keyint_max = fps * 2;
    //是否复制sps和pps放在每个关键帧的前面，该参数设置是让每个关键帧I帧都附带sps/pps
    //sps和pps 包含了初始化H.264解码器所需要的信息参数，包括编码所用的profile，level，图像的宽和高。deblock滤波器等。相当于http请求附带的数据信息吧，
    //sps：序列参数集
    //pps：图像参数集
    param.b_repeat_headers = 1;
    //多线程 0：多线程 1：单线程
    param.i_threads = 1;
    //编码质量
    x264_param_apply_profile(&param, "baseline");
    //打开编码器
    videoCodec = x264_encoder_open(&param);
    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
}
void VideoChannel::setVideoCallback(VideoCallback videoCallback) {
    this->videoCallback=videoCallback;
}

void VideoChannel::encodeData(int8_t *data) {
    //解码 nv21 yuvI420
    //数据data 容器pic_in
    //y数据  pic_in->img.plane
    memcpy(pic_in->img.plane[0], data, ySize);
    //pic_in->img.plane;
    for (int i = 0; i < uvSize; ++i) {
        //数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1);
        //UVVB据
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2);
    }
    // 现在已经转换成i420格式了，然后下面开始使用x264压缩
    //NALU单元
    x264_nal_t *pp_nal;
    //编码出来有几个数据 （多少NALU单元）
    int pi_nal;
    x264_picture_t pic_out;
    x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, &pic_out);
    int sps_len;
    int pps_len;
    uint8_t sps[100];
    uint8_t pps[100];
    // 遍历NALU单元
    for (int i = 0; i < pi_nal; ++i) {
        if (pp_nal[i].i_type == NAL_SPS) {
            //sps的长度 00 00  00 01
            // sps的开头是 00 00 00 01，所以内容区域要从第5个位置开始copy
            // p_payload 是该NAL单元存储数据的开始地址
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);
        } else if (pp_nal[i].i_type == NAL_PPS) {
            pps_len = pp_nal[i].i_payload - 4;
            // pps的开头是 00 00 00 01，所以内容区域要从第5个位置开始copy
            memcpy(pps, pp_nal[i].p_payload + 4, sps_len);
            sendSpsPPs(sps,pps,sps_len,pps_len);
        }else{
            //pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload
            sendFrame(pp_nal[i].i_type,pp_nal[i].p_payload,pp_nal[i].i_payload);
        }
    }
}
//格式 RTMP
void VideoChannel::sendSpsPPs(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    RTMPPacket *packet=new RTMPPacket;
    int bodySize = 13+sps_len+3+pps_len;
    RTMPPacket_Alloc(packet,bodySize);


    int i = 0;
    // 表示当前数据为sps或者pps 固定头
    packet->m_body[i++] = 0x17;
    // 类型固定写法 如果是关键帧或者非关键帧，则为0x01 0x00 0x00 0x00
    packet->m_body[i++] = 0x00;
    //composition time 0x000000
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    // 下面为sps+pps数据区

    // configurationVersion 版本
    packet->m_body[i++] = 0x01;
    // 编码规格 00 00 00 01 67 64 00 33 AC ...
    // 00 00 00 01表示边界符
    // 67 表示是sps类型 68 表示是pps 65 表示是关键帧
    // sps[1] 表示 avcProfileIndication = 64
    // sps[2] 表示 profile_compatibility = 00 // 00 表示 兼容所有
    // sps[3] 表示 profile_level = 33 // 33 表示编码级别
    // avcProfileIndication
    packet->m_body[i++] = sps[1];
    // profile_compatibility 兼容性
    packet->m_body[i++] = sps[2];
    // profile_level 编码级别
    packet->m_body[i++] = sps[3];
    // lengthSizeMinusOne 0xff包长数据所使用的字节数，通常为0xff
    packet->m_body[i++] = 0xFF;

    //整个sps
    // numOfSequenceParameterSets sps个数，通常为0xe1
    packet->m_body[i++] = 0xE1;
    //sps长度 int长度是4个字节 把4个字节转换成2个字节
    // sequenceParameterSetLength sps长度
    packet->m_body[i++] = (sps_len >> 8) & 0xff;// 先取高8位
    packet->m_body[i++] = sps_len & 0xff; // 再取低8位
    // sps内容
    memcpy(&packet->m_body[i], sps, sps_len);

    // pps
    i += sps_len;
    // numOfPictureParameterSets pps个数，通常为0x01
    packet->m_body[i++] = 0x01;
    // pictureParameterSetNALUnits pps长度
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    // pps内容
    memcpy(&packet->m_body[i], pps, pps_len);

    // sps+pps数据区组合完成

    //视频
    // 表明整个packet是video，还有audio，还有字幕
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySize;
    //随意分配一个管道（尽量避开rtmp.c中使用的）
    packet->m_nChannel = 10;
    //sps pps没有时间戳
    packet->m_nTimeStamp = 0;
    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    if(videoCallback){
        videoCallback(packet);
    }

}


