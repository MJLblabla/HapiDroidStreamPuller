#ifndef HAPI_HWEncoder_H
#define HAPI_HWEncoder_H

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>
#include <thread>
#include <unistd.h>
#include "EncodeParam.h"
#include "LogUtil.h"
#include "ThreadSafeQueue.h"

#define MEDIACODEC_BITRATE_MODE_CQ  0 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CQ
#define MEDIACODEC_BITRATE_MODE_VBR 1 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR
#define MEDIACODEC_BITRATE_MODE_CBR 2 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR
#define MEDIACODEC_TIMEOUT_USEC 100000//us
using namespace std;

template<class T>
class HWEncoder {

protected:
    AMediaCodec *media_codec_;
    AMediaFormat *media_format_;
    //数据队列
    ThreadSafeQueue<T *> mFrameQueue;
    //参数
    EncodeParam mEncodeParam;
    //编码器线程
    thread *encoderThread = nullptr;

    volatile bool m_Exit = true;
    volatile bool isStart = false;

//编码码器状态
    volatile int m_EncoderState = STATE_UNKNOWN;
    //同步
    std::mutex m_Mutex;
    std::condition_variable m_Cond;

    long startTime = 0;
    int baseTime = 0;
    int position = 0;
    int frameIndex = 0;

    virtual void configAMediaCodec() = 0;

    virtual int encodeFrame(T *frame) = 0;

private:
    static void sloop(HWEncoder *encoder) {
        encoder->loopFrame();
    }

    void loopFrame() {
        m_EncoderState = STATE_DECODING;

        position = 0;
        frameIndex = 0;
        startTime = GetSysCurrentTimeNS();

        while (!m_Exit) {
            while (m_EncoderState == STATE_PAUSE) {
                std::unique_lock<std::mutex> lock(m_Mutex);
                LOGCATE("DeMuxer::DecodingLoop waiting, m_MediaType");
                m_Cond.wait_for(lock, std::chrono::milliseconds(10));
            }
            if (m_EncoderState == STATE_STOP) {
                LOGCATE("DeMuxer::DecodingLoop  stop break thread");
                break;
            }
            if (m_Exit) {
                break;
            }
            while (mFrameQueue.Empty() && !m_Exit) {
                usleep(10 * 1000);
            }

            T *frame = mFrameQueue.Pop();
            if (frame == nullptr) {
                break;
            }
            int pts = encodeFrame(frame);

            if (pts >= 0) {
                position = pts;
                frameIndex++;
            }
            recvFrame();

            delete frame;
        }
        while (!mFrameQueue.Empty()) {
            T *frame = mFrameQueue.Pop();
            delete frame;
        }
    }

    void recvFrame() {
        while (!m_Exit) {
            AMediaCodecBufferInfo info;
            ssize_t status = AMediaCodec_dequeueOutputBuffer(media_codec_, &info, 1);

            if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                break;
            } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
                continue;
            } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                AMediaFormat *format = AMediaCodec_getOutputFormat(media_codec_);
                dequeueOutputBufferCall(status, &info, format, nullptr);
                AMediaFormat_delete(format);
                continue;
            } else {
                if (status < 0) {
                    return;
                }
                uint8_t *encodeData = AMediaCodec_getOutputBuffer(media_codec_, status,
                                                                  NULL/* out_size */);
                dequeueOutputBufferCall(status, &info, media_format_, encodeData);
                AMediaCodec_releaseOutputBuffer(media_codec_, status, false);
                if ((info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0) {
                    break;
                }
            }
        }

    }

public:

    ~HWEncoder() {
        if (isStart) {
            stop();
        }
        dequeueOutputBufferCall = nullptr;
        // mEncodeParam = nullptr;
        while (!mFrameQueue.Empty()) {
            T *frame = mFrameQueue.Pop();
            delete frame;
        }
    }

    void configMediaCodec(EncodeParam param) {
        mEncodeParam = param;
        configAMediaCodec();
    }

    void start() {
        m_Exit = false;
        encoderThread = new thread(sloop, this);
    }

    void pause() {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_EncoderState = STATE_PAUSE;
    }

    void resume() {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_EncoderState = STATE_DECODING;
        m_Cond.notify_all();
    }

    void stop() {
        if (media_codec_) {
            AMediaCodec_flush(media_codec_);
        }
        m_Exit = true;
        isStart = false;
        m_EncoderState = STATE_STOP;
        if (encoderThread != nullptr) {
            encoderThread->join();
            delete encoderThread;
            encoderThread = nullptr;
        }
        startTime = 0;
        startTime = 0;
        position = 0;
        frameIndex = 0;

        if (media_format_) {
            try {
                AMediaFormat_delete(media_format_);
            } catch (const exception &e) { //
                e.what();
            }
            media_format_ = nullptr;
        }

        if (media_codec_) {
            AMediaCodec_stop(media_codec_);
            AMediaCodec_delete(media_codec_);
            media_codec_ = nullptr;
        }
    }

    void pushFrame(T *frame) {
        mFrameQueue.Push(frame);
    }

    int sendFrame(uint8_t *data, int size, int pts) {

        ssize_t bufidx = AMediaCodec_dequeueInputBuffer(media_codec_, MEDIACODEC_TIMEOUT_USEC);
        if (bufidx < 0) {
            return -1;
        }
        if (!data) {
            AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, 0, pts,
                                         AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
            return pts;
        }
        size_t bufsize = 0;
        uint8_t *buf = AMediaCodec_getInputBuffer(media_codec_, bufidx, &bufsize);
        if (!buf) {
            LOGCATE("%s %d AMediaCodec_dequeueInputBuffer fail", __FUNCTION__, __LINE__);
            return -1;
        }
        memcpy(buf, data, size);
        media_status_t status = AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, size, pts, 0);
        LOGCATE("%s %d AMediaCodec_queueInputBuffer status (%d)", __FUNCTION__, __LINE__, status);

        return pts;
    }

    long getPosition() {
        return position;
    }

    int getFrameCachedSize() {
        return mFrameQueue.Size();
    }

//回调
    EncoderCall dequeueOutputBufferCall = nullptr;
};

#endif