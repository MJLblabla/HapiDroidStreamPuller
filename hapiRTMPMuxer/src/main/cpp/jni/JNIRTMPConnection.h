//
// Created by 1 on 2022/4/7.
//

#ifndef HAPI_JNISRTCONNECTION_H
#define HAPI_JNISRTCONNECTION_H

#include "RTMPConnection.h"
#include "JVMCallBack.h"
#include <jni.h>

#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "onEventCallback"

class JNIRTMPConnection {

public:
    RTMPConnection rtmpConnection;
    JVMCallBack jvmCall;

    JNIRTMPConnection(JNIEnv *jniEnv, jobject obj) {
        jvmCall.setJNIEnv(jniEnv, obj);

        rtmpConnection.connectCallBack = [this](int status) {
            bool isAttach = false;
            JNIEnv *env = jvmCall.GetJNIEnv(&isAttach);
            if (env == nullptr)
                return;
            jobject javaObj = jvmCall.GetJavaObj();
            jmethodID mid = env->GetMethodID(env->GetObjectClass(javaObj),
                                             JAVA_PLAYER_EVENT_CALLBACK_API_NAME, "(I)V");

            if (mid) {
                env->CallVoidMethod(javaObj, mid, status);
            }
            if (isAttach)
                jvmCall.GetJavaVM()->DetachCurrentThread();
        };
    }

    ~JNIRTMPConnection() {
        jvmCall.releaseJNIEnv();
    }
};

#endif //HAPISRT_JNISRTCONNECTION_H
