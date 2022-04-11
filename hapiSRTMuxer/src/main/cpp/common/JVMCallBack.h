//
// Created by 1 on 2022/4/7.
//

#ifndef HAPI_JVMCALLBACK_H
#define HAPI_JVMCALLBACK_H


#include <jni.h>

class JVMCallBack {

private:

public:
    JavaVM *m_JavaVM = nullptr;
    jobject m_JavaObj = nullptr;
    JNIEnv *GetJNIEnv(bool *isAttach);
    jobject GetJavaObj();
    JavaVM *GetJavaVM();

    JVMCallBack();
    ~JVMCallBack();
    void setJNIEnv(JNIEnv *jniEnv, jobject obj);
    void releaseJNIEnv();
};


#endif //HAPISRT_JVMCALLBACK_H
