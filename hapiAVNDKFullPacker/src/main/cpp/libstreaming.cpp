#include <jni.h>
#include <string>

#include <Muxer.h>

extern "C"
JNIEXPORT jlong
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1CreateContext(JNIEnv *env, jobject thiz) {

    auto *muxer = new Muxer();
    muxer->setJNIEnv(env, thiz);
    return reinterpret_cast<jlong>(muxer);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1DestroyContext(JNIEnv *env, jobject thiz,
                                                                         jlong handler) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    muxer->releaseJNIEnv();
    delete muxer;
    muxer = nullptr;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1Start(JNIEnv *env, jobject thiz,
                                                                jlong handler, jstring out_url,
                                                                jint frame_width, jint frame_height,
                                                                jint video_bit_rate, jint fps,
                                                                jint out_sample_fmt,
                                                                jint audio_channel_layout,
                                                                jint audio_sample_rate,
                                                                jint audio_bitrate) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    const char *url = env->GetStringUTFChars(out_url, nullptr);
    std::string str(url);
    EncodeParam encodeParam;
    //video
    encodeParam.frameWidth = frame_width;
    encodeParam.frameHeight = frame_height;
    encodeParam.videoBitRate = video_bit_rate;
    encodeParam.fps = fps;
    if (video_bit_rate == 0) {
        encodeParam.hasVideo = false;
    } else {
        encodeParam.hasVideo = true;
    }
    encodeParam.out_sample_fmt = AVSampleFormat(out_sample_fmt);
    encodeParam.audioChannelLayout = (audio_channel_layout);
    encodeParam.audioSampleRate = audio_sample_rate;
    encodeParam.audioBitrate = audio_bitrate;
    if (audio_bitrate == 0) {
        encodeParam.hasAudio = false;
    } else {
        encodeParam.hasAudio = true;
    }
    int ret = muxer->start(str, encodeParam);
    env->ReleaseStringUTFChars(out_url, url);
    return ret;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1OnAudioData(JNIEnv *env, jobject thiz,
                                                                      jlong handler,
                                                                      jbyteArray data,
                                                                      jint sample_fmt,
                                                                      jint audio_channel_layout,
                                                                      jint audio_sample_rate) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    jbyte *c_array = env->GetByteArrayElements(data, 0);
    int len_arr = env->GetArrayLength(data);
    AudioFrame audioFrame;
    audioFrame.dataSize = len_arr;
    audioFrame.data = reinterpret_cast<uint8_t *>(c_array);
    audioFrame.out_sample_fmt = AVSampleFormat(sample_fmt);
    audioFrame.audioSampleRate = audio_sample_rate;
    audioFrame.audioChannelLayout = (audio_channel_layout);
    muxer->onFrame2Encode(&audioFrame);
    audioFrame.data = nullptr;
    env->ReleaseByteArrayElements(data, c_array, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1OnVideoData(JNIEnv *env, jobject thiz,
                                                                      jlong handler, jint format,
                                                                      jbyteArray data, jint width,
                                                                      jint height,
                                                                      jint rotationDegrees,
                                                                      jint pixel_stride,
                                                                      jint row_padding) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    jbyte *c_array = env->GetByteArrayElements(data, 0);
    int len_arr = env->GetArrayLength(data);
    NativeImage videoFrame;
    videoFrame.dataSize = len_arr;
    videoFrame.data = reinterpret_cast<uint8_t *>(c_array);
    videoFrame.width = width;
    videoFrame.height = height;
    videoFrame.format = format;
    videoFrame.rotationDegrees = rotationDegrees;
    videoFrame.rowPadding = row_padding;
    videoFrame.pixelStride = pixel_stride;
    muxer->onFrame2Encode(&videoFrame);
    videoFrame.data = nullptr;
    env->ReleaseByteArrayElements(data, c_array, 0);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1Stop(JNIEnv *env, jobject thiz,
                                                               jlong handler) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    muxer->stop();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1Pause(JNIEnv *env, jobject thiz,
                                                                jlong handler) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    muxer->pause();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1Resume(JNIEnv *env, jobject thiz,
                                                                 jlong handler) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    muxer->resume();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1getMuxerStatus(JNIEnv *env, jobject thiz,
                                                                         jlong handler) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    return muxer->getMuxerStatus();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_hapi_avpacker_HapiDroidStreamContext_native_1getConnectStatus(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jlong handler) {
    auto *muxer = reinterpret_cast<Muxer *>(handler);
    return muxer->getConnectStatus();
}