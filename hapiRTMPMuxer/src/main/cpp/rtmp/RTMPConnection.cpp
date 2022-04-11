//
// Created by 1 on 2022/4/9.
//

#include "RTMPConnection.h"

RTMPConnection::RTMPConnection() : IConnection() {
    this->mRtmp = RTMP_Alloc();
}

RTMPConnection::~RTMPConnection() {
    if (!mRtmp)
        return;
    RTMP_Free(mRtmp);
    mRtmp = nullptr;
}

int RTMPConnection::innerConnect() {
    RTMP_Init(mRtmp);
    char *m_OutUrl = const_cast<char *>(url.data());
    int ret = RTMP_SetupURL(mRtmp, m_OutUrl);
    if (!ret) {
        return ret;
    }
    mRtmp->Link.timeout = 3;
    RTMP_EnableWrite(mRtmp);
    ret = RTMP_Connect(mRtmp, 0);
    if (!ret) {
        return ret;
    }
    //seek 到某一处
    ret = RTMP_ConnectStream(mRtmp, 0);
    return ret;
}


void RTMPConnection::innerDisconnect() {
    RTMP_Close(mRtmp);
}

void RTMPConnection::loopPack() {
    while (isStart && !exit) {
        while (rtmpPackQueue.Empty() && !exit) {
            usleep(10 * 1000);
        }
        RTMPPacket *packet = rtmpPackQueue.Pop();
        if (packet == nullptr) {
            continue;
        }
        if (mRtmp == nullptr) {
            return;
        }
        int result = RTMP_SendPacket(mRtmp, packet, 1);

        checkConnectStatus(result);
        RTMPPacket_Free(packet);
        packet = nullptr;
    }
}

void RTMPConnection::clear() {
    while (!rtmpPackQueue.Empty()) {
        RTMPPacket *packet = rtmpPackQueue.Pop();
        if (packet != nullptr) {
            RTMPPacket_Free(packet);
            packet = nullptr;
        }
    }
}

void RTMPConnection::checkConnectStatus(int setResult) {

    if (!setResult) {
        LOGCATE("RTMPConnection::loopPack( %d", setResult);
        if (!RTMP_IsConnected(mRtmp)) {
            mConnectedStatus = CONNECTED_STATUS_OFFLINE;
            connectCallBack(mConnectedStatus);
            return;
        }

        if (RTMP_IsTimedout(mRtmp)) {
            if (mConnectedStatus != CONNECTED_STATUS_TIMEOUT_PACKET) {
                mConnectedStatus = CONNECTED_STATUS_TIMEOUT_PACKET;
                connectCallBack(mConnectedStatus);
            }
        }
    } else {
        if (CONNECTED_STATUS_TIMEOUT_PACKET == mConnectedStatus) {
            mConnectedStatus = CONNECTED_STATUS_TIMEOUT_RESET;
            connectCallBack(mConnectedStatus);
        }
    }
}

void RTMPConnection::config(string pushUrl) {
    this->url = std::move(pushUrl);
}

void RTMPConnection::onAVFrame(AVFrame *frame) {

    if (frame->frameType == TYPE_VIDEO) {
        uint8_t t = *(frame->data + 4);
        uint8_t nalType = (t & 0x1f);

        if (nalType == 7 || nalType == 8) {
            auto pushSpsPps = [this](uint8_t *sps, int sps_len, uint8_t *pps, int pps_len) {

                int bodysize = sps_len + pps_len + 16;
                auto *packet = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
                RTMPPacket_Alloc(packet, bodysize);
                RTMPPacket_Reset(packet);

                char *body = packet->m_body;

                int i = 0;

                body[i++] = 0x17; //帧类型, 1 字节

                body[i++] = 0x00; //数据类型, 1 字节

                //00 00 00 合成时间, 3 字节
                body[i++] = 0x00;
                body[i++] = 0x00;
                body[i++] = 0x00;

                // 版本信息, 1 字节
                body[i++] = 0x01;

                //64 00 32 编码规则, 3 字节
                body[i++] = sps[1];
                body[i++] = sps[2];
                body[i++] = sps[3];

                body[i++] = 0xFF;//NALU 长度, 1 字节

                body[i++] = 0xE1; //sps 个数

                //00 19 SPS 长度, 2 字节
                body[i++] = (sps_len >> 8) & 0xff; //取高八位写入地址
                body[i++] = sps_len & 0xff; //取低八位写入高地址中

                //sps 数据部分
                memcpy(&body[i], sps, sps_len);

                i += sps_len;

                body[i++] = 0x01;
                body[i++] = (pps_len >> 8) & 0xff;
                body[i++] = pps_len & 0xff;
                memcpy(&body[i], pps, pps_len);

                packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
                packet->m_nBodySize = bodysize;
                packet->m_nTimeStamp = 0;
                packet->m_hasAbsTimestamp = 0;
                packet->m_nChannel = 0x04;
                packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
                packet->m_nInfoField2 = mRtmp->m_stream_id;

                rtmpPackQueue.Push(packet);
            };

            pushSpsPps(frame->data + 4, frame->spsLen - 4, frame->data + frame->spsLen + 4,
                       frame->ppsLen - 4);
        } else {
            auto pushVideoData = [this](uint8_t *video, int len, int type) {
                int data_len = len;
                int bodysize = data_len + 9;
                auto *packet = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
                RTMPPacket_Alloc(packet, bodysize);
                RTMPPacket_Reset(packet);

                char *body = packet->m_body;
                int i = 0;

                // 帧类型数据 : 分为两部分;
                // 前 4 位表示帧类型, 1 表示关键帧, 2 表示普通帧
                // 后 4 位表示编码类型, 7 表示 AVC 视频编码

                if (type == 5) {
                    body[i++] = 0x17;
                } else {
                    body[i++] = 0x27;
                }//1

                // 数据类型, 00 表示 AVC 序列头
                body[i++] = 0x01; //2

                // 合成时间, 一般设置 00 00 00
                body[i++] = 0x00;
                body[i++] = 0x00;
                body[i++] = 0x00;  //5

                // 长度
                body[i++] = (data_len >> 24) & 0xff;
                body[i++] = (data_len >> 16) & 0xff;
                body[i++] = (data_len >> 8) & 0xff;
                body[i++] = data_len & 0xff;     //9位


                memcpy(&body[i], video, data_len); //数据部分

                packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
                packet->m_nBodySize = bodysize;
                packet->m_nTimeStamp = RTMP_GetTime() - mStartTime;
                packet->m_hasAbsTimestamp = 0;
                packet->m_nChannel = 0x04;
                packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
                packet->m_nInfoField2 = mRtmp->m_stream_id;
                rtmpPackQueue.Push(packet);
            };
            pushVideoData(frame->data, frame->len, nalType);
        }

    } else {

        auto pushAudioData = [this](uint8_t *audio, int len) {
            int data_len = len;
            int bodysize = data_len + 2;
            auto *packet = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
            RTMPPacket_Alloc(packet, bodysize);
            RTMPPacket_Reset(packet);
            char *body = packet->m_body;
            body[0] = 0xAF;
            body[1] = 0x01;

            memcpy(&body[2], audio, data_len);

            packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
            packet->m_nBodySize = bodysize;
            packet->m_nTimeStamp = RTMP_GetTime() - mStartTime;
            packet->m_hasAbsTimestamp = 0;
            packet->m_nChannel = 0x05;
            packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
            packet->m_nInfoField2 = mRtmp->m_stream_id;

            rtmpPackQueue.Push(packet);
        };

        pushAudioData(frame->data, frame->len);
    }

}