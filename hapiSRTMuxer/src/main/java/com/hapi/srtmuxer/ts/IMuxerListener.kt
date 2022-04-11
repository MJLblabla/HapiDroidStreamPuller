package com.hapi.srtmuxer.ts



interface IMuxerListener {
    fun onOutputFrame(packet: Packet)
}