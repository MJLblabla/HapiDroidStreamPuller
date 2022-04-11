
package com.hapi.srtmuxer.ts.packets

import com.hapi.srtmuxer.ts.data.ITSElement
import com.hapi.srtmuxer.ts.utils.*
import java.nio.ByteBuffer

class TableHeader(
    private val tableId: Byte,
    private val sectionSyntaxIndicator: Boolean,
    private val reservedFutureUse: Boolean = false,
    payloadLength: Short,
    private val tableIdExtension: Short = 0,
    private val versionNumber: Byte,
    private val currentNextIndicator: Boolean = true,
    private val sectionNumber: Byte,
    private val lastSectionNumber: Byte,
) : ITSElement {
    override val bitSize = 64
    override val size = bitSize / Byte.SIZE_BITS

    private val sectionLength = payloadLength + 5 + Psi.CRC_SIZE // 5 - header

    override fun toByteBuffer(): ByteBuffer {
        val buffer = ByteBuffer.allocate(size)

        buffer.put(tableId)
        buffer.put(
            (sectionSyntaxIndicator shl 7)
                    or (reservedFutureUse shl 6)
                    or (0b11 shl 4)
                    // or (0b00 shl 2)
                    or ((sectionLength shr 8) and 0x3)
        )
        buffer.put(sectionLength)
        buffer.putShort(tableIdExtension)
        buffer.put(
            (0b11 shl 6)
                    or (versionNumber shl 1)
                    or (currentNextIndicator.toInt())
        )
        buffer.put(sectionNumber)
        buffer.put(lastSectionNumber)

        buffer.rewind()
        return buffer
    }
}

