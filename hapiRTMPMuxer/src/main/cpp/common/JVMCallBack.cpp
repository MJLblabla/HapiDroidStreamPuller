//
// Created by 1 on 2022/4/7.
//

#include "JVMCallBack.h"
JVMCallBack::JVMCallBack(){}
JVMCallBack::~JVMCallBack(){}

void JVMCallBack::setJNIEnv(JNIEnv *jniEnv, jobject obj) {
    jniEnv->GetJavaVM(&m_JavaVM);
    m_JavaObj = jniEnv->NewGlobalRef(obj);
}

void JVMCallBack::releaseJNIEnv() {
    bool isAttach = false;
    JNIEnv *env = GetJNIEnv(&isAttach);
    env->DeleteGlobalRef(m_JavaObj);
    if (isAttach)
        GetJavaVM()->DetachCurrentThread();
}

JNIEnv *JVMCallBack::GetJNIEnv(bool *isAttach) {
    JNIEnv *env;
    int status;
    if (nullptr == m_JavaVM) {
        return nullptr;
    }
    *isAttach = false;
    status = m_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status != JNI_OK) {
        status = m_JavaVM->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            return nullptr;
        }
        *isAttach = true;
    }
    return env;
}

jobject JVMCallBack::GetJavaObj() {
    return m_JavaObj;
}

JavaVM *JVMCallBack::GetJavaVM() {
    return m_JavaVM;
}
