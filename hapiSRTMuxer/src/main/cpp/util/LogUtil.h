
#ifndef HAPI_LOGUTIL_H
#define HAPI_LOGUTIL_H

#include<android/log.h>
#include <jni.h>
#include <ctime>

#define  LOG_TAG "HAPISTRT"
#define LOG_ABLE  1
#define  LOGCATE(...) {  if(LOG_ABLE){ __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__); }}
#define  LOGCATV(...) {  if(LOG_ABLE){ __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__);}}
#define  LOGCATD(...) { if(LOG_ABLE){ __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__);}}
#define  LOGCATI(...) { if(LOG_ABLE){ __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__);}}


#endif //HAPI_LOGUTIL_H
