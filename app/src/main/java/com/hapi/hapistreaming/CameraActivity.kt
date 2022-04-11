package com.hapi.hapistreaming

import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.widget.Button
import androidx.annotation.RequiresApi
import com.hapi.avparam.AudioEncodeParam
import com.hapi.avparam.EncodeParam
import com.hapi.avparam.VideoEncodeParam
import com.hapi.avrender.renderview.HapiGLSurfacePreview
import com.hapi.avcapture.DEFAULT_CHANNEL_LAYOUT
import com.hapi.avcapture.DEFAULT_SAMPLE_FORMAT
import com.hapi.avcapture.DEFAULT_SAMPLE_RATE
import com.hapi.avcapture.HapiTrackFactory
import com.hapi.avpackerclinet.HapiAVPackerClient
import java.io.File

class CameraActivity : AppCompatActivity() {
    var recordUrl = ""
    val videoEncodeParam = VideoEncodeParam(
        480,
        640,
        1000 * 1000,
        25,
        800 * 1000,
        (1.5 * 1000 * 1000).toInt()
    )
    val audioEncodeParam = AudioEncodeParam(
        DEFAULT_SAMPLE_RATE,
        DEFAULT_CHANNEL_LAYOUT,
        DEFAULT_SAMPLE_FORMAT,
        96000
    )
    private val recordClient by lazy { HapiAVPackerClient() }
    private val pusherClient by lazy { HapiAVPackerClient() }

    private val cameTrack by lazy {
        HapiTrackFactory.createCameraXTrack(this, this, 720, 1280)
    }

    private val microphoneTrack by lazy {
        HapiTrackFactory.createMicrophoneTrack(this)
    }

    private val screenCaptureTrack by lazy {
        HapiTrackFactory.createScreenCaptureTrack(this)
    }

    @RequiresApi(Build.VERSION_CODES.M)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_camera)
        lifecycle.addObserver(recordClient)
        lifecycle.addObserver(pusherClient)

        cameTrack.playerView = findViewById<HapiGLSurfacePreview>(R.id.preview)
        // microphoneTrack.isEarsBackAble = true

        cameTrack.start()
        microphoneTrack.start()

//        screenCaptureTrack.start(this, object : ScreenServicePlugin.ScreenCaptureServiceCallBack {
//            override fun onStart() {
//                Log.d("screenCaptureTrack", "onStart")
//            }
//
//            override fun onError(code: Int, msg: String?) {
//                Log.d("screenCaptureTrack", "onError")
//            }
//        }
//        )
        // recordClient.attachTrack(screenCaptureTrack)

        recordClient.attachTrack(cameTrack)
        recordClient.attachTrack(microphoneTrack)

        pusherClient.attachTrack(cameTrack)
        pusherClient.attachTrack(microphoneTrack)

        findViewById<Button>(R.id.btRecord)
            .setOnClickListener {

                if (it.isSelected) {
                    it.isSelected = false
                    (it as Button).text = "录制"
                    recordClient.stop()

                    MediaStoreUtils.insertVideoToMediaStore(this, recordUrl)
                } else {
                    it.isSelected = true
                    (it as Button).text = "结束"

                    recordUrl = if (Build.VERSION.SDK_INT > 29) {
                        cacheDir.absolutePath
                    } else {
                        Environment.getExternalStorageDirectory().getAbsolutePath()
                            .toString() + "/A"
                    } + "/${System.currentTimeMillis()}.mp4"

                    val file = File(recordUrl)
                    if (!file.exists()) {
                        file.createNewFile()
                    }
                    recordClient.start(
                        recordUrl, EncodeParam(
                            videoEncodeParam,
                            audioEncodeParam
                        )
                    )
                }
            }

        findViewById<Button>(R.id.btPush).setOnClickListener {
            if (it.isSelected) {
                it.isSelected = false
                (it as Button).text = "推流"
                pusherClient.stop()
            } else {
                it.isSelected = true
                (it as Button).text = "结束"
              // val url = "rtmp://pili-publish.qnsdk.com/sdk-live/manjiale"
                val url = "srt://pili-publish.qnsdk.com:1935?streamid=#!::h=sdk-live/manjiale,m=publish,domain=pili-publish.qnsdk.com"
                pusherClient.start(
                    url, EncodeParam(
                        videoEncodeParam,
                        audioEncodeParam
                    )
                )
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
    }
}