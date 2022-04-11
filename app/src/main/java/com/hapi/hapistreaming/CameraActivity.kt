package com.hapi.hapistreaming

import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.WindowManager
import android.widget.Button
import androidx.annotation.RequiresApi
import com.hapi.avcapture.*
import com.hapi.avrender.renderview.HapiGLSurfacePreview
import com.hapi.avcapture.screen.ScreenServicePlugin
import com.hapi.avpackerclinet.HapiAVPackerClient
import com.hapi.avparam.*
import com.hapi.avrender.renderview.HapiSLAudioRender
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

    //本地录制
    private val recordClient by lazy {
        HapiAVPackerClient()
    }

    //推流
    private val pusherClient by lazy {
        HapiAVPackerClient().apply {
            mMuxerConnectCallBack = object : MuxerConnectCallBack {
                //推流链接状态回调
                override fun onConnectedStatus(status: ConnectedStatus, msg: String?) {}
            }
        }
    }

    //摄像头轨道
    private val cameTrack by lazy {
        HapiTrackFactory.createCameraXTrack(this, this, 720, 1280).apply {
            frameCall = object : FrameCall<VideoFrame> {
                //帧回调
                override fun onFrame(frame: VideoFrame) {}
                //数据处理回调
                override fun onProcessFrame(frame: VideoFrame): VideoFrame {
                    return super.onProcessFrame(frame)
                }
            }
        }
    }

    //麦克轨道
    private val microphoneTrack by lazy {
        HapiTrackFactory.createMicrophoneTrack(
            this, DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT
        )
    }

    //屏幕采集轨道
    private val screenCaptureTrack by lazy {
        HapiTrackFactory.createScreenCaptureTrack(this)
    }

    //自定义轨道
    private val customAudioTrack by lazy {
        HapiTrackFactory.createCustomAudioTrack()
    }


    @RequiresApi(Build.VERSION_CODES.M)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_camera)
        lifecycle.addObserver(recordClient)
        lifecycle.addObserver(pusherClient)


        //如果需要预览视频轨道
        cameTrack.playerView = findViewById<HapiGLSurfacePreview>(R.id.preview)

        //如果需要耳返
        val mHapiSLAudioRender = HapiSLAudioRender()
        microphoneTrack.mAudioRender = mHapiSLAudioRender


        //开启相应的轨道
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
//
//        )
        // recordClient.attachTrack(screenCaptureTrack)

        //绑定轨道到推流器 或者 录制  一条轨道既可以推流也可以录制 比如 摄像头/麦克风 (720p)推流 +  摄像头/麦克风 (1080p)录制
        //再比如  摄像头/麦克风 推流 + 屏幕采集+麦克风 录制
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

                    //开始录制 传编码参数
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
                val url =
                    "srt://pili-publish.qnsdk.com:1935?streamid=#!::h=sdk-live/manjiale,m=publish,domain=pili-publish.qnsdk.com"
                //开始推流 传编码参数
                pusherClient.start(
                    url, EncodeParam(
                        videoEncodeParam,
                        audioEncodeParam
                    )
                )
            }
        }
    }

    override fun onResume() {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onResume()
    }

    override fun onPause() {
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
    }
}