#include <jni.h>
#include <string>
#include "AVResample.h"
#include <jni.h>



extern "C"
JNIEXPORT jlong
Java_com_hapi_avencoder_AVResampleContext_native_1CreateContext(JNIEnv *env, jobject thiz) {

    auto mAVResample = new AVResample();
    return reinterpret_cast<jlong>(mAVResample);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avencoder_AVResampleContext_native_1DestroyContext(JNIEnv *env, jobject thiz,
                                                                 jlong handler) {
    auto *avResample = reinterpret_cast<AVResample *>(handler);
    delete avResample;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avencoder_AVResampleContext_native_1Start(JNIEnv *env, jobject thiz, jlong handler,
                                                        jint frame_width, jint frame_height,
                                                        jint video_bit_rate, jint fps,
                                                        jint out_sample_fmt,
                                                        jint audio_channel_layout,
                                                        jint audio_sample_rate,
                                                        jint audio_bitrate) {

    auto *avResample = reinterpret_cast<AVResample *>(handler);
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
    avResample->start(encodeParam);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avencoder_AVResampleContext_native_1OnAudioData(JNIEnv *env, jobject thiz,
                                                              jlong handler, jbyteArray data,
                                                              jint sample_fmt,
                                                              jint audio_channel_layout,
                                                              jint audio_sample_rate,
                                                              jbyteArray out_frame) {


    auto *avResample = reinterpret_cast<AVResample *>(handler);

    jbyte *c_array = env->GetByteArrayElements(data, 0);
    int len_arr = env->GetArrayLength(data);

    AudioFrame audioFrame;
    audioFrame.dataSize = len_arr;
    audioFrame.data = reinterpret_cast<uint8_t *>(c_array);
    audioFrame.out_sample_fmt = AVSampleFormat(sample_fmt);
    audioFrame.audioSampleRate = audio_sample_rate;
    audioFrame.audioChannelLayout = (audio_channel_layout);

    jbyte *c_array_out = env->GetByteArrayElements(out_frame, 0);
    int len_arr_out = env->GetArrayLength(out_frame);

    AudioFrame outFrame;
    avResample->onFrame2Encode(&audioFrame,&outFrame);
    memcpy(c_array_out,outFrame.data,outFrame.dataSize);

    audioFrame.data = nullptr;
    env->ReleaseByteArrayElements(data, c_array, 0);

    env->ReleaseByteArrayElements(out_frame, c_array_out, 0);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_hapi_avencoder_AVResampleContext_native_1OnVideoData(JNIEnv *env, jobject thiz,
                                                              jlong handler, jint format,
                                                              jbyteArray data, jint width,
                                                              jint height, jint rotation_degrees,
                                                              jint pixel_stride, jint row_padding,
                                                              jbyteArray out_frame) {

    auto *avResample = reinterpret_cast<AVResample *>(handler);
    jbyte *c_array = env->GetByteArrayElements(data, 0);
    int len_arr = env->GetArrayLength(data);
    NativeImage videoFrame;
    videoFrame.dataSize = len_arr;
    videoFrame.data = reinterpret_cast<uint8_t *>(c_array);
    videoFrame.width = width;
    videoFrame.height = height;
    videoFrame.format = format;
    videoFrame.rotationDegrees = rotation_degrees;
    videoFrame.rowPadding = row_padding;
    videoFrame.pixelStride = pixel_stride;



    jbyte *c_array_out = env->GetByteArrayElements(out_frame, 0);
    int len_arr_out = env->GetArrayLength(out_frame);
    NativeImage outFrame;
    avResample->onFrame2Encode(&videoFrame,&outFrame);

    memcpy(c_array_out,outFrame.data,outFrame.dataSize);

    videoFrame.data = nullptr;
    env->ReleaseByteArrayElements(data, c_array, 0);
    env->ReleaseByteArrayElements(out_frame, c_array_out, 0);
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_hapi_avencoder_AVResampleContext_native_1OnAudioDataNewByte(JNIEnv *env, jobject thiz,
                                                                     jlong handler, jbyteArray data,
                                                                     jint sample_fmt,
                                                                     jint audio_channel_layout,
                                                                     jint audio_sample_rate) {
    auto *avResample = reinterpret_cast<AVResample *>(handler);

    jbyte *c_array = env->GetByteArrayElements(data, 0);
    int len_arr = env->GetArrayLength(data);

    AudioFrame audioFrame;
    audioFrame.dataSize = len_arr;
    audioFrame.data = reinterpret_cast<uint8_t *>(c_array);
    audioFrame.out_sample_fmt = AVSampleFormat(sample_fmt);
    audioFrame.audioSampleRate = audio_sample_rate;
    audioFrame.audioChannelLayout = (audio_channel_layout);

    AudioFrame outFrame;
    avResample->onFrame2Encode(&audioFrame,&outFrame);


    jbyteArray jarrRV =env->NewByteArray(outFrame.dataSize);

    jbyte gs_raw_data[outFrame.dataSize];
    memcpy(gs_raw_data,outFrame.data,outFrame.dataSize);
    env->SetByteArrayRegion(jarrRV, 0,outFrame.dataSize,gs_raw_data);

    audioFrame.data = nullptr;
    env->ReleaseByteArrayElements(data, c_array, 0);

    return jarrRV;

}