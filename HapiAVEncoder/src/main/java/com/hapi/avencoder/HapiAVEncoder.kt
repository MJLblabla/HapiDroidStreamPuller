package com.hapi.avencoder

import android.os.Build
import androidx.annotation.RequiresApi
import com.hapi.avparam.*

class HapiAVEncoder {

    val mVideoMediaEncoder: VideoMediaEncoder by lazy { VideoMediaEncoder() }
    val mAudioMediaEncoder: AudioMediaEncoder by lazy { AudioMediaEncoder() }

    private var mParam: EncodeParam? = null

    fun init(param: EncodeParam) {
        mParam = param;
        param.audioEncodeParam?.let {
            mAudioMediaEncoder.configure(param)
        }
        param.videoEncodeParam?.let {
            mVideoMediaEncoder.configure(param)
        }
    }

    fun start() {
        mParam?.audioEncodeParam?.let {
            mAudioMediaEncoder.start()
        }
        mParam?.videoEncodeParam?.let {
            mVideoMediaEncoder.start()
        }
    }

    fun stop() {
        mAudioMediaEncoder.stop()
        mVideoMediaEncoder.stop()
    }

    fun pause() {
        mAudioMediaEncoder.pause()
        mVideoMediaEncoder.pause()
    }

    fun resume() {
        mAudioMediaEncoder.resume()
        mVideoMediaEncoder.resume()
    }

    fun onAudioData(audioFrame: AudioFrame) {
        mAudioMediaEncoder.onFrame(audioFrame)
    }

    fun onVideoData(videoFrame: VideoFrame) {
        mVideoMediaEncoder.onFrame(videoFrame)
    }

    fun release() {
        mAudioMediaEncoder.release()
        mVideoMediaEncoder.release()
    }

}