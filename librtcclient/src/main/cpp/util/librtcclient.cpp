#include <jni.h>
#include <string>
#include <mutex>
#include<android/log.h>
#include <ctime>
#include "LogUtil.h"
#include "B.h"
extern "C" JNIEXPORT jstring JNICALL
Java_com_hapi_librtcclient_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    struct timespec now;
      clock_gettime(CLOCK_MONOTONIC, &now);
    GetSysCurrentTimeNS();
    auto b = new B();
    b->f1(1);
    return env->NewStringUTF(hello.c_str());
}

