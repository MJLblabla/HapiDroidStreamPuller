package com.hapi.srtmuxer.ts.utils

import com.hapi.srtmuxer.ts.IMuxerListener
import com.hapi.srtmuxer.ts.Packet


open class TSOutputCallback(private val muxerListener: IMuxerListener) {
    protected fun writePacket(packet: Packet) {
        return muxerListener.onOutputFrame(packet)
    }
}