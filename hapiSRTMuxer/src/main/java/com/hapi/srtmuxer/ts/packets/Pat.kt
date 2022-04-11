
package com.hapi.srtmuxer.ts.packets


import com.hapi.srtmuxer.ts.IMuxerListener
import com.hapi.srtmuxer.ts.data.ITSElement
import com.hapi.srtmuxer.ts.data.Service
import java.nio.ByteBuffer
import kotlin.experimental.or

class Pat(
    muxerListener: IMuxerListener,
    private val services: List<Service>,
    tsId: Short,
    versionNumber: Byte = 0,
    var packetCount: Int = 0,
) : Psi(
    muxerListener,
    PID,
    TID,
    true,
    false,
    tsId,
    versionNumber,
), ITSElement {
    companion object {
        // Table ids
        const val TID: Byte = 0x00

        // Pids
        const val PID: Short = 0x0000
    }

    override val bitSize: Int
        get() = 32 * services.filter { it.pmt != null }.size
    override val size: Int
        get() = bitSize / Byte.SIZE_BITS

    fun write() {
        if (services.any { it.pmt != null }) {
            write(toByteBuffer())
        }
    }

    override fun toByteBuffer(): ByteBuffer {
        val buffer = ByteBuffer.allocate(size)

        services
            .filter { it.pmt != null }
            .forEach {
                buffer.putShort(it.info.id)
                buffer.putShort(
                    (0b111 shl 13).toShort()  // reserved
                            or it.pmt!!.pid
                )
            }

        buffer.rewind()
        return buffer
    }
}