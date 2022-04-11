
package com.hapi.srtmuxer.ts.packets


import com.hapi.srtmuxer.ts.Frame
import com.hapi.srtmuxer.ts.IMuxerListener
import com.hapi.srtmuxer.ts.data.Stream
import com.hapi.srtmuxer.ts.descriptors.AdaptationField
import com.hapi.srtmuxer.ts.isAudio
import com.hapi.srtmuxer.ts.isVideo
import com.hapi.srtmuxer.ts.packets.Pes.StreamId.Companion.fromMimeType
import com.hapi.srtmuxer.ts.utils.TimeUtils

class Pes(
    muxerListener: IMuxerListener,
    val stream: Stream,
    private val hasPcr: Boolean,
) : TS(muxerListener, stream.pid) {
    fun write(frame: Frame) {
        val programClockReference = if (hasPcr) {
            TimeUtils.currentTime()
        } else {
            null
        }
        val adaptationField = AdaptationField(
            discontinuityIndicator = stream.discontinuity,
            randomAccessIndicator = frame.isKeyFrame,
            programClockReference = programClockReference
        )

        val header = PesHeader(
            streamId = fromMimeType(stream.mimeType).value,
            payloadLength = frame.buffer.remaining().toShort(),
            pts = frame.pts,
            dts = frame.dts
        )

        write(frame.buffer, adaptationField.toByteBuffer(), header.toByteBuffer(), true, frame.pts)
    }

    enum class StreamId(val value: Short) {
        PRIVATE_STREAM_1(0xbd.toShort()),
        AUDIO_STREAM_0(0xc0.toShort()),
        VIDEO_STREAM_0(0xe0.toShort()),
        METADATA_STREAM(0xfc.toShort()),
        EXTENDED_STREAM(0xfd.toShort());

        companion object {
            fun fromMimeType(mimeType: String): StreamId {
                return when {
                    mimeType.isVideo() -> {
                        VIDEO_STREAM_0
                    }
                    mimeType.isAudio() -> {
                        AUDIO_STREAM_0
                    }
                    else -> {
                        METADATA_STREAM
                    }
                }
            }
        }
    }
}