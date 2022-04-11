
package com.hapi.srtmuxer.ts.packets

import android.util.Log
import com.hapi.srtmuxer.ts.IMuxerListener
import com.hapi.srtmuxer.ts.Packet
import com.hapi.srtmuxer.ts.utils.MuxerConst
import com.hapi.srtmuxer.ts.utils.TSOutputCallback
import com.hapi.srtmuxer.ts.utils.toInt

import java.nio.ByteBuffer
import java.security.InvalidParameterException

open class TS(
    muxerListener: IMuxerListener,
    val pid: Short,
    private val transportErrorIndicator: Boolean = false,
    private val transportPriority: Boolean = false,
    private val transportScramblingControl: Byte = 0, // Not scrambled
    private var continuityCounter: Byte = 0,
) : TSOutputCallback(muxerListener) {

    companion object {
        const val SYNC_BYTE: Byte = 0x47
        const val PACKET_SIZE = 188
    }

    protected fun write(
        payload: ByteBuffer? = null,
        adaptationField: ByteBuffer? = null,
        specificHeader: ByteBuffer? = null,
        stuffingForLastPacket: Boolean = false,
        timestamp: Long = 0L
    ) {
        val payloadLimit = payload?.limit() ?: 0
        var payloadUnitStartIndicator = true

        var adaptationFieldIndicator = adaptationField != null
        val payloadIndicator = payload != null

        var packetIndicator = 0

        val buffer = ByteBuffer.allocateDirect(PACKET_SIZE * MuxerConst.MAX_OUTPUT_PACKET_NUMBER)

        Log.d("specificHeader","新包payload?.hasRemaining() == true || adaptationFieldIndicator")
        while (payload?.hasRemaining() == true || adaptationFieldIndicator) {

            buffer.limit(buffer.position() + PACKET_SIZE)

            // Write header to packet
            buffer.put(SYNC_BYTE)
            var byte =
                (transportErrorIndicator.toInt() shl 7) or (payloadUnitStartIndicator.toInt() shl 6) or (transportPriority.toInt() shl 5) or (pid.toInt() shr 8)

            buffer.put(byte.toByte())
            payloadUnitStartIndicator = false
            buffer.put(pid.toByte())


            byte =
                (transportScramblingControl.toInt() shl 6) or (continuityCounter.toInt() and 0xF) or
                        when {
                            adaptationFieldIndicator and payloadIndicator -> {
                                (0b11 shl 4)
                            }
                            adaptationFieldIndicator -> {
                                (0b10 shl 4)
                            }
                            payloadIndicator -> {
                                (0b01 shl 4)
                            }
                            else -> throw InvalidParameterException("TS must have either a payload either an adaption field")
                        }
            buffer.put(byte.toByte())

            continuityCounter = ((continuityCounter + 1) and 0xF).toByte()

            // Add adaptation fields first if needed
            if (adaptationFieldIndicator) {
                buffer.put(adaptationField!!) // Is not null if adaptationFieldIndicator is true
                adaptationFieldIndicator = false
            }

            // Then specific stream header. Mainly for PES header.
            Log.d("specificHeader","before specificHeader${buffer.position()}")
            specificHeader?.let {
                buffer.put(it)
                Log.d("specificHeader","addspecificHeader ${buffer.position()}")
            }

            // Fill packet with correct size of payload
            payload?.let {
                if (stuffingForLastPacket) {
                    // Add stuffing before last packet remaining payload
                    if (buffer.remaining() > it.remaining()) {

                        //ts头 + specificHeader头 取余数
                        val headerSize = (buffer.position()) % PACKET_SIZE

                        //当前第几个包
                        val currentPacketFirstPosition =
                            buffer.position() / PACKET_SIZE * PACKET_SIZE


                        //第四个字节
                        byte = buffer[currentPacketFirstPosition + 3].toInt()
                        byte = byte or (1 shl 5)

                        //移动到第四个字节
                        buffer.position(currentPacketFirstPosition + 3)
                        buffer.put(byte.toByte())

                        //移动到第五个字节
                        buffer.position(currentPacketFirstPosition + 4)

                        //188 =负载剩余 + 头部已经占有 -1
                        val stuffingLength = PACKET_SIZE - it.remaining() - headerSize - 1
                        // 一位放填充多长
                        buffer.put(stuffingLength.toByte())


                        //填充
                        if (stuffingLength >= 1) {
                            buffer.put(0.toByte())
                            for (i in 0..stuffingLength - 2) {
                                buffer.put(0xFF.toByte()) // Stuffing
                            }
                        }
                    }
                }

                // 荷载 最后 = 荷载当前位置 + buffer剩余（确保 < 荷载剩余） （5，4）->4  (5,24)
                it.limit(it.position() + buffer.remaining().coerceAtMost(it.remaining()))
                //直接放es荷载 那specificHeader？？
                buffer.put(it)
                //荷载最后 = 恢复最后
                it.limit(payloadLimit)
            }

            while (buffer.hasRemaining()) {
                buffer.put(0xFF.toByte())
            }

            val isLastPacket = payload?.let { !it.hasRemaining() } ?: true
            if (buffer.limit() == buffer.capacity() || isLastPacket) {
              Log.d("writePacket","发送之前 ${buffer.position()} ${buffer.limit()}  ${buffer.capacity()}")
                buffer.flip()
                Log.d("writePacket","发送之前flip ${buffer.position()} ${buffer.limit()}  ${buffer.capacity()}")
                writePacket(
                    Packet(
                        buffer,
                        packetIndicator == 0,
                        isLastPacket,
                        timestamp,
                        pid
                    )
                )
                buffer.flip()
                Log.d("writePacket","发送之后flip ${buffer.position()} ${buffer.limit()}  ${buffer.capacity()}")
                buffer.rewind()
                packetIndicator++
            }
        }
    }
}