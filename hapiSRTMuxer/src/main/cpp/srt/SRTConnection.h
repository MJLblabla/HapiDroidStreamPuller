//
// Created by 1 on 2022/4/7.
//

#ifndef HAPISRT_SRTCONNECTION_H
#define HAPISRT_SRTCONNECTION_H

#include "srt/srt.h"
#include <unistd.h>
#include <iostream>
#include "stdio.h"
#include "IConnection.h"

using namespace std;

class SrtConfig {
public:
    char *streamId;
    int streamIdLen;
    char *ipAddress;
    int port;
    int payload_size;
    int maxBW;
    int inputBW;

    SrtConfig() {

    }
//    SrtConfig(char *sid,
//              char *ip,
//              int port,
//              int payload_size,
//              int maxBW,
//              int inputBW) {
//        streamIdLen = strlen(sid);
//        strncpy(this->streamId, sid, streamIdLen + 1);
//        strncpy(this->ipAddress, ip, strlen(ip) + 1);
//        this->port = port;
//        this->payload_size = payload_size;
//        this->maxBW = maxBW;
//        this->inputBW = inputBW;
//    }

    ~SrtConfig() {
//        free(streamId);
//        free(ipAddress);
    }
};

class SRTMsgPacket {
public:
    char *buf;
    int len = 0;
    int boundary = 0;
    long srctime = 0;
    int msgttl = 0;

    SRTMsgPacket() {}

    ~SRTMsgPacket() {
//        if (buf != nullptr) {
//            free(buf);
//        }
//        buf = nullptr;
    }
};

class SRTConnection : public IConnection {

private:
    int PAYLOAD_SIZE = 1316;
    int PAT_PACKET_PERIOD = 40;
    SRTSOCKET mSRTSocket;
    SrtConfig srtConfig;

    ThreadSafeQueue<SRTMsgPacket *> srtPacketQueue;

protected:
    void loopPack() override;

    int innerConnect() override;

    void innerDisconnect() override;

    void clear() override;

public:

    SRTConnection();

    ~SRTConnection();

    void config(const SrtConfig &config);

    void pushSRTMsgPacket(SRTMsgPacket *packet);

    void checkConnectStatus(int sendRet);
    void biStats( SRT_TRACEBSTATS *status );

};


#endif //HAPISRT_SRTCONNECTION_H
