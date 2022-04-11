package com.hapi.srtmuxer.ts

import java.nio.ByteBuffer

/**
 * Packet internal representation.
 * A [Frame] is composed by multiple packets.
 */
data class Packet(
    /**
     * Contains data.
     */
    var buffer: ByteBuffer,
    /**
     * [Boolean.true] if this is the first packet that describes a frame.
     */
    var isFirstPacketFrame: Boolean,
    /**
     * [Boolean.true] if this is the last packet that describes a frame.
     */
    var isLastPacketFrame: Boolean,
    /**
     * Frame timestamp in µs.
     */
    var ts: Long, // in µs
    val pid: Short
)