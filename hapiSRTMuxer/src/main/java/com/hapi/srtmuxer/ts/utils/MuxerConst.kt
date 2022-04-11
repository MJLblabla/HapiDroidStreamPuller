package com.hapi.srtmuxer.ts.utils

object MuxerConst {
    const val PAT_PACKET_PERIOD = 40
    const val SDT_PACKET_PERIOD = 200

    /**
     * Number of MPEG-TS packet stream in output [Packet] returns by [IMuxerListener.onOutputFrame]
     */
    const val MAX_OUTPUT_PACKET_NUMBER = 7
}