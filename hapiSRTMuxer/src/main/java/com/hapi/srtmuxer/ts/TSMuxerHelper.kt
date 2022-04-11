package com.hapi.srtmuxer.ts

import android.media.MediaFormat

object TSMuxerHelper {
    object Video {
        /**
         * Get TS Muxer supported video encoders list
         */
        val supportedEncoders =
            listOf(
                MediaFormat.MIMETYPE_VIDEO_AVC,
                MediaFormat.MIMETYPE_VIDEO_HEVC
            )
    }

    object Audio {
        /**
         * Get TS Muxer supported audio encoders list
         */
        val supportedEncoders = listOf(MediaFormat.MIMETYPE_AUDIO_AAC)
    }
}