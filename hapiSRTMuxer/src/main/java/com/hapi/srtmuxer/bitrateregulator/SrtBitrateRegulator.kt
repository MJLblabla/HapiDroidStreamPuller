package com.hapi.srtmuxer.bitrateregulator

import android.util.Log
import android.util.Range
import com.hapi.avparam.EncodeParam
import com.hapi.srtmuxer.SRTConnection
import com.hapi.srtmuxer.Stats
import kotlin.math.max
import kotlin.math.min

class SrtBitrateRegulator {
    companion object {
        const val MINIMUM_DECREASE_THRESHOLD = 100000 // b/s
        const val MAXIMUM_INCREASE_THRESHOLD = 200000 // b/s
        const val SEND_PACKET_THRESHOLD = 50
        const val MIN_PKT_SND_LOSS_THRESHOLD = 5
    }

    var encodeParam: EncodeParam? = null

    var mVideoBitrateRegulatorCallBack: ((Int) -> Unit)? = null
    var mAudioBitrateRegulatorCallBack: ((Int) -> Unit)? = null

    var srtConnection: SRTConnection? = null

    private val onVideoTargetBitrateChange: ((Int) -> Unit) = {
        mVideoBitrateRegulatorCallBack?.invoke(it)
    }
    private val onAudioTargetBitrateChange: ((Int) -> Unit) = {
        mAudioBitrateRegulatorCallBack?.invoke(it)
    }

    private var bitrateRegulatorConfig: BitrateRegulatorConfig? = null

    private var mCurrentVideoBitrate = 0
    private var mCurrentAudioBitrate = 0

    private val mRegulatorJob = Scheduler(5000) {
        srtConnection?.getStats()?.let {
            update(
                it
            )
        }
    }

    fun start() {
        mCurrentVideoBitrate = encodeParam?.videoEncodeParam?.videoBitRate ?: 0
        mCurrentAudioBitrate = encodeParam?.audioEncodeParam?.audioBitrate ?: 0

        bitrateRegulatorConfig = BitrateRegulatorConfig.Builder()
            .setAudioBitrateRange(
                Range(
                    encodeParam?.audioEncodeParam?.minAudioBitRate ?: 0,
                    encodeParam?.audioEncodeParam?.maxAudioBitRate ?: 0
                )
            )
            .setVideoBitrateRange(
                Range(
                    encodeParam?.videoEncodeParam?.minVideoBitRate ?: 0,
                    encodeParam?.videoEncodeParam?.maxVideoBitRate ?: 0
                )
            )
            .build()
        if ((bitrateRegulatorConfig?.videoBitrateRange?.lower ?: 0) <= 0
            || (bitrateRegulatorConfig?.videoBitrateRange?.upper ?: 0) <= 0
        ) {
            return
        }
        mRegulatorJob.start()

    }

    fun stop() {
        mRegulatorJob.cancel()
    }

    private fun update(stats: Stats) {
        Log.d("SrtBitrateRegulator", "SrtBitrateRegulator ${stats.toString()}")
        bitrateRegulatorConfig ?: return
        val estimatedBandwidth = (stats.mbpsBandwidth * 1000000).toInt()
        Log.d("SrtBitrateRegulator", "estimatedBandwidth ${estimatedBandwidth}")

        if (mCurrentVideoBitrate <= 0) {
            return
        }

        if (mCurrentVideoBitrate > bitrateRegulatorConfig!!.videoBitrateRange.lower) {

            Log.d("SrtBitrateRegulator", "  视频 当前码率大于 mim")
            val newVideoBitrate = when {
                stats.pktSndLoss > MIN_PKT_SND_LOSS_THRESHOLD -> {
                    // Detected packet loss - quickly react
                    val result = mCurrentVideoBitrate - max(
                        mCurrentVideoBitrate * 20 / 100, // too late - drop bitrate by 20 %
                        MINIMUM_DECREASE_THRESHOLD // getting down by 100000 b/s minimum
                    )
                    Log.d(
                        "SrtBitrateRegulator",
                        "  丢包 ${stats.pktSndLoss}大于0 降低码率  ${mCurrentVideoBitrate} after ${result}"
                    )
                    result
                }
                stats.pktSndBuf > SEND_PACKET_THRESHOLD -> {
                    // Try to avoid congestion
                    val result = mCurrentVideoBitrate - max(
                        mCurrentVideoBitrate * 10 / 100, // drop bitrate by 10 %
                        MINIMUM_DECREASE_THRESHOLD // getting down by 100000 b/s minimum
                    )
                    Log.d(
                        "SrtBitrateRegulator",
                        " 没有收到ack的包  > 50  ${mCurrentVideoBitrate} after ${result}"
                    )
                    result
                }
                (mCurrentVideoBitrate + mCurrentAudioBitrate) > estimatedBandwidth -> {
                    // Estimated bitrate too low
                    val result = estimatedBandwidth - mCurrentAudioBitrate
                    Log.d(
                        "SrtBitrateRegulator",
                        " 带宽过大  > 50  ${mCurrentVideoBitrate} after ${result}"
                    )
                    result
                }
                else -> 0
            }

            if (newVideoBitrate != 0) {
                val after = max(
                    newVideoBitrate,
                    bitrateRegulatorConfig!!.videoBitrateRange.lower
                )
                onVideoTargetBitrateChange(
                    after
                ) // Don't go under videoBitrateRange.lower
                mCurrentVideoBitrate = after

                Log.d(
                    "SrtBitrateRegulator",
                    " 调节后码率 ${after}"
                )
                return
            }
        }

        // Can bitrate go upper?
        if (mCurrentVideoBitrate < bitrateRegulatorConfig!!.videoBitrateRange.upper) {
            val newVideoBitrate = when {
                (mCurrentVideoBitrate + mCurrentAudioBitrate) < estimatedBandwidth -> {

                    val result = mCurrentVideoBitrate + min(
                        (bitrateRegulatorConfig!!.videoBitrateRange.upper - mCurrentVideoBitrate) * 50 / 100, // getting slower when reaching target bitrate
                        MAXIMUM_INCREASE_THRESHOLD // not increasing to fast
                    )
                    Log.d(
                        "SrtBitrateRegulator",
                        " 带宽有上升空间   ${mCurrentVideoBitrate} after ${result}"
                    )

                    result
                }
                else -> 0
            }

            if (newVideoBitrate != 0) {
                val after = max(
                    newVideoBitrate,
                    bitrateRegulatorConfig!!.videoBitrateRange.lower
                )

                onVideoTargetBitrateChange(
                    after
                ) // Don't go under videoBitrateRange.lower
                mCurrentVideoBitrate = after
                Log.d(
                    "SrtBitrateRegulator",
                    " 调节后码率 ${after}"
                )
                return
            }
        }
    }

}