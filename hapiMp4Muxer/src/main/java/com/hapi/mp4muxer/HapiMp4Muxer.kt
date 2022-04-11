package com.hapi.mp4muxer

import android.media.MediaCodec
import android.media.MediaFormat
import android.media.MediaMuxer
import android.os.Build
import android.util.Log
import androidx.annotation.RequiresApi
import com.hapi.avparam.*
import java.nio.ByteBuffer

class HapiMp4Muxer : IMuxer {

    private var isStart = false
    private var isTrackOk = false;
    private var mMuxer: MediaMuxer? = null
    private var mVideoTrackIndex = -1
    private var mAudioTrackIndex = -1
    private var mParam: EncodeParam? = null
    override var mMuxerConnectCallBack: MuxerConnectCallBack? = null
    override var mVideoBitrateRegulatorCallBack: ((Int) -> Unit)?=null
    override var mAudioBitrateRegulatorCallBack: ((Int) -> Unit)?=null

    override val mVideoEncoderCallBack: EncoderCallBack by lazy {
        object : EncoderCallBack {
            override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {
                if (mVideoTrackIndex == -1) {
                    mMuxer?.let {
                        mVideoTrackIndex = it.addTrack(format)
                    }
                }
            }

            override fun onOutputBufferAvailable(
                codec: MediaCodec,
                index: Int,
                info: MediaCodec.BufferInfo
            ) {
                Log.d("HapiMp4Muxer","mVideoEncoderCallBack onOutputBufferAvailable")
                codec.getOutputBuffer(index)?.let {
                    writeData(it, info, mVideoTrackIndex)
                }
            }
        }
    }
    override val mAudioEncoderCallBack: EncoderCallBack by lazy {
        object : EncoderCallBack {
            override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {
                if (mAudioTrackIndex == -1) {
                    mMuxer?.let {
                        mAudioTrackIndex = it.addTrack(format)
                    }
                }
            }

            override fun onOutputBufferAvailable(
                codec: MediaCodec,
                index: Int,
                info: MediaCodec.BufferInfo
            ) {
                Log.d("HapiMp4Muxer","mAudioEncoderCallBack onOutputBufferAvailable")
                codec.getOutputBuffer(index)?.let {
                    writeData(it, info, mAudioTrackIndex)
                }
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    fun writeData(outputBuffer: ByteBuffer, bufferInfo: MediaCodec.BufferInfo, track: Int) {

        if (mParam!!.audioEncodeParam != null && mParam!!.videoEncodeParam != null) {
            if (mAudioTrackIndex == -1 || mVideoTrackIndex == -1) {
                return
            }
        }
        if (!isTrackOk) {
            isTrackOk = true;
            mMuxer?.start()
            mMuxerConnectCallBack?.onConnectedStatus(ConnectedStatus.CONNECTED_STATUS_START)
        }

        if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_CODEC_CONFIG != 0) {
            bufferInfo.size = 0
        } else if (bufferInfo.size != 0) {
            outputBuffer.position(bufferInfo.offset)
            outputBuffer.limit(bufferInfo.offset + bufferInfo.size)
            try {
                mMuxer!!.writeSampleData(track, outputBuffer, bufferInfo)
            } catch (e: Exception) {
                e.printStackTrace()
            }
//            if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
//
//            }
        }
    }

    override fun start(url: String, param: EncodeParam) {
        mParam = param;
        mMuxerConnectCallBack?.onConnectedStatus(ConnectedStatus.CONNECTED_STATUS_CONNECTED)
        mMuxer = MediaMuxer(url, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4)
        isStart = true;
    }

    override fun stop() {
        isStart = false;
        mMuxer?.stop()
        mMuxerConnectCallBack?.onConnectedStatus(ConnectedStatus.CONNECTED_STATUS_CLOSE)
    }

    override fun init() {
    }

    override fun unInit() {
        mMuxerConnectCallBack = null
        mMuxer?.release()
    }
}