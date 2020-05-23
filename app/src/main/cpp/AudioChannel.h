//
// Created by mayn on 2020/5/23.
//

#ifndef DEMONDK_AUDIOCHANNEL_H
#define DEMONDK_AUDIOCHANNEL_H


#include <jni.h>
#include "librtmp/rtmp.h"
#include "faac.h"
class AudioChannel {

    typedef void (*AudioCallback) (RTMPPacket *packet);
public:
    void encodeData(int8_t *data);
    void setAudioEncInfo(int samplesInHZ,int channels);

    jint getInputSamples();
    void setAudioCallback(AudioCallback audioCallback);
    RTMPPacket *getAudioTag();
private:
    AudioCallback audioCallback;
    int mChannels;
    faacEncHandle  audioCodec;
    u_long intputSamples;
    u_long maxOutputBytes;
    u_char *buffer =0;


};



#endif //DEMONDK_AUDIOCHANNEL_H
