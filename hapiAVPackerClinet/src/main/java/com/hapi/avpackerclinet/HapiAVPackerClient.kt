package com.hapi.avpackerclinet

import android.media.MediaCodec
import android.media.MediaFormat
import android.net.Uri
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver

import androidx.lifecycle.LifecycleOwner
import com.hapi.avcapture.FrameCall
import com.hapi.avcapture.IAudioTrack
import com.hapi.avcapture.IVideoTrack
import com.hapi.avcapture.Track
import com.hapi.avencoder.HapiAVEncoder
import com.hapi.avparam.*
import java.lang.Exception
import java.util.*
import kotlin.jvm.Throws

class HapiAVPackerClient : LifecycleEventObserver {

    private class InternalFrameCall<T>(
        val mEncoder: HapiAVEncoder,
        val track: Track<T>,
        var mute: Boolean = false
    ) : FrameCall<T> {
        override fun onFrame(frame: T) {
            if (mute) {
                return
            }
            if (frame is VideoFrame) {
                mEncoder.onVideoData(frame)
            }
            if (frame is AudioFrame) {
                mEncoder.onAudioData(frame)
            }
        }
    }

    var mMuxerConnectCallBack: MuxerConnectCallBack? = null

    private val mInnerMuxerConnectCallBack = object : MuxerConnectCallBack {
        override fun onConnectedStatus(status: ConnectedStatus, msg: String?) {
            if (status == ConnectedStatus.CONNECTED_STATUS_CONNECTED) {
                mHapiAVEncoder.start()
            }
            mMuxerConnectCallBack?.onConnectedStatus(status, msg)
        }
    }

    private val mInternalFrameCalls = LinkedList<InternalFrameCall<*>>()
    private var mMuxer: IMuxer? = null

    private val mHapiAVEncoder: HapiAVEncoder by lazy {
        HapiAVEncoder()
    }

    private fun <T> findInternalVFrameCall(track: Track<T>): InternalFrameCall<*>? {
        mInternalFrameCalls.forEach {
            if (it.track == track) {
                return it
            }
        }
        return null
    }


    fun attachTrack(track: Track<*>): Boolean {
        return if (findInternalVFrameCall(track) == null) {
            if (track is IAudioTrack) {
                val call = InternalFrameCall<AudioFrame>(mHapiAVEncoder, track)
                mInternalFrameCalls.add(call)
                track.innerFrameCalls.add(call)
                true
            } else
                if (track is IVideoTrack) {
                    val call = InternalFrameCall<VideoFrame>(mHapiAVEncoder, track)
                    mInternalFrameCalls.add(call)
                    track.innerFrameCalls.add(call)
                    true
                } else {
                    false
                }
        } else {
            false
        }
    }

    fun detachTrack(track: Track<*>) {
        val index =
            findInternalVFrameCall(track)?.let {
                mInternalFrameCalls.remove(it)
                if (track is IAudioTrack) {
                    track.innerFrameCalls.remove(it as FrameCall<*>)
                }
                if (track is IVideoTrack) {
                    track.innerFrameCalls.remove(it as FrameCall<*>)
                }
            }
    }

    fun muteTrack(isMute: Boolean, track: Track<*>): Boolean {
        val call = findInternalVFrameCall(track)
        return if (call != null) {
            call.mute = isMute
            true
        } else {
            false
        }
    }

    @Throws(Exception::class)
    fun start(url: String, param: EncodeParam) {
        mMuxer?.unInit()
        val router = Uri.parse(url)
        val scheme = router.scheme
        mMuxer = when {
            url.endsWith("mp4") -> reflectionMuxer("com.hapi.mp4muxer.HapiMp4Muxer")
            scheme == "srt" -> reflectionMuxer("com.hapi.srtmuxer.HapiSRTMuxer")
            scheme == "rtmp" -> reflectionMuxer("com.hapi.rtmpmuxer.HapiRTMPMuxer")
            else -> throw  Exception(" unSupport url")
        }
        mMuxer?.init()
        mMuxer!!.mMuxerConnectCallBack = mInnerMuxerConnectCallBack
        mMuxer?.let {
            mHapiAVEncoder.mVideoMediaEncoder.mEncoderCallBack = it.mVideoEncoderCallBack
            mHapiAVEncoder.mAudioMediaEncoder.mEncoderCallBack = it.mAudioEncoderCallBack
        }

        mMuxer?.mAudioBitrateRegulatorCallBack = {
            mHapiAVEncoder.mAudioMediaEncoder.updateBitRate(it)
        }

        mMuxer?.mVideoBitrateRegulatorCallBack = {
            mHapiAVEncoder.mVideoMediaEncoder.updateBitRate(it)
        }

        mHapiAVEncoder.init(param)
        mMuxer?.start(url, param)
    }

    private fun reflectionMuxer(className: String): IMuxer {
        val clz = Class.forName(className)
        return clz.newInstance() as IMuxer
    }

    fun stop() {
        mHapiAVEncoder.stop()
        mMuxer?.stop()
    }

    fun pause() {
        mHapiAVEncoder.pause()
    }

    fun resume() {
        mHapiAVEncoder.resume()
    }

    fun release() {
        mMuxer?.unInit()
        mMuxer = null;
        mHapiAVEncoder.mVideoMediaEncoder.mEncoderCallBack = null
        mHapiAVEncoder.mAudioMediaEncoder.mEncoderCallBack = null
        mHapiAVEncoder.release()
    }

    override fun onStateChanged(source: LifecycleOwner, event: Lifecycle.Event) {
        if (event == Lifecycle.Event.ON_DESTROY) {
            stop()
            release()
        }
    }
}