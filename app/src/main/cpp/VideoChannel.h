//
// Created by Administrator on 2020/5/22.
//

#ifndef NDKDEMO_VIDEOCHANNEL_H
#define NDKDEMO_VIDEOCHANNEL_H


#include <x264.h>
#include "librtmp/rtmp.h"

class VideoChannel {
//C的回调
    typedef void  (*VideoCallback)(RTMPPacket* packet);
public:
    void setVideoEncInfo(int width, int height, int fps, int bitrate);


    void encodeData(int8_t *data);

    void setVideoCallback(VideoCallback videoCallback);
    void sendFrame(int type, uint8_t *payload, int i_payload);
private:int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;//当前的帧有多少个Y
    int uvSize;//当前的帧有多少的UV
    x264_t *videoCodec;//当前的帧有多少的UV
    //一帧
    x264_picture_t *pic_in;
    VideoCallback videoCallback;
    void sendSpsPPs(uint8_t *sps, uint8_t *pps, int len, int ppsLen);
};


#endif //NDKDEMO_VIDEOCHANNEL_H
