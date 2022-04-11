package com.hapi.avencoder

import com.hapi.avparam.*

class AVResampleContext {

    companion object {
        // Used to load the 'avencoder' library on application startup.
        init {
            System.loadLibrary("avencoder")
        }
    }

    private var mNativeContextHandler: Long = -1;
    fun init() {
        if (mNativeContextHandler != -1L) {
            unit()
        }
        mNativeContextHandler = native_CreateContext()
    }

    fun unit() {
        if (mNativeContextHandler == -1L) {
            native_DestroyContext(mNativeContextHandler)
        }
        mNativeContextHandler = -1;
    }

    fun setParam(param: EncodeParam) {
        if (mNativeContextHandler == -1L) {
            return;
        }
        native_Start(
            mNativeContextHandler,

            param.videoEncodeParam?.frameWidth ?: 0,
            param.videoEncodeParam?.frameHeight ?: 0,
            param.videoEncodeParam?.videoBitRate ?: 0,
            param.videoEncodeParam?.fps ?: 0,
            //audio
            param.audioEncodeParam?.audioFormat?.ffmpegFMT
                ?: SampleFormat.ENCODING_PCM_16BIT.ffmpegFMT,
            param.audioEncodeParam?.channelConfig?.FFmpegChannel
                ?: ChannelConfig.LEFT.FFmpegChannel,
            param.audioEncodeParam?.sampleRateInHz ?: 0,
            param.audioEncodeParam?.audioBitrate ?: 0

        );
    }

    fun onAudioData(audioFrame: AudioFrame): ByteArray {
        return native_OnAudioDataNewByte(
            mNativeContextHandler,
            audioFrame.data,
            audioFrame.audioFormat.ffmpegFMT,
            audioFrame.channelConfig.FFmpegChannel,
            audioFrame.sampleRateInHz
        )
    }

    fun onAudioData(audioFrame: AudioFrame, outFrame: ByteArray) {
        native_OnAudioData(
            mNativeContextHandler,
            audioFrame.data,
            audioFrame.audioFormat.ffmpegFMT,
            audioFrame.channelConfig.FFmpegChannel,
            audioFrame.sampleRateInHz, outFrame!!
        )
    }

    fun onVideoData(videoFrame: VideoFrame, outFrame: ByteArray) {
        native_OnVideoData(
            mNativeContextHandler,
            videoFrame.format.fmt,
            videoFrame.data,
            videoFrame.width,
            videoFrame.height,
            videoFrame.rotationDegrees,
            videoFrame.pixelStride,
            videoFrame.rowPadding,
            outFrame
        )
    }

    private external fun native_CreateContext(): Long
    private external fun native_DestroyContext(handler: Long)
    private external fun native_Start(
        handler: Long,

        frameWidth: Int,
        frameHeight: Int,
        videoBitRate: Int,
        fps: Int,
        //audio
        out_sample_fmt: Int,
        audioChannelLayout: Int,
        audioSampleRate: Int,
        audioBitrate: Int,

        )

    private external fun native_OnAudioData(
        handler: Long,
        data: ByteArray,
        sample_fmt: Int,
        audioChannelLayout: Int,
        audioSampleRate: Int,
        outFrame: ByteArray
    )

    private external fun native_OnAudioDataNewByte(
        handler: Long,
        data: ByteArray,
        sample_fmt: Int,
        audioChannelLayout: Int,
        audioSampleRate: Int
    ): ByteArray

    private external fun native_OnVideoData(
        handler: Long,
        format: Int,
        data: ByteArray,
        width: Int,
        height: Int,
        rotationDegrees: Int,
        pixelStride: Int, rowPadding: Int,
        outFrame: ByteArray
    )

}