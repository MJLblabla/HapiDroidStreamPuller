//
// Created by 1 on 2022/3/25.
//
#include "Muxer.h"
#include "MP4OutputContext.h"
#include "RTMPOutputContext.h"
#include "SRTOutputContext.h"
#include "yuvutil.h"


Muxer::Muxer() {

}

Muxer::~Muxer() {
    if (outputStreamContext != nullptr) {
        delete outputStreamContext;
        outputStreamContext = nullptr;
    }
    if (src_yuv_temp != nullptr) {
        free(src_yuv_temp);
        src_yuv_temp = nullptr;
    }
    if (m_pSwrCtx) {
        swr_free(&m_pSwrCtx);
    }
    m_pSwrCtx = nullptr;
}

int Muxer::getConnectStatus() {
    if (outputStreamContext != nullptr) {
        return outputStreamContext->mConnectedStatus;
    } else {
        return CONNECTED_STATUS_NULL;
    }
}

int Muxer::getMuxerStatus() const {
    return mMuxerState;
}

int Muxer::start(string url, EncodeParam param) {
    m_OutUrl = std::move(url);
    encodeParam = param;
    int isStarted = 1;
    if (outputStreamContext != nullptr) {
        delete outputStreamContext;
        outputStreamContext = nullptr;
    }

    auto startsWith = [](const string &s, const string &sub) -> int {
        return s.find(sub) == 0 ? 1 : 0;
    };
    auto endsWith = [](const string &s, const string &sub) -> int {
        return s.rfind(sub) == (s.length() - sub.length()) ? 1 : 0;
    };
    if (endsWith(m_OutUrl, "mp4")) {
        outputStreamContext = new MP4OutputContext();
    } else if (startsWith(m_OutUrl, "rtmp")) {
        outputStreamContext = new RTMPOutputContext();
    } else {
        outputStreamContext = new MP4OutputContext();
    }

    videoEncoder.
            dequeueOutputBufferCall = [this](ssize_t status, AMediaCodecBufferInfo *info,
                                             AMediaFormat *format,
                                             uint8_t *data) {
        outputStreamContext->onVideoEncodePacket(status, info, format, data);
    };
    audioEncoder.
            dequeueOutputBufferCall = [this](ssize_t status, AMediaCodecBufferInfo *info,
                                             AMediaFormat *format,
                                             uint8_t *data) {
        outputStreamContext->onAudioEncodePacket(status, info, format, data);
    };

    outputStreamContext->
            connectCallBack = [this](int status) {

        switch (status) {
            case CONNECTED_STATUS_START:
                break;
            case CONNECTED_STATUS_CONNECTED:
                videoEncoder.start();
                audioEncoder.start();

                break;
            case CONNECTED_STATUS_CONNECT_FAIL:
            case CONNECTED_STATUS_OFFLINE:
                stop();
                break;
            default:
                break;
        }

        bool isAttach = false;
        JNIEnv *env = GetJNIEnv(&isAttach);
        if (env == nullptr)
            return;
        jobject javaObj = GetJavaObj();
        jmethodID mid = env->GetMethodID(env->GetObjectClass(javaObj),
                                         JAVA_PLAYER_EVENT_CALLBACK_API_NAME, "(I)V");

        if (mid) {
            env->CallVoidMethod(javaObj, mid, status);
        }

        if (isAttach)
            GetJavaVM()->DetachCurrentThread();
    };

    try {
        if (param.hasVideo) {
            videoEncoder.
                    configMediaCodec(param);
        }
        if (param.hasAudio) {
            audioEncoder.
                    configMediaCodec(param);
        }
        outputStreamContext->
                open(m_OutUrl, param
        );
        mMuxerState = STATE_DECODING;
    } catch (
            const exception &e
    ) { // const &捕获异常，可以用基类
        isStarted = 0;

        stop();

    }
    return
            isStarted;
}

void Muxer::stop() {
    if (encodeParam.hasVideo) {
        videoEncoder.stop();
    }
    if (encodeParam.hasAudio) {
        audioEncoder.stop();
    }
    outputStreamContext->close();
    mMuxerState = STATE_STOP;
}

void Muxer::pause() {
    if (encodeParam.hasVideo) {
        videoEncoder.pause();
    }
    if (encodeParam.hasAudio) {
        audioEncoder.pause();
    }
    mMuxerState = STATE_PAUSE;
}

void Muxer::resume() {
    if (encodeParam.hasVideo) {
        videoEncoder.resume();
    }
    if (encodeParam.hasAudio) {
        audioEncoder.resume();
    }
    mMuxerState = STATE_DECODING;
}

int Muxer::onFrame2Encode(AudioFrame *inputFrame) {

    if (!encodeParam.hasAudio) {
        return 0;
    }
    if (mMuxerState != STATE_DECODING) {
        return 0;
    }
    if (audioEncoder.getFrameCachedSize() > maxAudiFrameCachedSize) {
        return 0;
    }
    auto resizeAudioFrame = new AudioFrame();

    resizeAudioFrame->out_sample_fmt = encodeParam.out_sample_fmt;
    resizeAudioFrame->audioChannelLayout = encodeParam.audioChannelLayout;
    resizeAudioFrame->audioSampleRate = encodeParam.audioSampleRate;
    resizeAudioFrame->dataSize = inputFrame->dataSize;
    // resizeAudioFrame->data = static_cast<uint8_t *>(malloc(inputFrame->dataSize));

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
    audioEncoder.pushFrame(resizeAudioFrame);
    return 1;
}

int Muxer::onFrame2Encode(NativeImage *inputFrame) {

    if (!encodeParam.hasVideo) {
        return 0;
    }
    if (mMuxerState != STATE_DECODING) {
        return 0;
    }
    if (videoEncoder.getFrameCachedSize() > maxVideoFrameCachedSize) {
        return 0;
    }
    auto resizeInputFrame = new NativeImage();
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

    videoEncoder.pushFrame(resizeInputFrame);
    return 1;
}

void Muxer::setJNIEnv(JNIEnv *jniEnv, jobject obj) {
    jniEnv->GetJavaVM(&m_JavaVM);
    m_JavaObj = jniEnv->NewGlobalRef(obj);
}

void Muxer::releaseJNIEnv() {
    bool isAttach = false;
    JNIEnv *env = GetJNIEnv(&isAttach);
    env->DeleteGlobalRef(m_JavaObj);
    if (isAttach)
        GetJavaVM()->DetachCurrentThread();
}

JNIEnv *Muxer::GetJNIEnv(bool *isAttach) {
    JNIEnv *env;
    int status;
    if (nullptr == m_JavaVM) {
        LOGCATE("HWCodecPlayer::GetJNIEnv m_JavaVM == nullptr");
        return nullptr;
    }
    *isAttach = false;
    status = m_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status != JNI_OK) {
        status = m_JavaVM->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            LOGCATE("HWCodecPlayer::GetJNIEnv failed to attach current thread");
            return nullptr;
        }
        *isAttach = true;
    }
    return env;
}

jobject Muxer::GetJavaObj() {
    return m_JavaObj;
}

JavaVM *Muxer::GetJavaVM() {
    return m_JavaVM;
}
