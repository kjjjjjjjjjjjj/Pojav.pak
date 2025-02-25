#ifndef POJAVLAUNCHER_LOG_H
#define POJAVLAUNCHER_LOG_H

#include <android/log.h>

#ifndef TAG
#define TAG "jrelog"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,    TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,    TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,    TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,    TAG, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif

