//
// Created by Administrator on 2020/5/22.
//

#include "VideoChannel.h"
#include "include/x264.h"

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    mWidth=width;
    mHeight=height;
    mFps=fps;
    mBitrate=bitrate;
    ySize=width*height;
    uvSize=ySize/4;
    //初始化x264的编码器
    //编码
    x264_param_t param;
    //初始化编码器 编码速度最快  编码质量零延时
    x264_param_default_preset(&param,"ultrafast","zerolatency");
    //base_line 3.2编码负责度
    param.i_level_idc=32;
    //服务器支持I420 输入数据格式 NV21
    param.i_csp=X264_CSP_I420;

    param.i_width=width;
    param.i_height=height;
    //无b帧 首开
    param.i_bframe=0;
    //参数i_rc_method表示码率控制，CQP(恒定质量).CRF(恒定码率)，ABR（平均码率）
    param.rc.i_rc_method=X264_RC_ABR;
    //码率
    param.rc.i_bitrate=bitrate/1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate=bitrate/1000*1.2;
    //设置了i_vbv_max_bitrate必须设置此参数，码c率控制区大小，单位kbps
    param.rc.i_vbv_buffer_size=bitrate/1000;

    param.i_fps_num=fps;
    param.i_fps_den=1;
    //帧率 不是直接设置，需要换算出来
    param.i_timebase_den=param.i_fps_num;
    param.i_timebase_num=param.i_fps_den;
    //用fps而不是时间错来计算帧间距离
    param.b_vfr_input=0;
    //帧距离 2s一个关键帧
    param.i_keyint_max=fps*2;
    //是否复制sps和pps放在每个关键帧的前面，该参数设置是让每个关键帧I帧都附带sps/pps
    //sps和pps 包含了初始化H.264解码器所需要的信息参数，包括编码所用的profile，level，图像的宽和高。deblock滤波器等。相当于http请求附带的数据信息吧，
    //sps：序列参数集
    //pps：图像参数集
    param.b_repeat_headers=1;
    //多线程 0：多线程 1：单线程
    param.i_threads=1;
    //编码质量
    x264_param_apply_profile(&param,"baseline");
    //打开编码器
    videoCodec = x264_encoder_open(&param);
    pic_in=new x264_picture_t;
    x264_picture_alloc(pic_in,X264_CSP_I420,width,height);
}

void VideoChannel::encodeData(int8_t *data) {
    //解码 nv21 yuvI420
    //数据data 容器pic_in
    memcpy(pic_in->img.plane[0],data,ySize);
    //pic_in->img.plane;
    for (int i = 0; i < uvSize; ++i) {
        //v数据
        *(pic_in->img.plane[1]+i) = *(data+ySize+i*2+1);
        *(pic_in->img.plane[2]+i) = *(data+ySize+i*2);


    }

}


