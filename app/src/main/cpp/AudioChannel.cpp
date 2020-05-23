//
// Created by mayn on 2020/5/23.
//

#include <pty.h>
#include <cstring>
#include "AudioChannel.h"
#include "librtmp/rtmp.h"
//编码器信息作为首帧
RTMPPacket *AudioChannel::getAudioTag(){
    u_char *buf;
    u_long len;
    faacEncGetDecoderSpecificInfo(audioCodec,&buf,&len);
    int bodySize =2 +len;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet,bodySize);
    packet->m_body[0] = 0xAF;
    if (mChannels ==1){
        packet->m_body[0] = 0xAE;
    }
    packet->m_body[1] = 0x00;

    //编码之后的aac数据 内容 不固定
    memcpy(&packet->m_body[2],buf,len);

    //aac 表示相对时间
    packet->m_hasAbsTimestamp=0;
    packet->m_nBodySize=bodySize;
    packet->m_packetType=RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel=0x11;
    packet->m_headerType=RTMP_PACKET_SIZE_LARGE;
    return packet;
}

//不断调用
void AudioChannel::encodeData(int8_t *data) {
    int bytelen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), intputSamples,
                                buffer, maxOutputBytes);
    if (bytelen > 0) {
        RTMPPacket *packet = new RTMPPacket;
        int bodySize  =2 +bytelen;
        RTMPPacket_Alloc(packet,bytelen);
        packet->m_body[0] = 0xAF;
        if (mChannels == 1) {
            packet->m_body[0] = 0xAE;
        }
        //编码出的声音 都是 0x01
        packet->m_body[1] = 0x01;
        //编码之后的aac数据 内容 不固定
        memcpy(&packet->m_body[2],buffer,bytelen);

        //aac 表示相对时间
        packet->m_hasAbsTimestamp=0;
        packet->m_nBodySize=bodySize;
        packet->m_packetType=RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel=0x11;
        packet->m_headerType=RTMP_PACKET_SIZE_LARGE;
        audioCallback(packet);
    }
}

void AudioChannel::setAudioCallback(AudioChannel::AudioCallback audioCallback) {
    this->audioCallback=audioCallback;
}

//初始化
void AudioChannel::setAudioEncInfo(int smplesInHZ, int channels) {
    //最小缓冲大小
    audioCodec = faacEncOpen(smplesInHZ, channels, &intputSamples, &maxOutputBytes);
    //设置参数
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);
    //
    config->mpegVersion = MPEG4;
    //1c标准
    config->aacObjectType = LOW;
    //16位
    config->inputFormat = FAAC_INPUT_16BIT;
    config->outputFormat = 0;
    faacEncSetConfiguration(audioCodec, config);
    buffer = new u_char[maxOutputBytes];
}

int AudioChannel::getInputSamples() {
    return intputSamples;
}
