package com.hapi.srtmuxer

import android.util.Log
import com.hapi.srtmuxer.mode.SRTConfig
import java.nio.ByteBuffer

class SRTConnection {

    private external fun stringFromJNI(): String

    companion object {
        // Used to load the 'srtmuxer' library on application startup.
        init {
            System.loadLibrary("srtmuxer")
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

    fun config(config: SRTConfig) {
        if(mNativeContextHandler==-1L){
            return
        }
        native_config(
            mNativeContextHandler,
            config.streamId,
            config.ipAddress,
            config.port,
            config.payloadSize,
            config.maxBW,
            config.inputBW
        )
    }

    fun connect() {
        if(mNativeContextHandler==-1L){
            return
        }
        native_connect(mNativeContextHandler)
    }

    fun send(
        msg: ByteBuffer,
        ttl: Int,
        srcTime: Long,
        boundary: Int
    ) {
        if(mNativeContextHandler==-1L){
            return
        }
        Log.d("SRTConnection", "send ${boundary} ${msg.position()}  ${msg.limit()}")
        native_send(mNativeContextHandler, msg, msg.position(),msg.remaining(), ttl, srcTime, boundary)
    }

    fun getStats(): Stats? {
        if(mNativeContextHandler==-1L){
            return null
        }
        return native_getStats(mNativeContextHandler)
    }

    fun close() {
        if(mNativeContextHandler==-1L){
            return
        }
        native_close(mNativeContextHandler)
    }

    private external fun native_init(): Long
    private external fun native_uninit(handler: Long)
    private external fun native_config(
        handler: Long,
        streamId: String,
        ipAddress: String,
        port: Int,
        payloadSize: Int,
        maxBW: Int,
        inputBW: Int,
    )

    private external fun native_getStats(handler: Long): Stats
    private external fun native_connect(handler: Long)
    private external fun native_send(
        handler: Long, msg: ByteBuffer,position:Int, offset: Int,
        ttl: Int,
        srcTime: Long,
        boundary: Int
    )

    private external fun native_close(handler: Long)

}