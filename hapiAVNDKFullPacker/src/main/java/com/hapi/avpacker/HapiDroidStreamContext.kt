package com.hapi.avpacker

import android.util.Log
import com.hapi.avparam.*

class HapiDroidStreamContext {

    companion object {
        // Used to load the 'libstreaming' library on application startup.
        init {
            System.loadLibrary("libstreaming")
        }
    }

    var mConnectEventCallback: ConnectEventCallback? = null

    private var isNativeStart = false

    private var mNativeContextHandler: Long = -1L

    private fun onEventCallback(code: Int) {
        Log.d("HapiDroidStreamContext", "onEventCallback ${code}")
        mConnectEventCallback?.onConnectStatusChange(code)
    }

    fun init() {
        mNativeContextHandler = native_CreateContext()
    }

    fun unit() {
        if(mNativeContextHandler==-1L){
            native_DestroyContext(mNativeContextHandler)
        }
        mNativeContextHandler =-1;
    }

    fun start(url: String, param: EncodeParam): Int {
        if(mNativeContextHandler==-1L){
            return -1;
        }
        val ret = native_Start(
            mNativeContextHandler,
            url,
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
        )
        if (ret == 1) {
            isNativeStart = true
        }
        return ret
    }

    fun stop() {
        if (!isNativeStart) {
            return
        }
        isNativeStart = false
        native_Stop(mNativeContextHandler)
    }

    fun pause() {
        if (!isNativeStart) {
            return
        }
        native_Pause(mNativeContextHandler)
    }

    fun resume() {
        if (!isNativeStart) {
            return
        }
        native_Resume(mNativeContextHandler)
    }

    fun onAudioData(audioFrame: AudioFrame) {
        if (!isNativeStart) {
            return
        }
        native_OnAudioData(
            mNativeContextHandler,
            audioFrame.data,
            audioFrame.audioFormat.ffmpegFMT,
            audioFrame.channelConfig.FFmpegChannel,
            audioFrame.sampleRateInHz
        )
    }

    fun onVideoData(videoFrame: VideoFrame) {
        if (!isNativeStart) {
            return
        }
        native_OnVideoData(
            mNativeContextHandler,
            videoFrame.format.fmt,
            videoFrame.data,
            videoFrame.width,
            videoFrame.height,
            videoFrame.rotationDegrees,
            videoFrame.pixelStride,
            videoFrame.rowPadding
        )
    }

    fun getConnectStatus(): Int {
        if (!isNativeStart) {
            return ConnectedStatus.CONNECTED_STATUS_NULL.intStatus;
        }
        return native_getConnectStatus(mNativeContextHandler);
    }

    fun getMuxerStatus(): Int {
        if (!isNativeStart) {
            return MuxerStatus.STATE_UNKNOWN.intStatus;
        }
        return native_getMuxerStatus(mNativeContextHandler)
    }


    private external fun native_getMuxerStatus(handler: Long): Int;
    private external fun native_getConnectStatus(handler: Long): Int;
    private external fun native_CreateContext(): Long
    private external fun native_DestroyContext(handler: Long)
    private external fun native_Start(
        handler: Long,
        outUrl: String,
        frameWidth: Int,
        frameHeight: Int,
        videoBitRate: Int,
        fps: Int,
        //audio
        out_sample_fmt: Int,
        audioChannelLayout: Int,
        audioSampleRate: Int,
        audioBitrate: Int,
    ): Int

    private external fun native_OnAudioData(
        handler: Long,
        data: ByteArray,
        sample_fmt: Int,
        audioChannelLayout: Int,
        audioSampleRate: Int
    )

    private external fun native_OnVideoData(
        handler: Long,
        format: Int,
        data: ByteArray,
        width: Int,
        height: Int,
        rotationDegrees: Int,
        pixelStride: Int, rowPadding: Int
    )

    private external fun native_Stop(handler: Long)
    private external fun native_Pause(handler: Long)
    private external fun native_Resume(handler: Long)


    interface ConnectEventCallback {
        fun onConnectStatusChange(status: Int)
    }
}