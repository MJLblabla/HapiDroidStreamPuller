//
// Created by 1 on 2022/4/7.
//

#include "SRTConnection.h"

SRTConnection::SRTConnection() : IConnection() {
    srt_startup();
    mSRTSocket = srt_create_socket();
}

SRTConnection::~SRTConnection() {
    srt_cleanup();
}

void SRTConnection::config(const SrtConfig &config) {
    this->srtConfig = config;
    LOGCATE("%s %d  SRTConnection::config   %d",
            __FUNCTION__, __LINE__, config.port);
//    this->srtConfig.streamIdLen =   config.streamIdLen ;//= strlen(string_stream_id);
//    this->srtConfig.streamId =    config.streamId;//=string_stream_id;
//    this->srtConfig.ipAddress =   config.ipAddress;//=string_ip_address;
//    this->srtConfig.port =   config.port;
//    this->srtConfig.payload_size =  config.payload_size;
//    this->srtConfig.maxBW =  config.maxBW ;
//    this->srtConfig.inputBW =   config.inputBW ;
}

int SRTConnection::innerConnect() {
    int minversion = SRT_VERSION_FEAT_HSv5;
    if (srtConfig.payload_size <= 0) {
        srtConfig.payload_size = PAYLOAD_SIZE;
    }
    srt_setsockopt(mSRTSocket, 0, SRTO_MINVERSION, &minversion, sizeof minversion);
    srt_setsockopt(mSRTSocket, 0, SRTO_PAYLOADSIZE, &srtConfig.payload_size,
                   sizeof srtConfig.payload_size);
    int transMode = SRTT_LIVE;
    srt_setsockopt(mSRTSocket, 0, SRTO_TRANSTYPE, &transMode, sizeof transMode);

    srt_setsockopt(mSRTSocket, 0, SRTO_STREAMID, srtConfig.streamId, srtConfig.streamIdLen);
    int yes = 1;
    srt_setsockopt(mSRTSocket, 0, SRTO_SENDER, &yes, sizeof yes);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(srtConfig.ipAddress);
    servaddr.sin_port = htons(srtConfig.port);

    int ret = srt_connect(mSRTSocket, (struct sockaddr *) &servaddr, sizeof servaddr);

    if (ret != SRT_ERROR) {
        srt_setsockopt(mSRTSocket, 0, SRTO_MAXBW, &srtConfig.maxBW, sizeof srtConfig.maxBW);
        srt_setsockopt(mSRTSocket, 0, SRTO_INPUTBW, &srtConfig.inputBW, sizeof srtConfig.inputBW);
    }

    LOGCATE("%s %d srt_connect  srt_connect %d  %s %s ",
            __FUNCTION__, __LINE__, ret, srtConfig.streamId, srtConfig.ipAddress);

    return ret != SRT_ERROR;
}

void SRTConnection::innerDisconnect() {
    srt_close(mSRTSocket);
}

void SRTConnection::clear() {
    while (!srtPacketQueue.Empty()) {
        SRTMsgPacket *packet = srtPacketQueue.Pop();
        if (packet) {
            delete packet;
            packet = nullptr;
        }
    }
}

void SRTConnection::pushSRTMsgPacket(SRTMsgPacket *packet) {
    if (isStart && !exit) {
        srtPacketQueue.Push(packet);

//        SRT_MSGCTRL mc = srt_msgctrl_default;
//        mc.msgttl = packet->msgttl;
//        mc.srctime = packet->srctime;
//        mc.boundary = packet->boundary;
//        int retSend = srt_sendmsg2(mSRTSocket, packet->buf, packet->len, &mc);
//        if (retSend == -1) {
//            LOGCATE("%s %d srt_sendmsg2srt_sendmsg2srt %d    %d boundary-> %d   %d",
//                    __FUNCTION__, __LINE__,
//                    retSend, packet->len, packet->boundary, packet->msgttl);
//        }
//        checkConnectStatus(retSend);
//
//        delete packet;
    }
}

void SRTConnection::checkConnectStatus(int sendRet) {
    if (sendRet == -1) {
        if (mConnectedStatus != CONNECTED_STATUS_OFFLINE) {
            SRT_SOCKSTATUS sockstatus = srt_getsockstate(mSRTSocket);
            if (sockstatus > SRTS_BROKEN) {
                mConnectedStatus = CONNECTED_STATUS_OFFLINE;
                connectCallBack(mConnectedStatus);
            }
        } else {
            if (mConnectedStatus != CONNECTED_STATUS_TIMEOUT_PACKET) {
                mConnectedStatus = CONNECTED_STATUS_TIMEOUT_PACKET;
                connectCallBack(mConnectedStatus);
            }
        }
    } else {
        if (mConnectedStatus == CONNECTED_STATUS_TIMEOUT_PACKET) {
            mConnectedStatus = CONNECTED_STATUS_TIMEOUT_RESET;
            connectCallBack(mConnectedStatus);
        }
    }
}

void SRTConnection::biStats(SRT_TRACEBSTATS *status) {
    srt_bistats(mSRTSocket, status, 1, 1);
}


void SRTConnection::loopPack() {
    while (!exit) {
        while (srtPacketQueue.Empty() && !exit) {
            usleep(10 * 1000);
        }
        SRTMsgPacket *packet = srtPacketQueue.Pop();
        if (packet == nullptr) {
            continue;
        }
        SRT_MSGCTRL mc = srt_msgctrl_default;
        mc.msgttl = packet->msgttl;
        mc.srctime = packet->srctime;
        mc.boundary = packet->boundary;
        int retSend = srt_sendmsg2(mSRTSocket, packet->buf, packet->len, &mc);
        LOGCATE("%s %d srt_sendmsg2srt_sendmsg2srt %d    %d boundary-> %d   %d",
                __FUNCTION__, __LINE__,
                retSend, packet->len, packet->boundary, packet->msgttl);
        if (retSend == -1) {
        }
        checkConnectStatus(retSend);
//
//        delete packet;
        delete packet;
    }
}

