#ifndef AVENCODER_AVRESAMPLE_H
#define AVENCODER_AVRESAMPLE_H
//
// Created by 1 on 2022/4/6.
//
#include "EncodeParam.h"
extern "C"
{
#include <libswresample/swresample.h>
}
#include "yuvutil.h"

class AVResample {

private:
    EncodeParam encodeParam;
    SwrContext *m_pSwrCtx = nullptr;
    uint8_t *src_yuv_temp = nullptr;
public:


    AVResample();

    ~AVResample();

    int start(const EncodeParam& param);

    //添加音频数据到音频队列
    void onFrame2Encode(AudioFrame *inputFrame, AudioFrame *resizeAudioFrame);

    //添加视频数据到视频队列
    void onFrame2Encode(NativeImage *inputFrame,NativeImage *resizeInputFrame);
};

#endif

