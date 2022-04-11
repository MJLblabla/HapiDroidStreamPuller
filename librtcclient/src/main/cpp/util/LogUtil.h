
#ifndef HAPI_LOGUTIL_H
#define HAPI_LOGUTIL_H

#include<android/log.h>

#include <jni.h>


#define  LOG_TAG "qiniurecorder"
#define LOG_ABLE  1
#define  LOGCATE(...) {  if(LOG_ABLE){ __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__); }}
#define  LOGCATV(...) {  if(LOG_ABLE){ __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__);}}
#define  LOGCATD(...) { if(LOG_ABLE){ __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__);}}
#define  LOGCATI(...) { if(LOG_ABLE){ __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__);}}


static long long GetSysCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long curTime = ((long long) (time.tv_sec)) * 1000 + time.tv_usec / 1000;
    return curTime;
}

static  long GetSysCurrentTimeNS() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000LL + now.tv_nsec;
}

#endif //HAPI_LOGUTIL_H
