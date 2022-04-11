package com.hapi.rtmpmuxer

import android.util.Log
import java.nio.ByteBuffer

class RTMPConnection {

    enum class AVFrameType(val intV: Int) {
        TYPE_VIDEO(0),
        TYPE_AUDIO(1)
    };
    /**
     * A native method that is implemented by the 'rtmpmuxer' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'rtmpmuxer' library on application startup.
        init {
            System.loadLibrary("rtmpmuxer")
        }
    }


    var mConnectEventCallback: (code: Int) -> Unit = {}

    private var mNativeContextHandler: Long = -1L

    private fun onEventCallback(code: Int) {
        Log.d("SRTConnection", "onEventCallback ${code}")
        mConnectEventCallback(code)
    }

    fun init() {
        mNativeContextHandler = native_init();
    }

    fun unInit() {
        native_uninit(mNativeContextHandler)
        mNativeContextHandler = -1;
    }

    fun setUrl(url: String) {
        if (mNativeContextHandler == -1L) {
            return
        }
        native_setUrl(
            mNativeContextHandler,
            url
        )
    }

    fun connect() {
        if (mNativeContextHandler == -1L) {
            return
        }
        native_connect(mNativeContextHandler)
    }

    fun sendFrame(
        byteBuffer: ByteBuffer,
        mediaType: AVFrameType,

        spsLen: Int = 0,
        ppsLen: Int = 0
    ) {
        if (mNativeContextHandler == -1L) {
            return
        }
        if(byteBuffer.limit()<4){
            return
        }

        native_sendFrame(
            mNativeContextHandler,
            byteBuffer,
            byteBuffer.position(),
            byteBuffer.limit(),
            mediaType.intV,

            spsLen,
            ppsLen
        )
    }


    fun close() {
        if (mNativeContextHandler == -1L) {
            return
        }
        native_close(mNativeContextHandler)
    }

    private external fun native_init(): Long
    private external fun native_uninit(handler: Long)
    private external fun native_setUrl(
        handler: Long,
        url: String
    )

    private external fun native_connect(handler: Long)

    private external fun native_sendFrame(
        handler: Long,
        msg: ByteBuffer,
        offset: Int,
        limit: Int,
        mediaType: Int,

        spsLen: Int,
        ppsLen: Int
    )

    private external fun native_close(handler: Long)
}