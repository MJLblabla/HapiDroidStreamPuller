#include <jni.h>
#include <string>
#include "JNIRTMPConnection.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_native_1init(JNIEnv *env, jobject thiz) {
    auto *connection = new JNIRTMPConnection(env, thiz);
    return reinterpret_cast<jlong>(connection);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_native_1uninit(JNIEnv *env, jobject thiz, jlong handler) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    delete connection;
    connection = nullptr;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_native_1setUrl(JNIEnv *env, jobject thiz, jlong handler,
                                                      jstring url) {

    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    const char *curl = env->GetStringUTFChars(url, nullptr);
    std::string strUrl(curl);
    connection->rtmpConnection.config(strUrl);
    env->ReleaseStringUTFChars(url, curl);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_native_1connect(JNIEnv *env, jobject thiz, jlong handler) {

    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    connection->rtmpConnection.open();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_native_1close(JNIEnv *env, jobject thiz, jlong handler) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    connection->rtmpConnection.close();
}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_hapi_rtmpmuxer_RTMPConnection_native_1sendFrame(JNIEnv *env, jobject thiz, jlong handler,
//                                                         jbyteArray data, jint offset, jint limit,
//                                                         jint media_type,
//                                                         jint sps_len, jint pps_len) {
//
//    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
//    jbyte *c_array = env->GetByteArrayElements(data, 0);
//
//    AVFrame avFrame;
//    avFrame.len = limit - offset;
//    avFrame.data =reinterpret_cast<uint8_t *>(c_array);
//
//    if (media_type == 0) {
//        avFrame.frameType = TYPE_VIDEO;
//    } else {
//        avFrame.frameType = TYPE_AUDIO;
//    }
//
//    avFrame.spsLen = sps_len;
//    avFrame.ppsLen = pps_len;
//    connection->rtmpConnection.onAVFrame(&avFrame);
//    env->ReleaseByteArrayElements(data, c_array, 0);
//}


extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_rtmpmuxer_RTMPConnection_native_1sendFrame(JNIEnv *env, jobject thiz, jlong handler,
                                                         jobject msg, jint offset, jint limit,
                                                         jint media_type,
                                                         jint sps_len, jint pps_len) {

    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    auto *buf    = (jbyte*) env->GetDirectBufferAddress(msg);

    AVFrame avFrame;
    avFrame.len = limit - offset;
    avFrame.data =reinterpret_cast<uint8_t *>(buf);

    if (media_type == 0) {
        avFrame.frameType = TYPE_VIDEO;
    } else {
        avFrame.frameType = TYPE_AUDIO;
    }

    avFrame.spsLen = sps_len;
    avFrame.ppsLen = pps_len;
    connection->rtmpConnection.onAVFrame(&avFrame);
}