#ifndef QV2LOG_H
#define QV2LOG_H

#include <cstdio>
#include <ctime>
#include <unistd.h>

#ifdef __APPLE__
#include <pthread.h>
#elif defined(__linux__)
#include <sys/syscall.h>
#endif

#define QV2_LOG_TAG_DEFAULT "Qv2Codec"

#ifndef LOG_TAG
#define LOG_TAG QV2_LOG_TAG_DEFAULT
#endif

inline const char* get_timestamp() {
    static char buf[32];
    time_t now = time(nullptr);
    strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", localtime(&now));
    return buf;
}

inline long get_current_tid() {
#ifdef __APPLE__
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return static_cast<long>(tid);
#elif defined(__linux__)
    return static_cast<long>(syscall(SYS_gettid));
#else
    return 0;
#endif
}

#define QV2_LOG(level, tag, fmt, ...) \
    printf("[%s] %5d %5ld %s    %s: " fmt "\n", \
           get_timestamp(), (int)getpid(), get_current_tid(), \
           level, tag, ##__VA_ARGS__)

#if LOG_DEBUG
#define QV2_LOGD(fmt, ...) QV2_LOG("D", LOG_TAG, fmt, ##__VA_ARGS__)
#define QV2_LOGV(fmt, ...) QV2_LOG("V", LOG_TAG, fmt, ##__VA_ARGS__)
#else
#define QV2_LOGD(fmt, ...) ((void)0)
#define QV2_LOGV(fmt, ...) ((void)0)
#endif

#define QV2_LOGI(fmt, ...) QV2_LOG("I", LOG_TAG, fmt, ##__VA_ARGS__)
#define QV2_LOGW(fmt, ...) QV2_LOG("W", LOG_TAG, fmt, ##__VA_ARGS__)
#define QV2_LOGE(fmt, ...) QV2_LOG("E", LOG_TAG, fmt, ##__VA_ARGS__)

#endif // QV2LOG_H
