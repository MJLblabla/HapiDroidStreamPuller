#include <jni.h>
#include <string>
#include <Stats.h>
#include "JNISRTConnection.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_hapi_srtmuxer_SRTConnection_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1init(JNIEnv *env, jobject thiz) {
    auto *connection = new JNIRTMPConnection(env, thiz);
    return reinterpret_cast<jlong>(connection);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1uninit(JNIEnv *env, jobject thiz, jlong handler) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    delete connection;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1config(JNIEnv *env, jobject thiz, jlong handler,
                                                    jstring stream_id, jstring ip_address,
                                                    jint port, jint payload_size, jint max_bw,
                                                    jint input_bw) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    char *string_stream_id = const_cast<char *>(env->GetStringUTFChars(stream_id, nullptr));
    char *string_ip_address = const_cast<char *>(env->GetStringUTFChars(ip_address, nullptr));

    SrtConfig config;
    config.streamIdLen = strlen(string_stream_id);

    config.streamId = string_stream_id;
    config.ipAddress = string_ip_address;

    config.port = port;
    config.payload_size = payload_size;
    config.maxBW = max_bw;
    config.inputBW = input_bw;

    connection->srtConnection.config(config);
//    env->ReleaseStringUTFChars(stream_id, string_stream_id);
//    env->ReleaseStringUTFChars(ip_address, string_ip_address);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1connect(JNIEnv *env, jobject thiz, jlong handler) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    connection->srtConnection.open();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1send(JNIEnv *env, jobject thiz, jlong handler,
                                                  jobject msg, jint offset, jint limit, jint ttl,
                                                  jlong src_time,
                                                  jint boundary) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    char *buf = (char *) env->GetDirectBufferAddress(msg);
    auto *p = new SRTMsgPacket();

    p->boundary = boundary;
    p->srctime = src_time;
    p->msgttl = ttl;
    //p->buf = buf; //temp;
    int len = limit - offset;
    char *temp = (char *) calloc(len, sizeof(char));
    memcpy(temp, buf + offset, len);
    p->buf = temp;
    p->len = len;
    connection->srtConnection.pushSRTMsgPacket(p);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1close(JNIEnv *env, jobject thiz, jlong handler) {
    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    connection->srtConnection.close();
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_hapi_srtmuxer_SRTConnection_native_1getStats(JNIEnv *env, jobject thiz, jlong handler) {

    auto *connection = reinterpret_cast<JNIRTMPConnection *>(handler);
    SRT_TRACEBSTATS tracebstats;
    connection->srtConnection.biStats(&tracebstats);
    return Stats::getJava(env, tracebstats);
}