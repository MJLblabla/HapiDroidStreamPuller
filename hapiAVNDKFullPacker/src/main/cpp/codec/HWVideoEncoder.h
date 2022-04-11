//
// Created by 1 on 2022/3/25.
//
#ifndef ENCODER_HWVIDEOENCODER_H
#define ENCODER_HWVIDEOENCODER_H

#include "HWEncoder.h"

class HWVideoEncoder : public HWEncoder<NativeImage> {
protected:
    void configAMediaCodec() override {
        int width = mEncodeParam.frameWidth;
        int height = mEncodeParam.frameHeight;
        int fps = mEncodeParam.fps;
        int bitrate = mEncodeParam.videoBitRate;
        baseTime = 1000000L / mEncodeParam.fps;
        int inited_ = -1;

        const char *mine = "video/avc";
        media_codec_ = AMediaCodec_createEncoderByType(mine);
        media_format_ = AMediaFormat_new();
        AMediaFormat_setString(media_format_, "mime", mine);

        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_WIDTH, width); // 视频宽度
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_HEIGHT, height); // 视频高度

        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_BIT_RATE, bitrate);
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_FRAME_RATE, fps);
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);//s
        //
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_COLOR_FORMAT, 19);
        // AMediaFormat_setInt32(media_format_, "bitrate-mode", MEDIACODEC_BITRATE_MODE_VBR);
        media_status_t status = AMEDIA_OK;

        if ((status = AMediaCodec_configure(media_codec_, media_format_, NULL, NULL,
                                            AMEDIACODEC_CONFIGURE_FLAG_ENCODE))) {
            LOGCATE("%s %d AMediaCodec_configure fail (%d)", __FUNCTION__, __LINE__, status);
            throw std::runtime_error("AMediaCodec_configure fail ");
        }
        LOGCATE("%s %d AMediaCodec_configure status: %d %s", __FUNCTION__, __LINE__, status,
                AMediaFormat_toString(media_format_));

        if ((status = AMediaCodec_start(media_codec_))) {
            LOGCATE("%s %d AMediaCodec_start fail (%d)", __FUNCTION__, __LINE__, status);
            throw std::runtime_error("AMediaCodec_start  fail ");
        }
//        mTrackIndex = AMediaMuxer_addTrack(mMuxer, media_format_);
        inited_ = 1;
        startTime = GetSysCurrentTimeNS();
        LOGCATE("%s %d inited: %s", __FUNCTION__, __LINE__, inited_ ? "success" : "false");
    }

    int encodeFrame(NativeImage *frame) override {
        uint8_t *data = frame->data;
        int size = frame->dataSize;
        long current = GetSysCurrentTimeNS();
        int pts = sendFrame(data, size, (int) ((current - startTime) / 1000));
        frameIndex++;
        return pts;
    }

public:
    HWVideoEncoder() {};

    ~HWVideoEncoder() {};
};

#endif //ENCODER_HWVIDEOENCODER_H
