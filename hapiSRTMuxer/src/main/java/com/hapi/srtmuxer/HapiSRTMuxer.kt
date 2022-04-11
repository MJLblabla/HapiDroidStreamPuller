package com.hapi.srtmuxer

import android.media.MediaCodec
import android.media.MediaFormat
import android.net.Uri
import android.util.Log
import com.hapi.avparam.*
import com.hapi.srtmuxer.bitrateregulator.SrtBitrateRegulator
import com.hapi.srtmuxer.mode.Boundary
import com.hapi.srtmuxer.mode.MsgCtrl
import com.hapi.srtmuxer.mode.SRTConfig
import com.hapi.srtmuxer.ts.Frame
import com.hapi.srtmuxer.ts.IMuxerListener
import com.hapi.srtmuxer.ts.Packet
import com.hapi.srtmuxer.ts.TSMuxer
import com.hapi.srtmuxer.ts.data.ServiceInfo
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.net.InetAddress

class HapiSRTMuxer : IMuxer {
    private var audioTsStreamId: Short? = null
    private var videoTsStreamId: Short? = null
    private val tsServiceInfo: ServiceInfo by lazy {
        ServiceInfo(
            ServiceInfo.ServiceType.DIGITAL_TV,
            0x4698,
            "HapiSRTMuxer",
            "HapiSRTMuxer"
        )
    }

    private var mParam: EncodeParam? = null
    override var mVideoBitrateRegulatorCallBack: ((Int) -> Unit)? = null
    override var mAudioBitrateRegulatorCallBack: ((Int) -> Unit)? = null
    private val mSRTConnection = SRTConnection()
    private val mMuxerListener = object : IMuxerListener {
        override fun onOutputFrame(packet: Packet) {

            val boundary = when {
                packet.isFirstPacketFrame && packet.isLastPacketFrame -> Boundary.SOLO
                packet.isFirstPacketFrame -> Boundary.FIRST
                packet.isLastPacketFrame -> Boundary.LAST
                else -> Boundary.SUBSEQUENT
            }
            val msgCtrl =
                if (packet.ts == 0L) {
                    MsgCtrl(boundary = boundary)
                } else {
                    MsgCtrl(
                        ttl = 500,
                        srcTime = packet.ts,
                        boundary = boundary
                    )
                }
            mSRTConnection.send(
                packet.buffer,
                msgCtrl.ttl,
                msgCtrl.srcTime,
                msgCtrl.boundary.intv
            )
        }
    }
    private val tsMuxer by lazy { TSMuxer(mMuxerListener) }
    override var mMuxerConnectCallBack: MuxerConnectCallBack? = null

    override val mVideoEncoderCallBack: EncoderCallBack by lazy {
        object : EncoderCallBack {
            override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {}
            override fun onOutputBufferAvailable(
                codec: MediaCodec,
                index: Int,
                info: MediaCodec.BufferInfo
            ) {
                val buffer = codec.getOutputBuffer(index) ?: return;
                if (info.flags == MediaCodec.BUFFER_FLAG_CODEC_CONFIG) {
                    return
                }
                val isKey = info.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME
                tsMuxer.encode(
                    Frame(
                        buffer,
                        codec.outputFormat.getString(MediaFormat.KEY_MIME)!!,
                        System.nanoTime() / 1000,// info.presentationTimeUs, // pts
                        null, // dts
                        isKey,
                        if (isKey) {
                            videoGenerateExtra(codec.outputFormat)
                        } else {
                            null
                        }
                    ), videoTsStreamId!!
                )
            }
        }
    }

    override val mAudioEncoderCallBack: EncoderCallBack by lazy {
        object : EncoderCallBack {
            override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {}
            override fun onOutputBufferAvailable(
                codec: MediaCodec,
                index: Int,
                info: MediaCodec.BufferInfo
            ) {
                val buffer = codec.getOutputBuffer(index) ?: return;
                if (info.flags == MediaCodec.BUFFER_FLAG_CODEC_CONFIG) {
                    return
                }
                tsMuxer.encode(
                    Frame(
                        buffer,
                        codec.outputFormat.getString(MediaFormat.KEY_MIME)!!,
                        System.nanoTime() / 1000,//  info.presentationTimeUs, // pts
                        null, // dts
                        info.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME,
                        audioGenerateExtra(buffer, codec.outputFormat)
                    ), audioTsStreamId!!
                )
            }
        }
    }

    private val mSrtBitrateRegulator by lazy {
        val srtBitrateRegulator = SrtBitrateRegulator()
        srtBitrateRegulator.mVideoBitrateRegulatorCallBack = mVideoBitrateRegulatorCallBack
        srtBitrateRegulator.mAudioBitrateRegulatorCallBack = mAudioBitrateRegulatorCallBack
        srtBitrateRegulator.srtConnection = mSRTConnection
        srtBitrateRegulator
    }

    init {
        tsMuxer.addService(tsServiceInfo)
        val streams = mutableListOf<String>()
        MediaFormat.MIMETYPE_VIDEO_AVC.let { streams.add(it) }
        MediaFormat.MIMETYPE_AUDIO_AAC.let { streams.add(it) }
        tsMuxer.addStreams(tsServiceInfo, streams).let {
            videoTsStreamId = it[0]
            audioTsStreamId = it[1]
        }
        mSRTConnection.mConnectEventCallback = {
            if (it == ConnectedStatus.CONNECTED_STATUS_START.intStatus) {
                mSrtBitrateRegulator.start()
            }
            mMuxerConnectCallBack?.onConnectedStatus(it.toConnectedStatus())
        }
    }

    override fun start(url: String, param: EncodeParam) {
        mParam = param
        mSrtBitrateRegulator.encodeParam = mParam;

        GlobalScope.launch(Dispatchers.IO) {
            val router = Uri.parse(url)
            val scheme = router.scheme
            val inetAddress = InetAddress.getByName(router.host)
            val ip: String? = inetAddress.hostAddress // 获取主机ip
            val path = router.path
            val port = router.port
            var streamId = router.getQueryParameter("streamid")!!
            if (streamId.isEmpty()) {
                val ar = url.split("streamid=")
                streamId = ar[1]
            }
            val config = SRTConfig()
            config.streamId = streamId;
            config.port = port
            config.ipAddress = ip ?: ""
            config.maxBW = 0
            config.inputBW =
                (param.audioEncodeParam?.audioBitrate ?: 0) + (param.videoEncodeParam?.videoBitRate
                    ?: 0)
            mSRTConnection.config(config)
            mSRTConnection.connect()
        }

    }

    override fun stop() {
        mSrtBitrateRegulator.stop()
        mSRTConnection.close()
    }

    override fun init() {
        mSRTConnection.init()
    }

    override fun unInit() {
        mSRTConnection.unInit()
    }
}