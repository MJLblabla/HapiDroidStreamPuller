//
// Created by 1 on 2022/3/26.
//

#ifndef ENCODER_RTMPOUTPUTSTREAM_H
#define ENCODER_RTMPOUTPUTSTREAM_H

#include "OutputStreamContext.h"
#include <rtmp.h>
#include "EncodeParam.h"
#include <media/NdkMediaMuxer.h>
#include "ThreadSafeQueue.h"
#include "thread"
using namespace std;

class RTMPOutputContext : public OutputStreamContext {

private:
    /**
  * rtmp 推流实例
  */
    RTMP *mRtmp = 0;
    /**
       * 推流时间
       */
    long mStartTime = 0;

    //音频帧队列
    ThreadSafeQueue<RTMPPacket *>
            rtmpPackQueue;

    volatile bool m_Exit = false;
    thread *pushThread = nullptr;

    void loopPush();
    static void connect(RTMPOutputContext *rtmpOutputStream);

public:
    RTMPOutputContext();

    ~RTMPOutputContext();

    void open(string url,EncodeParam param) override;

    void close() override;

    void onVideoEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                             uint8_t *data) override;

    void onAudioEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                             uint8_t *data) override;
};


#endif //ENCODER_RTMPOUTPUTSTREAM_H
