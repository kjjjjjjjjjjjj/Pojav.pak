//
// Created by maks on 15.01.2025.
//
#include "native_hooks.h"

#include <jni.h>
#include <stdbool.h>
#include <bytehook.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "stdio_is.h"

#define TAG __FILE_NAME__
#include <log.h>

static _Atomic bool exit_tripped = false;

static int exit_code = 0;

typedef void (*exit_func)(int);
// Use the exit hook *only* to store the exit code.
static void custom_exit(int code) {
    exit_code = code;
    BYTEHOOK_CALL_PREV(custom_exit, exit_func, code);
    BYTEHOOK_POP_STACK();
}

static void custom_atexit() {
    if(exit_tripped) {
        return;
    }
    exit_tripped = true;
    nominal_exit(exit_code, false);
}

static void create_hooks(bytehook_hook_all_t bytehook_hook_all_p) {
    bytehook_stub_t stub_exit = bytehook_hook_all_p(NULL, "exit", &custom_exit, NULL, NULL);
    LOGI("Successfully initialized exit hook, stub: %p", stub_exit);
    // Only apply chmod hooks on devices where the game directory is in games/PojavLauncher
    // which is below API 29
    if(android_get_device_api_level() < 29) {
        create_chmod_hooks(bytehook_hook_all_p);
    }
}

static bool init_hooks() {
    void* bytehook_handle = dlopen("libbytehook.so", RTLD_NOW);
    if(bytehook_handle == NULL) {
        goto dlerror;
    }

    bytehook_hook_all_t bytehook_hook_all_p;
    int (*bytehook_init_p)(int mode, bool debug);

    bytehook_hook_all_p = dlsym(bytehook_handle, "bytehook_hook_all");
    bytehook_init_p = dlsym(bytehook_handle, "bytehook_init");

    if(bytehook_hook_all_p == NULL || bytehook_init_p == NULL) {
        goto dlerror;
    }
    int bhook_status = bytehook_init_p(BYTEHOOK_MODE_AUTOMATIC, false);
    if(bhook_status == BYTEHOOK_STATUS_CODE_OK) {
        create_hooks(bytehook_hook_all_p);
        return true;
    } else {
        LOGE("bytehook_init failed (%i)", bhook_status);
        dlclose(bytehook_handle);
        return false;
    }

    dlerror:
    if(bytehook_handle != NULL) dlclose(bytehook_handle);
    LOGE("Failed to load hook library: %s", dlerror());
    return false;
}

JNIEXPORT void JNICALL
Java_net_kdt_pojavlaunch_utils_JREUtils_initializeHooks(JNIEnv *env, jclass clazz) {
    bool hooks_ready = init_hooks();
    if(!hooks_ready) {
        LOGE("Failed to initialize native hooks!");
    }
    // Always register atexit, because that's what we will call our exit from.
    // We only use the hook to capture the exit code.
    atexit(custom_atexit);
}