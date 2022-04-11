//
// Created by 1 on 2022/3/25.
//

#ifndef ENCODER_OUTPUTSTREAMCONTEXT_H
#define ENCODER_OUTPUTSTREAMCONTEXT_H

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>
#include "EncodeParam.h"


enum ConnectedStatus {
    CONNECTED_STATUS_NULL = 1,
    CONNECTED_STATUS_START = 2,
    CONNECTED_STATUS_CONNECTED = 3,
    CONNECTED_STATUS_CONNECT_FAIL = 4,
    CONNECTED_STATUS_TIMEOUT_PACKET = 5,
    CONNECTED_STATUS_TIMEOUT_RESET = 6,
    CONNECTED_STATUS_OFFLINE = 7,
    CONNECTED_STATUS_CLOSE = 8
};

using namespace std;
typedef std::function<void(int)> ConnectCallBack;

class OutputStreamContext {
private:

protected:
    EncodeParam param;
public:
    int mConnectedStatus = CONNECTED_STATUS_NULL;
    ConnectCallBack connectCallBack;

    OutputStreamContext() {};

    virtual ~OutputStreamContext() {
        connectCallBack = nullptr;
    };

    virtual void open(string url, EncodeParam param) = 0;

    virtual void close() = 0;

    virtual void
    onVideoEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                        uint8_t *data) = 0;

    virtual void
    onAudioEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                        uint8_t *data) = 0;

};

#endif //ENCODER_OUTPUTSTREAMCONTEXT_H
