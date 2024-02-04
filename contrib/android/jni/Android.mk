LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := swe-2.10.03

LOCAL_LDFLAGS   += -ffunction-sections -fdata-sections -Wl,--gc-sections
LOCAL_CFLAGS    += -ffunction-sections -fdata-sections -fvisibility=hidden -Wall -Wno-error=implicit-function-declaration
LOCAL_SRC_FILES := swedate.c swehouse.c swejpl.c swemmoon.c swemplan.c sweph.c swephlib.c swecl.c swehel.c swejni.c
include $(BUILD_SHARED_LIBRARY)