package com.hapi.librtcclient

class NativeLib {

    /**
     * A native method that is implemented by the 'librtcclient' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'librtcclient' library on application startup.
        init {
            System.loadLibrary("librtcclient")
        }
    }
}