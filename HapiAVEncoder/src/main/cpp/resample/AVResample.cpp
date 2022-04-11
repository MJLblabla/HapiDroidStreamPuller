//
// Created by 1 on 2022/4/6.
//

#include "AVResample.h"

AVResample::AVResample() {

}

AVResample::~AVResample() {
    if (m_pSwrCtx) {
        swr_free(&m_pSwrCtx);
    }
    m_pSwrCtx = nullptr;
    if (src_yuv_temp == nullptr) {
        free(src_yuv_temp);
    }
    src_yuv_temp = nullptr;
}

int AVResample::start(const EncodeParam& param) {
    encodeParam = param;
    return 1;
}

void AVResample::onFrame2Encode(AudioFrame *inputFrame, AudioFrame *resizeAudioFrame) {

    resizeAudioFrame->out_sample_fmt = encodeParam.out_sample_fmt;
    resizeAudioFrame->audioChannelLayout = encodeParam.audioChannelLayout;
    resizeAudioFrame->audioSampleRate = encodeParam.audioSampleRate;
    resizeAudioFrame->dataSize = inputFrame->dataSize;

    if (inputFrame->audioSampleRate != encodeParam.audioSampleRate
        || inputFrame->audioChannelLayout != encodeParam.audioChannelLayout
        || inputFrame->out_sample_fmt != encodeParam.out_sample_fmt
            ) {
        //只适配byte 8位 每个通道有多少数据
        int input_nb_samples = inputFrame->dataSize / inputFrame->getChannelCount() /
                               (av_get_bytes_per_sample(inputFrame->out_sample_fmt));
//
//        string frameKey = to_string(inputFrame->audioSampleRate) + "" +
//                          to_string(inputFrame->audioChannelLayout) + "" +
//                          to_string(inputFrame->out_sample_fmt);

        if (m_pSwrCtx == nullptr) {
            m_pSwrCtx = swr_alloc();
            //todo 重新初始化
            m_pSwrCtx = swr_alloc_set_opts(m_pSwrCtx,
                                           encodeParam.audioChannelLayout,
                                           encodeParam.out_sample_fmt,
                                           encodeParam.audioSampleRate,

                                           inputFrame->audioChannelLayout,
                                           inputFrame->out_sample_fmt,
                                           inputFrame->audioSampleRate,

                                           0,
                                           0);//输入格式

            swr_init(m_pSwrCtx);
            //  lastInputAudioFrameKey = frameKey;
        }

        int64_t delay = swr_get_delay(m_pSwrCtx, inputFrame->audioSampleRate);
        int64_t out_count = av_rescale_rnd(
                input_nb_samples + delay, //本次要处理的数据个数
                encodeParam.audioSampleRate,
                inputFrame->audioSampleRate,
                AV_ROUND_UP);

        av_samples_alloc(&(resizeAudioFrame->data),
                         NULL,
                         encodeParam.getChannelCount(),
                         out_count,
                         encodeParam.out_sample_fmt,
                         0);

        int out_samples = swr_convert(m_pSwrCtx, &(resizeAudioFrame->data), out_count,
                                      (const uint8_t **) &(inputFrame->data),
                                      input_nb_samples);

        resizeAudioFrame->dataSize =
                out_samples * (av_get_bytes_per_sample(encodeParam.out_sample_fmt)) *
                encodeParam.getChannelCount();

    } else {
        resizeAudioFrame->data = static_cast<uint8_t *>(malloc(inputFrame->dataSize));
        resizeAudioFrame->dataSize = inputFrame->dataSize;
        memcpy(resizeAudioFrame->data, inputFrame->data, inputFrame->dataSize);
    }
}

void AVResample::onFrame2Encode(NativeImage *inputFrame, NativeImage *resizeInputFrame) {

    int widthsrc = inputFrame->width;
    int heightsrc = inputFrame->height;

    auto pBuffer = inputFrame->data;
    int size = inputFrame->dataSize;

    //先转换成i420
    int src_y_size = widthsrc * heightsrc;
    int src_u_size = (widthsrc >> 1) * (heightsrc >> 1);
    if (src_yuv_temp == nullptr) {
        src_yuv_temp = static_cast<uint8_t *>(malloc(widthsrc * heightsrc * 3 / 2));
    }
    switch (inputFrame->format) {
        case IMAGE_FORMAT_I420:
            memcpy(src_yuv_temp, pBuffer, size);
            break;
        case IMAGE_FORMAT_NV12:
            libyuv::NV12ToI420(pBuffer, widthsrc,
                               pBuffer + src_y_size, widthsrc,
                               src_yuv_temp, widthsrc,
                               src_yuv_temp + src_y_size, widthsrc >> 1,
                               src_yuv_temp + src_y_size + src_u_size, widthsrc >> 1,
                               widthsrc, heightsrc);
            break;
        case IMAGE_FORMAT_NV21:
            libyuv::NV21ToI420(pBuffer, widthsrc,
                               pBuffer + src_y_size, widthsrc,
                               src_yuv_temp, widthsrc,
                               src_yuv_temp + src_y_size, widthsrc >> 1,
                               src_yuv_temp + src_y_size + src_u_size, widthsrc >> 1,
                               widthsrc, heightsrc);
            break;
        case IMAGE_FORMAT_RGBA:
            libyuv::ABGRToI420(pBuffer, widthsrc * inputFrame->pixelStride + inputFrame->rowPadding,
                               src_yuv_temp, widthsrc,
                               src_yuv_temp + src_y_size, widthsrc / 2,
                               src_yuv_temp + src_y_size + src_u_size, widthsrc / 2,
                               widthsrc, heightsrc);
            break;
    }

    //缩放图片
    uint8_t *target_yuv = nullptr;

    target_yuv = static_cast<uint8_t *>(malloc(
            encodeParam.frameWidth * encodeParam.frameHeight * 3 / 2));

    uint8_t *temp_yuv = nullptr;

    if (inputFrame->rotationDegrees != 0) {
        libyuv::RotationMode mode = libyuv::kRotate0;
        int roteW = widthsrc;
        int roteH = heightsrc;
        if (inputFrame->rotationDegrees == 90) {
            mode = libyuv::kRotate90;
            roteH = widthsrc;
            roteW = heightsrc;
        } else if (inputFrame->rotationDegrees == 180) {
            mode = libyuv::kRotate180;
        } else if (inputFrame->rotationDegrees == 270) {
            mode = libyuv::kRotate270;
            roteH = widthsrc;
            roteW = heightsrc;
        }
        temp_yuv = static_cast<uint8_t *>(malloc(
                widthsrc * heightsrc * 3 / 2));
        YuvUtil_RoteI420(src_yuv_temp, widthsrc, heightsrc, temp_yuv, mode);
        YuvUtil_ScaleI420(temp_yuv, roteW, roteH, target_yuv, encodeParam.frameWidth,
                          encodeParam.frameHeight, 3);
        free(temp_yuv);
    } else {
        YuvUtil_ScaleI420(src_yuv_temp, widthsrc, heightsrc, target_yuv, encodeParam.frameWidth,
                          encodeParam.frameHeight, 3);
    }

    resizeInputFrame->data = target_yuv;
    resizeInputFrame->dataSize = encodeParam.frameWidth * encodeParam.frameHeight * 3 / 2;
    resizeInputFrame->format = IMAGE_FORMAT_I420;
    resizeInputFrame->width = encodeParam.frameWidth;
    resizeInputFrame->height = encodeParam.frameHeight;
}
