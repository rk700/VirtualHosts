LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= hookHosts.c

LOCAL_LDLIBS    := -llog

LOCAL_MODULE:= hookHosts

include $(BUILD_SHARED_LIBRARY)
