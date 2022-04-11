//
// Created by 1 on 2022/3/26.
//

#ifndef ENCODER_MP4OUTPUTSTREAM_H
#define ENCODER_MP4OUTPUTSTREAM_H

#include "OutputStreamContext.h"
#include <media/NdkMediaMuxer.h>
#include "LogUtil.h"

class MP4OutputContext : public OutputStreamContext {

private:
    AMediaMuxer *mMuxer = nullptr;
    FILE *fp_ = nullptr;

    size_t addVideoTrack(AMediaFormat
                         *media_format) {
        videoTrackId = AMediaMuxer_addTrack(mMuxer, media_format);

        if(param.hasAudio && param.hasVideo){
            if (videoTrackId != -1 && audioTrackId != -1) {
                AMediaMuxer_start(mMuxer);
            }
        } else{
            AMediaMuxer_start(mMuxer);
        }
        return videoTrackId;
    }

    size_t addAudioTrack(AMediaFormat
                         *media_format) {
        audioTrackId = AMediaMuxer_addTrack(mMuxer, media_format);

        if(param.hasAudio && param.hasVideo){
            if (videoTrackId != -1 && audioTrackId != -1) {
                AMediaMuxer_start(mMuxer);
            }
        } else{
            AMediaMuxer_start(mMuxer);
        }

        return audioTrackId;
    }

    void writeSampleData(size_t trackIdx,
                         uint8_t *data,
                         AMediaCodecBufferInfo *info
    ) {

        if(param.hasAudio && param.hasVideo){
            if (videoTrackId != -1 && audioTrackId != -1) {
                LOGCATE("AMediaMuxer_writeSampleData  size %d trackIdx %zu pts %ld", info->size,
                        trackIdx,
                        info->presentationTimeUs);
                AMediaMuxer_writeSampleData(mMuxer, trackIdx, data, info);
            }
            return;
        }
        AMediaMuxer_writeSampleData(mMuxer, trackIdx, data, info);

    }

public:
    size_t videoTrackId = -1;
    size_t audioTrackId = -1;

    MP4OutputContext(): OutputStreamContext() {}

    ~MP4OutputContext(){}

    void open(string url,EncodeParam param) override {
        this->param = param;
        mConnectedStatus = CONNECTED_STATUS_START;
        connectCallBack(mConnectedStatus);
        char *m_OutUrl = const_cast<char *>(url.data());
        fp_ = fopen(m_OutUrl, "wb");
        int result = 0;
        if (!(fp_ = fopen(m_OutUrl, "wb"))) {
            result = -1;
            //   return result;
        }
        int mFd = fileno(fp_);
        mMuxer = AMediaMuxer_new(mFd, AMEDIAMUXER_OUTPUT_FORMAT_MPEG_4);
        mConnectedStatus = CONNECTED_STATUS_CONNECTED;
        connectCallBack(mConnectedStatus);
    }

    void close() override {
        mConnectedStatus = CONNECTED_STATUS_CLOSE;
        connectCallBack(mConnectedStatus);
        if(mMuxer){
            AMediaMuxer_stop(mMuxer);
            AMediaMuxer_delete(mMuxer);
        }
        if (fp_ != nullptr) {
            fclose(fp_);
            fp_ = nullptr;
        }
    }

    void onVideoEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                             uint8_t *data) override {
        if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            addVideoTrack(format);
        } else {
            writeSampleData(videoTrackId, data, info);
        }
    }

    void onAudioEncodePacket(ssize_t status, AMediaCodecBufferInfo *info, AMediaFormat *format,
                             uint8_t *data) override {
        if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            addAudioTrack(format);
        } else {
            writeSampleData(audioTrackId, data, info);
        }
    }

};

#endif //ENCODER_MP4OUTPUTSTREAM_H
