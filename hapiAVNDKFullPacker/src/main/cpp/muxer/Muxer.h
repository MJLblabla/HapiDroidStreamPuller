//
// Created by 1 on 2022/3/25.
//

#ifndef ENCODER_MUXER_H
#define ENCODER_MUXER_H

extern "C"
{
#include <libswresample/swresample.h>
}

#include "HWAudioEncoder.h"
#include "HWVideoEncoder.h"
#include "OutputStreamContext.h"
#include "SRTOutputContext.h"
#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "onEventCallback"
#define JAVA_PLAYER_MUXER_CALLBACK_API_NAME "onMuxerStatusCallback"

class Muxer {
private:
    int maxVideoFrameCachedSize = 8;
    int maxAudiFrameCachedSize = 8;
    //编码码器状态
    volatile int mMuxerState = STATE_UNKNOWN;
    string m_OutUrl;
    OutputStreamContext *outputStreamContext = nullptr;
    HWVideoEncoder videoEncoder;
    HWAudioEncoder audioEncoder;
    EncodeParam encodeParam;
    uint8_t *src_yuv_temp = nullptr;
    SwrContext *m_pSwrCtx = nullptr;


    JavaVM *m_JavaVM = nullptr;
    jobject m_JavaObj = nullptr;

    JNIEnv *GetJNIEnv(bool *isAttach);

    jobject GetJavaObj();

    JavaVM *GetJavaVM();

public:
    Muxer();

    ~Muxer();

    int getConnectStatus();

    int getMuxerStatus() const;

    int start(string url, EncodeParam param);

    void pause();

    void resume();

    void stop();

    //添加音频数据到音频队列
    int onFrame2Encode(AudioFrame *inputFrame);

    //添加视频数据到视频队列
    int onFrame2Encode(NativeImage *inputFrame);

    void setJNIEnv(JNIEnv *jniEnv, jobject obj);

    void releaseJNIEnv();
};

#endif //ENCODER_MUXER_H
