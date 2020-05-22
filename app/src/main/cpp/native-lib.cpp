#include <jni.h>
#include <string>
#include "x264.h"
#include "librtmp/rtmp.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_anlu_demondk_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    x264_picture_t *x264Picture = new x264_picture_t();

    RTMP_Alloc();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
