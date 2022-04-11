#include "SRTOutputContext.h"
#include "ADTSUitl.h"

SRTOutputContext::SRTOutputContext() : OutputStreamContext() {
    srt_startup();

    mSRTSocket = srt_create_socket();
    if (mSRTSocket == SRT_ERROR) {
        LOGCATE("SRTOutputContext srt_create_socket fail %d ", mSRTSocket);
    }
}

SRTOutputContext::~SRTOutputContext() {
    srt_cleanup();
}

void SRTOutputContext::open(string url, EncodeParam param) {
    this->param = param;
    mConnectedStatus = CONNECTED_STATUS_START;
    connectCallBack(mConnectedStatus);
    if (pushThread != nullptr) {
        delete pushThread;
        pushThread = nullptr;
    }

//    ServiceInfo tsServiceInfo(
//            DIGITAL_TV,
//            0x4698,
//            "blabla",
//            "hapi"
//    );
//    tsMuxer.listener = [this](Packet *packet) {
//        tsOutInner(packet);
//    };
//    serviceId = tsMuxer.addService(tsServiceInfo);
    pushThread = new thread(connect, this);
}

void SRTOutputContext::connect(SRTOutputContext *context) {
    context->loopPush();
}

[[noreturn]] void SRTOutputContext::loopPush() {
    //Maximum payload size sent in one UDP packet (0 if unlimited)
    int minversion = SRT_VERSION_FEAT_HSv5;
    srt_setsockopt(mSRTSocket, 0, SRTO_MINVERSION, &minversion, sizeof minversion);
    srt_setsockopt(mSRTSocket, 0, SRTO_PAYLOADSIZE, &PAYLOAD_SIZE, sizeof PAYLOAD_SIZE);
    int transMode = SRTT_LIVE;
    srt_setsockopt(mSRTSocket, 0, SRTO_TRANSTYPE, &transMode, sizeof transMode);

    string streamid = "#!::h=sdk-live/manjiale,m=publish,domain=pili-publish.qnsdk.com";
    int sizeSid = streamid.length();
    char *sid = "#!::h=sdk-live/manjiale,m=publish,domain=pili-publish.qnsdk.com";
    srt_setsockopt(mSRTSocket, 0, SRTO_STREAMID, sid, sizeSid);

//
//    struct sockaddr_in sa{};
//    //todo
//    //srt://pili-publish.qnsdk.com:1935?streamid=#!::h=sdk-live/manjiale,m=publish,domain=pili-publish.qnsdk.com
//    sa.sin_port = htons(atoi("1935"));
//    inet_pton(AF_INET, "59.83.211.145", &sa.sin_addr);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("59.83.211.145");;
    servaddr.sin_port = htons(1935);

    int yes = 1;
    srt_setsockopt(mSRTSocket, 0, SRTO_SENDER, &yes, sizeof yes);

    int ret = srt_connect(mSRTSocket, (struct sockaddr *) &servaddr, sizeof servaddr);

    LOGCATE("%s %d srt_connect  srt_connect %d",
            __FUNCTION__, __LINE__, ret);

    if (ret == SRT_ERROR) {
        mConnectedStatus = CONNECTED_STATUS_CONNECT_FAIL;
        connectCallBack(mConnectedStatus);
        return;
    } else {
        m_Exit = false;
        mConnectedStatus = CONNECTED_STATUS_CONNECTED;
        connectCallBack(mConnectedStatus);

        int maxBW = 0;
        int inputBW = param.audioBitrate + param.videoBitRate;

        srt_setsockopt(mSRTSocket, 0, SRTO_MAXBW, &maxBW, sizeof maxBW);
        srt_setsockopt(mSRTSocket, 0, SRTO_INPUTBW, &inputBW, sizeof inputBW);

        //SRTSOCKET u, const char* buf, int len, SRT_MSGCTRL *mctrl
        // srt_sendmsg2();
        while (!m_Exit) {
            while (mPackQueue.Empty() && !m_Exit) {
                usleep(10 * 1000);
            }
            Frame *packet = mPackQueue.Pop();
            if (packet == nullptr) {
                continue;
            }
            packCount++;
            if (packCount == 40 || packet->isKeyFrame) {
                SimpleBuffer pat;
                tsMuxer.create_pmt(&pat);

                int ret1 = srt_send(mSRTSocket, pat.data(), pat.size());
                SimpleBuffer pmt;
                tsMuxer.create_pmt(&pmt);
                int ret2 = srt_send(mSRTSocket, pmt.data(), pmt.size());
                packCount = 0;
            }


            packet->pts = packet->pts;
            packet->dts = packet->dts;



            if (packet->trackId == tsMuxer.videoStreamId) {

                int PAYLOAD_Limit=500;
                if (packet->len > PAYLOAD_Limit) {
                    SRT_MSGCTRL mc = srt_msgctrl_default;
                    int count = packet->len / PAYLOAD_Limit;
                    int mod = packet->len % PAYLOAD_Limit;
                    if (mod > 0) {
                        count++;
                    }
                    int position = 0;
                    int limit = 0;
                    for (int i = 0; i < count; i++) {

                        position = i * PAYLOAD_Limit;
                        if (i == count - 1) {
                            limit = i * PAYLOAD_Limit+ mod -1;
                        } else {
                            limit = (i + 1) * PAYLOAD_Limit -1;
                        }

                        if (i == 0) {
                            mc.boundary = 3;
                        } else if (i == count - 1) {
                            mc.boundary = 1;
                        } else {
                            mc.boundary = 0;
                        }

                        mc.srctime = GetSysCurrentTimeNS() / 1000;//packet->pts;

                        int len = limit - position  +1;

                        uint8_t *temp = static_cast<uint8_t *>(malloc(len));

                        memcpy(temp,packet->buffer+position,len);

                        SimpleBuffer sb;
                        tsMuxer.pes_video(&sb, temp, len,
                                          packet->pts, packet->dts,mc.boundary == 1);

                        int len2 = sb.size();
                        int retSend = srt_sendmsg2(mSRTSocket, sb.data(), len2,
                                                   &mc);
                        free(temp);
                        LOGCATE("%s %d cxccccsrt_sendmsg2srt_sendmsg2srt_sendmsg2srt_sendmsg2  srt_connect %d %d %d  %d %d",
                                __FUNCTION__, __LINE__, retSend, mc.boundary, len2 , position,limit);

                    }
                } else {
                    SimpleBuffer sb;
                    SRT_MSGCTRL mc = srt_msgctrl_default;
                    mc.boundary = 2;
                    mc.srctime = GetSysCurrentTimeNS() / 1000;//packet->pts;
                    tsMuxer.pes_video(&sb, packet->buffer, packet->len, packet->pts, packet->dts,
                                      true);

                    int len = sb.size();
                    int retSend = srt_sendmsg2(mSRTSocket, sb.data(), len,
                                               &mc);
                    LOGCATE("%s %d nnnnnnsrt_sendmsg2srt_sendmsg2srt_sendmsg2srt_sendmsg2  srt_connect %d %d %d",
                            __FUNCTION__, __LINE__, retSend, mc.boundary, len);
                }

            } else {
                SimpleBuffer sb;
                SRT_MSGCTRL mc = srt_msgctrl_default;
                mc.boundary = 2;
                mc.srctime = GetSysCurrentTimeNS() / 1000;//packet->pts;
                tsMuxer.pes_audio(&sb, packet->buffer, packet->len, packet->pts, packet->dts);
                int len = sb.size();
                int retSend = srt_sendmsg2(mSRTSocket, sb.data(), len,
                                           &mc);
                LOGCATE("%s %d srt_sendmsg2srt_sendmsg2srt_sendmsg2srt_sendmsg2  srt_connect %d %d %d",
                        __FUNCTION__, __LINE__, retSend, mc.boundary, len);
            }

            // tsMuxer.encode(packet, packet->trackId);
            delete packet;
            packet = nullptr;
        }
    }
}

void SRTOutputContext::close() {

    srt_close(mSRTSocket);
    mConnectedStatus = CONNECTED_STATUS_CLOSE;
    connectCallBack(mConnectedStatus);
    m_Exit = true;
    try {
        if (pushThread) {
            pushThread->join();
            delete pushThread;
            pushThread = nullptr;
        }
    } catch (const exception &e) {
    }

//    while (!mPackQueue.Empty()) {
//        SRTMsgPacket *packet = mPackQueue.Pop();
//        if (packet) {
//            delete packet;
//            packet = nullptr;
//        }
//    }
}

void SRTOutputContext::onVideoEncodePacket(ssize_t status, AMediaCodecBufferInfo *info,
                                           AMediaFormat *format, uint8_t *data) {


//    if (videoTrackId <= 0) {
//        videoTrackId = tsMuxer.addStream(serviceId, MIMETYPE_VIDEO_AVC);
//    }
    if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        return;
    }

    int type = data[4] & 0x1f;
    LOGCATE("%s %d onVideoEncodePacket flag %d   type%d",
            __FUNCTION__, __LINE__, info->flags, type);

    bool isKey = info->flags == 1;
    if (info->size <= 0) {
        return;
    }
    if (info->flags == 1) {
        uint8_t *sps = nullptr;
        size_t sps_len = 0;
        uint8_t *pps = nullptr;
        size_t pps_len = 0;
        bool sps_ok = AMediaFormat_getBuffer(format, "csd-0", (void **) &sps, &sps_len);
        bool pps_ok = AMediaFormat_getBuffer(format, "csd-1", (void **) &pps, &pps_len);
        int headSize = 7 + sps_len + pps_len;
        int newSize = headSize + info->size;
        uint8_t *frameDate = static_cast<uint8_t *>(malloc(newSize));
        frameDate[0] = 0x00;
        frameDate[1] = 0x00;
        frameDate[2] = 0x00;
        frameDate[3] = 0x01;
        frameDate[4] = (0x46);
        frameDate[5] = (0x01);
        frameDate[6] = (0x50);
        memcpy(frameDate + 7, sps, sps_len);
        memcpy(frameDate + 7 + sps_len, pps, pps_len);
        memcpy(frameDate + headSize, data, info->size);

        auto f = new Frame(frameDate, newSize, info->presentationTimeUs, info->presentationTimeUs,
                           true, tsMuxer.videoStreamId);
        // tsMuxer.encode(&frame, videoTrackId);
        mPackQueue.Push(f);

    } else {
        auto *frameDate = static_cast<uint8_t *>(malloc(info->size));
        memcpy(frameDate, data, info->size);

        auto f = new Frame(frameDate, info->size, info->presentationTimeUs,
                           info->presentationTimeUs,
                           false, tsMuxer.videoStreamId);
        mPackQueue.Push(f);
    }

}


void SRTOutputContext::onAudioEncodePacket(ssize_t status, AMediaCodecBufferInfo *info,
                                           AMediaFormat *format, uint8_t *data) {

//    if (audioTrackId <= 0) {
//        audioTrackId = tsMuxer.addStream(serviceId, MIMETYPE_AUDIO_AAC);
//    }
    if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        return;
    }
    int type = data[4] & 0x1f;
    LOGCATE("%s %d onAudioEncodePacket flag %d   type%d",
            __FUNCTION__, __LINE__, info->flags, type);

    bool isKey = info->flags == 1;
    if (info->size <= 2) {
        return;
    }
    auto *adts = static_cast<uint8_t *>(malloc(7));
    addADTS(adts, info->size, param.audioSampleRate, param.getChannelCount());

    int newSize = 7 + info->size;
    auto *newDate = static_cast<uint8_t *>(malloc(newSize));
    memcpy(newDate, adts, 7);
    memcpy(newDate + 7, data, info->size);

    auto f = new Frame(newDate, newSize, info->presentationTimeUs, info->presentationTimeUs, false,
                       tsMuxer.audioStreamId);
    mPackQueue.Push(f);

//    if (isKey) {
//
//        //  tsMuxer.encode(&frame, audioTrackId);
//
//    } else {
//        auto *frameDate = static_cast<uint8_t *>(malloc(info->size));
//        memcpy(frameDate, data, info->size);
//        auto f = new Frame(frameDate, info->size, info->presentationTimeUs, -1,
//                           false, tsMuxer.audioStreamId);
//        mPackQueue.Push(f);
//    }
}
//
//void SRTOutputContext::tsOutInner(Packet *packet) {
//
//    uint8_t *buf = packet->buffer;
//    int len = packet->bufferLen;
//    bool isFirstPacketFrame = packet->isFirstPacketFrame;
//    bool isLastPacketFrame = packet->isLastPacketFrame;
//    long srcTime = packet->ts; // in µs
//    int index = packet->index;
//
//    SRT_MSGCTRL mc = srt_msgctrl_default;
//
//    if (isFirstPacketFrame
//        && isLastPacketFrame
//            ) {
//        mc.boundary = 2;
//    } else if (isFirstPacketFrame) {
//        mc.boundary = 3;
//    } else if (isLastPacketFrame) {
//        mc.boundary = 2;
//    } else {
//        mc.boundary = 0;
//    }
//    if (srcTime != 0) {
//       mc.srctime = GetSysCurrentTimeNS()/1000;
//    }
//
//    int ret = srt_sendmsg2(mSRTSocket, reinterpret_cast<const char *>(buf), len,
//                           &mc);
//////
////        const char message [] = "This message should be sent to the other side";
////        int ret  = srt_sendmsg2(mSRTSocket, message, sizeof message,  &mc);
//    LOGCATE("%s %d srt_sendmsg2srt_sendmsg2srt_sendmsg2srt_sendmsg2  srt_connect %d %d %d",
//            __FUNCTION__, __LINE__, ret, mc.boundary, len);
//
////
////    mPackQueue.Push(srtPacket);
////    LOGCATE(" mPackQueue.Push(srtPacket)   index %d   boundary %d   size %d->  ",
////            index, srtPacket->mctrl->boundary,mPackQueue.Size());
//}