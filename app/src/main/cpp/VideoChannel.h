//
// Created by Administrator on 2020/5/22.
//

#ifndef NDKDEMO_VIDEOCHANNEL_H
#define NDKDEMO_VIDEOCHANNEL_H


class VideoChannel {

public:
    void setVideoEncInfo(jint width, jint height, jint fps, jint bitrate);


    void encodeData(int8_t *data);

private:int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;//当前的帧有多少个Y
    int uvSize;//当前的帧有多少的UV
    x264_t *videoCodec;//当前的帧有多少的UV
    //一帧
    x264_picture_t *pic_in;
};


#endif //NDKDEMO_VIDEOCHANNEL_H
