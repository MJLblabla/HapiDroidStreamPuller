package com.hapi.rtmpmuxer

import android.media.MediaCodec
import android.media.MediaFormat
import android.util.Log
import com.hapi.avparam.*
import java.nio.ByteBuffer

class HapiRTMPMuxer : IMuxer {

    private val mRTMPConnection by lazy {
        RTMPConnection().apply {
            mConnectEventCallback = {
                mMuxerConnectCallBack?.onConnectedStatus(it.toConnectedStatus(), "")
            }
        }
    }
    override var mMuxerConnectCallBack: MuxerConnectCallBack? = null

    override var mVideoBitrateRegulatorCallBack: ((Int) -> Unit)? = null
    override var mAudioBitrateRegulatorCallBack: ((Int) -> Unit)? = null

    override val mVideoEncoderCallBack: EncoderCallBack
            by lazy {
                object : EncoderCallBack {
                    override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {}

                    override fun onOutputBufferAvailable(
                        codec: MediaCodec,
                        index: Int,
                        info: MediaCodec.BufferInfo
                    ) {
                        val buffer = codec.getOutputBuffer(index) ?: return;
                        if (info.size <= 0) {
                            return
                        }
                        var spsLen: Int = 0
                        var ppsLen: Int = 0
                        val byteBuf = if (info.flags == MediaCodec.BUFFER_FLAG_CODEC_CONFIG) {
                            val csd0 = codec.outputFormat.getByteBuffer("csd-0")
                            val csd1 = codec.outputFormat.getByteBuffer("csd-1")
                            spsLen = csd0?.limit() ?: 0
                            ppsLen = csd1?.limit() ?: 0
                            buffer
                        } else {
                            //  buffer.rewind()
                            buffer
                        }
                        val t = buffer[4];

                        mRTMPConnection.sendFrame(
                            byteBuf, RTMPConnection.AVFrameType.TYPE_VIDEO,
                            spsLen, ppsLen
                        )
                    }
                }
            }
    override val mAudioEncoderCallBack: EncoderCallBack
            by lazy {
                object : EncoderCallBack {
                    override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {}

                    override fun onOutputBufferAvailable(
                        codec: MediaCodec,
                        index: Int,
                        info: MediaCodec.BufferInfo
                    ) {
                        val buffer = codec.getOutputBuffer(index) ?: return;

                        if (info.size <= 0) {
                            return
                        }

                        val spsLen: Int = 0
                        val ppsLen: Int = 0

                        mRTMPConnection.sendFrame(
                            buffer, RTMPConnection.AVFrameType.TYPE_AUDIO,
                            spsLen, ppsLen
                        )
                    }
                }
            }


    override fun start(url: String, param: EncodeParam) {
        mRTMPConnection.setUrl(url)
        mRTMPConnection.connect()
    }

    override fun stop() {
        mRTMPConnection.close()
    }

    override fun init() {
        mRTMPConnection.init()
    }

    override fun unInit() {
        mRTMPConnection.unInit()
    }
}