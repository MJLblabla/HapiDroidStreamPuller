package com.hapi.srtmuxer.ts.descriptors


import com.hapi.srtmuxer.ts.data.ITSElement
import com.hapi.srtmuxer.ts.utils.TSConst
import com.hapi.srtmuxer.ts.utils.put
import com.hapi.srtmuxer.ts.utils.putShort
import com.hapi.srtmuxer.ts.utils.shl
import java.nio.ByteBuffer
import kotlin.math.pow

class AdaptationField(
    private val discontinuityIndicator: Boolean = false,
    private val randomAccessIndicator: Boolean = false,
    private val elementaryStreamPriorityIndicator: Boolean = false,
    private val programClockReference: Long? = null,
    private val originalProgramClockReference: Long? = null,
    private val spliceCountdown: Byte? = null,
    private val transportPrivateData: ByteBuffer? = null,
    private val adaptationFieldExtension: ByteBuffer? = null
) : ITSElement {
    override val bitSize = computeBitSize()
    override val size = bitSize / Byte.SIZE_BITS

    private val adaptationFieldLength = size - 1 // 1 - adaptationField own size

    private fun computeBitSize(): Int {
        var nBits = 16 // 16 - header
        programClockReference?.let { nBits += 48 }
        originalProgramClockReference?.let { nBits += 48 }
        spliceCountdown?.let { nBits += 8 }
        transportPrivateData?.let { nBits += transportPrivateData.remaining() }
        adaptationFieldExtension?.let { nBits += adaptationFieldExtension.remaining() }

        return nBits
    }

    override fun toByteBuffer(): ByteBuffer {
        val buffer = ByteBuffer.allocate(size)

        buffer.put(adaptationFieldLength)
        buffer.put(((discontinuityIndicator shl 7)
                or (randomAccessIndicator shl 6)
                or (elementaryStreamPriorityIndicator shl 5)
                or ((programClockReference?.let { 1 } ?: 0) shl 4)
                or ((originalProgramClockReference?.let { 1 } ?: 0) shl 3)
                or ((spliceCountdown?.let { 1 } ?: 0) shl 2)
                or ((transportPrivateData?.let { 1 } ?: 0) shl 1)
                or (adaptationFieldExtension?.let { 1 } ?: 0)
                ))

        programClockReference?.let {
            addClockReference(buffer, it)
        }
        originalProgramClockReference?.let {
            addClockReference(buffer, it)
        }
        spliceCountdown?.let {
            buffer.put(spliceCountdown)
            NotImplementedError("spliceCountdown not implemented yet")
        }
        transportPrivateData?.let {
            buffer.put(it.remaining().toByte())
            buffer.put(transportPrivateData)
            NotImplementedError("transportPrivateData not implemented yet")
        }
        adaptationFieldExtension?.let {
            NotImplementedError("adaptationFieldExtension not implemented yet")
        }

        buffer.rewind()
        return buffer
    }

    private fun addClockReference(buffer: ByteBuffer, timestamp: Long) {
        val pcrBase =
            (TSConst.SYSTEM_CLOCK_FREQ * timestamp / 1000000 /* µs -> s */ / 300) % 2.toDouble()
                .pow(33)
                .toLong()
        val pcrExt = (TSConst.SYSTEM_CLOCK_FREQ * timestamp / 1000000 /* µs -> s */) % 300

        /**
         * PCR Base -> 33 bits
         * Reserved -> 6 bits (0b111111)
         * PCR Ext -> 9 bits
         */
        buffer.putInt((pcrBase shr 1).toInt())
        buffer.putShort(
            (((pcrBase and 0x1) shl 15)
                    or (0b111111 shl 9)
                    or (pcrExt and 0x1FF))
        )
    }
}
