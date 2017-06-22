//
// Created by liuruikai756 on 21/06/2017.
//
#include "jni.h"
#include <android/log.h>
#include <dlfcn.h>
#include <stddef.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#define LOG_TAG "VirtualHosts"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef void (*MSHookType)(void *symbol, void *replace, void **result);

typedef struct {
    const char *hostname;
    const char *addr;
} Host;

static Host hosts [] = {
        {"www.google.com", "216.58.200.33"}
};

static int hostsLen = sizeof(hosts)/sizeof(Host);

static void *findSymbol(const char *path, const char *symbol) {
    void *handle = dlopen(path, RTLD_LAZY);
    if(!handle) {
        LOGE("handle %s is null", path);
        return NULL;
    }

    void *target = dlsym(handle, symbol);
    if(!target) {
        LOGE("symbol %s is null", symbol);
    }
    return target;
}

static int (*old_getaddrinfo) (const char *hostname, const char *servname,
                               const struct addrinfo *hints, void *context, struct addrinfo **res);

static int new_getaddrinfo(const char *hostname, const char *servname,
                           const struct addrinfo *hints, void *context, struct addrinfo **res) {
//    LOGE("android_getaddrinfofornetcontext for hostname %s", hostname);
    // hostname must be a numerical network address if AI_NUMERICHOST
    if(!(hints->ai_flags & AI_NUMERICHOST)) {
        for(int i=0; i<hostsLen; i++) {
            Host *host = &hosts[i];
            if(!strcmp(hostname, host->hostname)) { // hostname matched
                struct addrinfo *v_addrinfo = (struct addrinfo *)malloc(sizeof(struct addrinfo)+sizeof(struct sockaddr_in));
                memset(v_addrinfo, 0, sizeof(struct addrinfo)+sizeof(struct sockaddr_in));
                v_addrinfo->ai_family = AF_INET; //only support ipv4
                v_addrinfo->ai_addrlen = sizeof(struct sockaddr_in);
                v_addrinfo->ai_addr = (struct sockaddr *)(v_addrinfo+1);
                struct sockaddr_in sa;
                inet_pton(AF_INET, host->addr, &(sa.sin_addr));
                sa.sin_family = AF_INET;
                memcpy(v_addrinfo+1, &sa, sizeof(struct sockaddr_in));

                *res = v_addrinfo;
                return 0;
            }
        }
    }
    return old_getaddrinfo(hostname, servname, hints, context, res);
}


static void doHook() {
    MSHookType hookFunc;
    // Cydia::MSHookFunction
    hookFunc = findSymbol("libva-native.so", "_ZN5Cydia14MSHookFunctionEPvS0_PS0_");
    void *target = findSymbol("libc.so", "android_getaddrinfofornetcontext");
    if(!hookFunc || !target) {
        LOGE("Failed: MSHookFunction %p, android_getaddrinfofornetcontext %p",
             hookFunc, target);
        return;
    }

    hookFunc(target, (void *)&new_getaddrinfo, (void **)&old_getaddrinfo);
}


jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    doHook();

    LOGI("native hook done");
    return JNI_VERSION_1_6;
}
