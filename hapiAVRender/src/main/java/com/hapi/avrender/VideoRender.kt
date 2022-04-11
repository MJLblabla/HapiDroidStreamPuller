package com.hapi.avrender

import com.hapi.avparam.VideoFrame


interface VideoRender {
    fun onFrame(frame: VideoFrame)
}