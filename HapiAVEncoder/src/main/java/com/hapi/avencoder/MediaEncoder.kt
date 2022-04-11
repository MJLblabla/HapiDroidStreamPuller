package com.hapi.avencoder

import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import com.hapi.avparam.*
import java.lang.Exception
import java.util.concurrent.LinkedBlockingDeque
import java.util.concurrent.TimeUnit


abstract class MediaEncoder<T> {

    protected open val tag = ""
    var mEncoderCallBack: EncoderCallBack? = null

    var mCodingStatus: CodingStatus = CodingStatus.STATE_UNKNOWN
        private set
    private val lock = Object()
    protected var mediaCodec: MediaCodec? = null;
    protected var mediaFormat: MediaFormat? = null
    protected var mEncodeParam = EncodeParam()
    private val mEncoderHandler: android.os.Handler by lazy {
        val handlerThread = HandlerThread("MediaEncoder")
        handlerThread.start()
        Handler(handlerThread.looper)
    }
    private val mFrameQueue: LinkedBlockingDeque<T> by lazy {
        LinkedBlockingDeque<T>()
    }
    protected val mAVResampleContext = AVResampleContext()
    protected abstract fun configure()
    protected abstract fun consumeFrame(frame: T?, codec: MediaCodec, id: Int)

    fun updateBitRate(bitRate: Int) {
        val bundle = Bundle()
        bundle.putInt(MediaCodec.PARAMETER_KEY_VIDEO_BITRATE, bitRate)
        mediaCodec?.setParameters(bundle)
    }

    private val mMediaCodecCallBack by lazy {
        object : MediaCodec.Callback() {
            override fun onInputBufferAvailable(codec: MediaCodec, id: Int) {
                Log.d("MediaEncoder${tag}", "onInputBufferAvailable: MediaCodec")
                if (mCodingStatus == CodingStatus.STATE_STOP ||
                    mCodingStatus == CodingStatus.STATE_UNKNOWN
                ) {
                    return
                }
                var frame: T? = null// = mFrameQueue.poll(200, TimeUnit.MILLISECONDS)
                while (
                    frame == null &&
                    (mCodingStatus != CodingStatus.STATE_STOP ||
                            mCodingStatus != CodingStatus.STATE_UNKNOWN
                            )
                ) {
                    frame = mFrameQueue.poll(200, TimeUnit.MILLISECONDS)
                }
                synchronized(lock) {
                    if (mCodingStatus == CodingStatus.STATE_STOP ||
                        mCodingStatus == CodingStatus.STATE_UNKNOWN
                    ) {
                        return
                    }
                    try {
                        val inputBuffer = codec.getInputBuffer(id)
                        inputBuffer!!.clear()
                        consumeFrame(frame, codec, id)

                    } catch (e: IllegalStateException) {
                        e.printStackTrace()
                    }
                    frame?.let {
                        mFrameQueue.poll()
                    }
                }
            }

            override fun onOutputBufferAvailable(
                codec: MediaCodec,
                index: Int,
                info: MediaCodec.BufferInfo
            ) {
                Log.d("MediaEncoder${tag}", "onOutputBufferAvailable: MediaCodec")
                synchronized(lock) {
                    if (mCodingStatus == CodingStatus.STATE_STOP ||
                        mCodingStatus == CodingStatus.STATE_UNKNOWN
                    ) {
                        return
                    }

                    mEncoderCallBack?.onOutputBufferAvailable(codec, index, info)
                    codec.releaseOutputBuffer(index, false)
                }
            }

            override fun onError(p0: MediaCodec, p1: MediaCodec.CodecException) {
                Log.d("MediaEncoder${tag}", "onError(p0: MediaCodec")
                p1.printStackTrace()
            }

            override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {
                Log.d("MediaEncoder${tag}", "onOutputFormatChanged: MediaCodec")
                mEncoderCallBack?.onOutputFormatChanged(codec, format)
            }
        }
    }

    fun configure(encodeParam: EncodeParam) {
        mAVResampleContext.init()
        mAVResampleContext.setParam(encodeParam)
        mEncodeParam = encodeParam;
        configure()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mediaCodec!!.setCallback(mMediaCodecCallBack, mEncoderHandler)
        } else {
            mediaCodec!!.setCallback(mMediaCodecCallBack)
        }
    }

    open fun start() {
        synchronized(lock) {
            mediaCodec!!.start()
            mEncoderCallBack?.onConfigure(mediaCodec!!, mediaFormat!!)
            mCodingStatus = CodingStatus.STATE_DECODING
        }
    }

    open fun stop(): Boolean {
        synchronized(lock) {
            if (mediaCodec == null || mCodingStatus == CodingStatus.STATE_STOP || mCodingStatus == CodingStatus.STATE_UNKNOWN) {
                return false
            }
            //  mediaCodec?.setCallback(null)
            // mediaCodec?.signalEndOfInputStream()
            mediaCodec!!.flush()
            mediaCodec!!.stop()

            mCodingStatus = CodingStatus.STATE_STOP
            mFrameQueue.clear()
        }
        return true
    }

    fun pause(): Boolean {
        if (mediaCodec == null || mCodingStatus != CodingStatus.STATE_DECODING) {
            return false
        }
        mCodingStatus = CodingStatus.STATE_PAUSE
        return true
    }

    fun resume(): Boolean {
        if (mediaCodec == null || mCodingStatus != CodingStatus.STATE_PAUSE) {
            return false
        }
        mCodingStatus = CodingStatus.STATE_DECODING
        return true
    }

    fun release() {
        mCodingStatus = CodingStatus.STATE_UNKNOWN
        mAVResampleContext.unit()
    }

    fun onFrame(frame: T) {
        if (mediaCodec == null
            || mCodingStatus != CodingStatus.STATE_DECODING
        ) {
            return
        }
        if (mFrameQueue.size > 10) {
            return
        }
        mFrameQueue.push(frame)
    }
}

class VideoMediaEncoder : MediaEncoder<VideoFrame>() {
    override val tag = "video"
    private var outFrame: ByteArray? = null
    private var startTime = 0L;
    override fun configure() {
        val m_width = mEncodeParam.videoEncodeParam!!.frameWidth
        val m_height = mEncodeParam.videoEncodeParam!!.frameHeight

        mediaCodec = MediaCodec.createEncoderByType("video/avc")
        mediaFormat = MediaFormat.createVideoFormat("video/avc", m_width, m_height)
        mediaFormat?.setInteger(
            MediaFormat.KEY_BIT_RATE,
            mEncodeParam.videoEncodeParam!!.videoBitRate
        )
        mediaFormat?.setInteger(
            MediaFormat.KEY_FRAME_RATE,
            mEncodeParam.videoEncodeParam!!.fps
        )

        mediaFormat?.setInteger(
            MediaFormat.KEY_COLOR_FORMAT,
            19//   MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible
        )
        mediaFormat?.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1) //关键帧间隔时间 单位s

        //   mediaFormat.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR);
        //   mediaFormat.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR);
        mediaCodec!!.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)
        startTime = System.nanoTime()

        outFrame = ByteArray(m_width * m_height * 3 / 2)
    }

    override fun stop(): Boolean {
        startTime = 0L
        return super.stop()
    }


    override fun consumeFrame(frame: VideoFrame?, codec: MediaCodec, id: Int) {
        var len = 0
        val now = System.nanoTime()
        val pts = (now - startTime) / 1000;

        if (frame != null) {
            val inputBuffer = codec.getInputBuffer(id)
            if (
                frame.width != mEncodeParam.videoEncodeParam!!.frameWidth ||
                frame.height != mEncodeParam.videoEncodeParam!!.frameHeight ||
                frame.rotationDegrees == 0
            ) {
                mAVResampleContext.onVideoData(frame, outFrame!!)
                len = outFrame!!.size
                inputBuffer?.put(outFrame!!)
            } else {
                inputBuffer?.put(frame.data)
            }
        }
        codec.queueInputBuffer(id, 0, len, pts, 0)
    }
}

class AudioMediaEncoder : MediaEncoder<AudioFrame>() {

    override val tag = "audio"
    private var m_SamplesCount = 0;
    private var baseTime = 0L;
    private var outFrame: ByteArray? = null

    override fun configure() {
        mediaFormat =
            MediaFormat.createAudioFormat(
                MediaFormat.MIMETYPE_AUDIO_AAC,
                mEncodeParam.audioEncodeParam!!.sampleRateInHz,
                mEncodeParam.audioEncodeParam!!.channelConfig.count
            )
        mediaFormat?.setInteger(
            MediaFormat.KEY_BIT_RATE,
            mEncodeParam.audioEncodeParam!!.audioBitrate
        )
        mediaFormat?.setInteger(
            MediaFormat.KEY_AAC_PROFILE,
            MediaCodecInfo.CodecProfileLevel.AACObjectLC
        )
        mediaFormat?.setInteger(
            MediaFormat.KEY_CHANNEL_COUNT,
            mEncodeParam.audioEncodeParam!!.channelConfig.count
        )
        mediaFormat?.setInteger(
            MediaFormat.KEY_SAMPLE_RATE,
            mEncodeParam.audioEncodeParam!!.sampleRateInHz
        )
        mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC)
        mediaCodec?.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)

        baseTime = (mEncodeParam.audioEncodeParam!!.sampleRateInHz
                * mEncodeParam.audioEncodeParam!!.audioFormat.deep
                * mEncodeParam.audioEncodeParam!!.channelConfig.count).toLong()
    }


    override fun stop(): Boolean {
        baseTime = 0
        m_SamplesCount = 0
        outFrame = null
        return super.stop()
    }

    override fun consumeFrame(frame: AudioFrame?, codec: MediaCodec, id: Int) {


        val pts = (m_SamplesCount * 1000000.0 / baseTime)
        var len = 0;
        if (frame != null) {
            val inputBuffer = codec.getInputBuffer(id)
            if (
                frame.audioFormat != mEncodeParam.audioEncodeParam!!.audioFormat ||
                frame.channelConfig != mEncodeParam.audioEncodeParam!!.channelConfig ||
                frame.sampleRateInHz != mEncodeParam.audioEncodeParam!!.sampleRateInHz
            ) {
                if (outFrame == null) {
                    outFrame = mAVResampleContext.onAudioData(frame)
                } else {
                    mAVResampleContext.onAudioData(frame, outFrame!!)
                }

                inputBuffer?.put(outFrame!!)
                len = outFrame!!.size
            } else {
                inputBuffer?.put(frame.data)
                len = frame.data.size
            }
        }
        m_SamplesCount += len

        codec.queueInputBuffer(id, 0, len, pts.toLong(), 0)
    }
}