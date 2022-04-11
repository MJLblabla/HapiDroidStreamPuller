package com.hapi.libai

class NativeLib {

    /**
     * A native method that is implemented by the 'libai' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'libai' library on application startup.
        init {
            System.loadLibrary("libai")
        }
    }
}