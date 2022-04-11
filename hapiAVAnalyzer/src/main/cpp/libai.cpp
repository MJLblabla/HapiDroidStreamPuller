#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_hapi_libai_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_hapi_hapiplay_OpenGLRender_native_1create(JNIEnv *env, jobject thiz) {
    // TODO: implement native_create()
}