package com.hapi.srtmuxer.ts.data

data class Stream(
    val mimeType: String,
    val pid: Short,
    val discontinuity: Boolean = false
)
