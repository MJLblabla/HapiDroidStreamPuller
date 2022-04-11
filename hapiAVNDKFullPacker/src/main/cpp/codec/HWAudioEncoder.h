//
// Created by 1 on 2022/3/25.
//

#ifndef ENCODER_HWAUDIOENCODER_H
#define ENCODER_HWAUDIOENCODER_H

#include "HWEncoder.h"
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>
#include "EncodeParam.h"

class HWAudioEncoder : public HWEncoder<AudioFrame> {
protected:
    void configAMediaCodec() override {
        m_SamplesCount = 0;
        int sampleRate = mEncodeParam.audioSampleRate;
        int channelCount = mEncodeParam.getChannelCount();
        int bitrate = mEncodeParam.audioBitrate;
        baseTime = sampleRate *  (av_get_bytes_per_sample(mEncodeParam.out_sample_fmt)) * channelCount ;
        int inited_ = -1;

        const char *mine = "audio/mp4a-latm";
        media_codec_ = AMediaCodec_createEncoderByType(mine);
        media_format_ = AMediaFormat_new();

        AMediaFormat_setString(media_format_, "mime", mine);
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_BIT_RATE, bitrate);
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_AAC_PROFILE, 2);
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);//s
        AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);

        media_status_t status = AMEDIA_OK;

        if ((status = AMediaCodec_configure(media_codec_, media_format_, NULL, NULL,
                                            AMEDIACODEC_CONFIGURE_FLAG_ENCODE))) {
            LOGCATE("%s %d audio AMediaCodec_configure fail (%d)", __FUNCTION__, __LINE__, status);
            throw std::runtime_error("AMediaCodec_configure fail ");
        }
        LOGCATE("%s %d audio AMediaCodec_configure status: %d %s", __FUNCTION__, __LINE__, status,
                AMediaFormat_toString(media_format_));

        if ((status = AMediaCodec_start(media_codec_))) {
            LOGCATE("%s %d  audio AMediaCodec_start fail (%d)", __FUNCTION__, __LINE__, status);
            throw std::runtime_error("AMediaCodec_start  fail ");
        }
        LOGCATE("%s %d audio HWAudioEncoder  start (%d)", __FUNCTION__, __LINE__, status);
        inited_ = 1;
        m_Exit = false;
    }

    int m_SamplesCount = 0;

    int encodeFrame(AudioFrame *frame) override {
        uint8_t *data = frame->data;
        int size = frame->dataSize;
        int pts = sendFrame(data, size, (int) (m_SamplesCount * 1000000.0 / baseTime));
        m_SamplesCount = m_SamplesCount + size;
        return pts;
    }

public:
    HWAudioEncoder() = default;

    ~HWAudioEncoder() = default;
};

#endif //ENCODER_HWAUDIOENCODER_H
