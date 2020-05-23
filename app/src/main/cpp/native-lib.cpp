#include <jni.h>
#include <string>
#include <pthread.h>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
#include "macro.h"
#include "safe_queue.h"
VideoChannel *videoChannel;
AudioChannel *audioChannel;
int isStart = 0;
int readyPushing = 0;
pthread_t pid;
SafeQueue<RTMPPacket *> packets;
//同步音视频的关键，
uint32_t start_time;

void callback(RTMPPacket *packet){
    if (packet){
        packet->m_nTimeStamp =RTMP_GetTime()-start_time;
        //加入队列
        packets.put(packet);
    }
}
void releasePackets(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

void *start(void *args) {
    char *url = static_cast<char *> (args);
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("alloc rtmp 初始化失败");
        return NULL;
    }
    RTMP_Init(rtmp);
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("alloc rtmp 设置地址失败%s", url);
        return NULL;
    }
    rtmp->Link.timeout = 5;
    RTMP_EnableWrite(rtmp);
    ret = RTMP_Connect(rtmp, 0);
    if (!ret) {
        LOGE("连接服务器:%s", url);
        return NULL;
    }
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接流:%s", url);
        return NULL;
    }
    start_time = RTMP_GetTime();
    //准备开始推流
    readyPushing = 1;
    packets.setWork(1);
    RTMPPacket *packet = 0;
    callback(audioChannel->getAudioTag());
    while (readyPushing) {
        packets.get(packet);
        LOGE("取出一帧数据");
        if (!readyPushing) {
            break;
        }
        if (!packet) {
            continue;
        }
        packet->m_nInfoField2 = rtmp->m_stream_id;
        ret = RTMP_SendPacket(rtmp, packet, 1);
        releasePackets(packet);
    }

    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete (url);
    return 0;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_anlu_demondk_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    x264_picture_t *x264Picture = new x264_picture_t();

    RTMP_Alloc();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_anlu_demondk_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);
    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(callback);


}extern "C"
JNIEXPORT void JNICALL
Java_com_anlu_demondk_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject thiz, jint width,
                                                         jint height, jint fps, jint bitrate) {
    if (!videoChannel) {

        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);

}extern "C"
JNIEXPORT void JNICALL
Java_com_anlu_demondk_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {
    //推流 path是url地址
    const char *path = env->GetStringUTFChars(path_, 0);
    if (isStart) {
        return;
    }
    isStart = 1;
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);

    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);

}extern "C"
JNIEXPORT void JNICALL
Java_com_anlu_demondk_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray data_) {

    if (!videoChannel || !readyPushing) {
        return;
    }
   jbyte  *date = env->GetByteArrayElements(data_,NULL);

    videoChannel->encodeData(date);

    env->ReleaseByteArrayElements(data_,date,0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_anlu_demondk_LivePusher_native_1pushAudio(JNIEnv *env, jobject thiz, jbyteArray bytes_) {

    jbyte *data = env->GetByteArrayElements(bytes_,NULL);

    if (!audioChannel||!readyPushing){
        return;
    }
    audioChannel->encodeData(data);
    env->ReleaseByteArrayElements(bytes_,data,0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_anlu_demondk_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject thiz, jint sampleRateInHz,
                                                         jint channels) {
    if (audioChannel){
        audioChannel->setAudioEncInfo(sampleRateInHz,channels);
    }


}extern "C"
JNIEXPORT jint JNICALL
Java_com_anlu_demondk_LivePusher_getInputSamples(JNIEnv *env, jobject thiz) {
        if (audioChannel){
            return audioChannel->getInputSamples();
        }
    return -1;

}