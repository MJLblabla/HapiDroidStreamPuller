package com.hapi.srtmuxer.ts.data

import java.nio.ByteBuffer

interface ITSElement {
    val size: Int
    val bitSize: Int

    fun toByteBuffer(): ByteBuffer
}