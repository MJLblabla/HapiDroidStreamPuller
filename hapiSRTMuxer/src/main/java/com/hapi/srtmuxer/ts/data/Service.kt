package com.hapi.srtmuxer.ts.data

import com.hapi.srtmuxer.ts.packets.Pmt


class Service(
    val info: ServiceInfo,
    var pmt: Pmt? = null,
    var streams: MutableList<Stream> = mutableListOf(),
    var pcrPid: Short? = null
)