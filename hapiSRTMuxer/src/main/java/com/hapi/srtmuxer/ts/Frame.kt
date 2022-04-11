package com.hapi.srtmuxer.ts

import java.nio.ByteBuffer

/**
 * Frame internal representation.
 */
data class Frame(
    /**
     * Contains an audio or video frame data.
     */
    var buffer: ByteBuffer,

    /**
     * Frame mime type
     */
    val mimeType: String,

    /**
     * Presentation timestamp in µs
     */
    var pts: Long,

    /**
     * Decoded timestamp in µs (not used).
     */
    var dts: Long? = null,

    /**
     * [Boolean.true] if frame is a key frame (I-frame for AVC/HEVC and audio frames)
     */
    val isKeyFrame: Boolean = false,

    /**
     * Contains extra frame description.
     * For AAC, it contains ADTS.
     */
    val extra: ByteBuffer? = null
)