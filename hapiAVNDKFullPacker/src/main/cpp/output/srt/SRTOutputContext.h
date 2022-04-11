#ifndef ENCODER_SRTOUTPUTSTREAM_H
#define ENCODER_SRTOUTPUTSTREAM_H

#include "OutputStreamContext.h"
#include "EncodeParam.h"
#include "srt/srt.h"
#include <unistd.h>
#include <media/NdkMediaMuxer.h>
#include <Frame.h>
#include "ThreadSafeQueue.h"
#include "thread"
#include "TSMuxer.h"

class SRTMsgPacket {
public:
    uint8_t *buf;
    int len = 0;
    SRT_MSGCTRL *mctrl;

    SRTMsgPacket() {};

    ~SRTMsgPacket() {
        if (buf != nullptr) {
            free(buf);
        }
        buf = nullptr;
    }
};

class SRTOutputContext : public OutputStreamContext {
private:
    int PAYLOAD_SIZE = 1316;
    int PAT_PACKET_PERIOD = 40;
    SRTSOCKET mSRTSocket;
    volatile bool m_Exit = false;
    thread *pushThread = nullptr;
    ThreadSafeQueue<Frame *>
            mPackQueue;
    TSMuxer tsMuxer;

    [[noreturn]] [[noreturn]] void loopPush();
    static void connect(SRTOutputContext *context);
  //  void tsOutInner(Packet *packet);
    int packCount=0;


public:

    SRTOutputContext();

    ~SRTOutputContext();

    void open(string url, EncodeParam param) override;

    void close() override;

    void onVideoEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                             uint8_t *data) override;

    void onAudioEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                             uint8_t *data) override;

};


#endif

