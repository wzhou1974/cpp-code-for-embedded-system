# the fille is for building on ASOP build environment, not for NDK environment
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -fexceptions
# current ASOP build environment only support c++14
LOCAL_CPPFLAGS += -frtti -g -std=c++14
LOCAL_CFLAGS += -DANDROID_FLATORM
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/3rdparty
LOCAL_C_INCLUDES += $(LOCAL_PATH)/3rdparty/rest_rpc/third/msgpack/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/3rdparty/rest_rpc/include
# copy boost include directory to project directory, ASOP build environment doesn't accept
# directory out of ASOP. Maybe could, but I don't know currently.

# rest_rpc need boost::asio header file
LOCAL_C_INCLUDES += $(LOCAL_PATH)/boost/include/boost-1_73

LOCAL_SRC_FILES := idcm/dmc/msg-passer.cpp
LOCAL_MODULE := libmsg-passer
LOCAL_STRIP_MODULE := false
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

