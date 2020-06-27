LOCAL_PATH:= $(call my-dir)

# prebuild libboost_filesystem
include $(CLEAR_VARS)
LOCAL_MODULE := boost_filesystem
LOCAL_SRC_FILES := /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/lib/libboost_filesystem-clang-mt-a64-1_73.a
NDK_APP_DST_DIR := ../install/android/3rdparty/lib/$(TARGET_ARCH_ABI)
include $(PREBUILT_STATIC_LIBRARY)

# prebuild libboost_system
include $(CLEAR_VARS)
LOCAL_MODULE := boost_system
LOCAL_SRC_FILES := /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/lib/libboost_system-clang-mt-a64-1_73.a
NDK_APP_DST_DIR := ../install/android/3rdparty/lib/$(TARGET_ARCH_ABI)
include $(PREBUILT_STATIC_LIBRARY)

# prebuild libssl.so
include $(CLEAR_VARS)
LOCAL_MODULE := openssl_ssl
LOCAL_SRC_FILES := /media/walter/a76b8e08-5286-4747-b6ad-f631e4e0de29/android/out/target/product/generic_arm64/system/lib64/libssl.so
NDK_APP_DST_DIR := ../install/android/3rdparty/lib/$(TARGET_ARCH_ABI)
include $(PREBUILT_SHARED_LIBRARY)

#prebuild libcrypt.so
include $(CLEAR_VARS)
LOCAL_MODULE := openssl_crypto
LOCAL_SRC_FILES := /media/walter/a76b8e08-5286-4747-b6ad-f631e4e0de29/android/out/target/product/generic_arm64/system/lib64/libcrypto.so
NDK_APP_DST_DIR := ../install/android/3rdparty/lib/$(TARGET_ARCH_ABI)
include $(PREBUILT_SHARED_LIBRARY)

# build libmsg-passer.so
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DANDROID_FLATORM -DCPPHTTPLIB_OPENSSL_SUPPORT
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_C_INCLUDES := ./3rdparty
LOCAL_C_INCLUDES += ./3rdparty/fmt/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/third/msgpack/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/include
LOCAL_C_INCLUDES += ./3rdparty/cinatra/include
LOCAL_C_INCLUDES += /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/include/boost-1_73
LOCAL_SRC_FILES:= ./idcm/dmc/msg-passer.cpp
LOCAL_MODULE := libmsg-passer
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := boost_filesystem
LOCAL_SHARED_LIBRARIES += boost_system
NDK_APP_DST_DIR := ../install/android/$(APP_OPTIM)/lib/$(TARGET_ARCH_ABI)
cmd-strip :=
include $(BUILD_SHARED_LIBRARY)

# build fake-dmc
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DANDROID_FLATORM -DCPPHTTPLIB_OPENSSL_SUPPORT
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_C_INCLUDES := ./3rdparty
LOCAL_C_INCLUDES += ./3rdparty/fmt/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/third/msgpack/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/include
LOCAL_C_INCLUDES += ./3rdparty/cinatra/include
LOCAL_C_INCLUDES += /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/include/boost-1_73
LOCAL_SRC_FILES := ./test-idcm/dmc/fake-dmc.cpp
LOCAL_CPP_EXTENSION := .cxx .cpp .cc
LOCAL_SRC_FILES += ./3rdparty/fmt/src/format.cc
LOCAL_SRC_FILES += ./3rdparty/fmt/src/os.cc
LOCAL_MODULE := fake-dmc
LOCAL_SHARED_LIBRARIES := libmsg-passer
LOCAL_SHARED_LIBRARIES += boost_filesystem
LOCAL_SHARED_LIBRARIES += boost_system
LOCAL_LDLIBS := -llog
NDK_APP_DST_DIR := ../install/android/$(APP_OPTIM)/bin/$(TARGET_ARCH_ABI)
cmd-strip :=
include $(BUILD_EXECUTABLE)

# build fake-dlc
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DANDROID_FLATORM -DCPPHTTPLIB_OPENSSL_SUPPORT
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_C_INCLUDES := ./3rdparty
LOCAL_C_INCLUDES += ./3rdparty/fmt/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/third/msgpack/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/include
LOCAL_C_INCLUDES += ./3rdparty/PPK_ASSERT/src
LOCAL_C_INCLUDES += /media/walter/a76b8e08-5286-4747-b6ad-f631e4e0de29/android/external/boringssl/src/include
LOCAL_C_INCLUDES += /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/include/boost-1_73

LOCAL_SRC_FILES := ./test-idcm/dlc/fake-dlc.cpp
LOCAL_SRC_FILES += ./common/hh_helper.cpp
LOCAL_SRC_FILES += ./common/utils.cpp
LOCAL_SRC_FILES += ./3rdparty/hash-library/md5.cpp
LOCAL_SRC_FILES += ./3rdparty/PPK_ASSERT/src/ppk_assert.cpp

LOCAL_MODULE := fake-dlc
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := boost_filesystem
LOCAL_SHARED_LIBRARIES += boost_system
NDK_APP_DST_DIR := ../install/android/$(APP_OPTIM)/bin/$(TARGET_ARCH_ABI)
include $(BUILD_EXECUTABLE)

# build dlc
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cxx .cpp .cc
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS += -DANDROID_FLATORM -D_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM -DCINATRA_ENABLE_SSL
LOCAL_CPPFLAGS += -DDCPPHTTPLIB_OPENSSL_SUPPORT -DUSE_BOOST_FILESYSTEM
LOCAL_CPPFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
LOCAL_CPPFLAGS += -Wno-unused-parameter

LOCAL_C_INCLUDES := ./3rdparty
LOCAL_C_INCLUDES += ./3rdparty/fmt/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/third/msgpack/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/include
LOCAL_C_INCLUDES += ./3rdparty/cinatra/include
LOCAL_C_INCLUDES += ./3rdparty/spdlog/include
LOCAL_C_INCLUDES += ./3rdparty/PPK_ASSERT/src
LOCAL_C_INCLUDES += /media/walter/a76b8e08-5286-4747-b6ad-f631e4e0de29/android/external/boringssl/src/include
LOCAL_C_INCLUDES += LOCAL_C_INCLUDES += /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/include/boost-1_73

LOCAL_SRC_FILES := ./idcm/dlc/main.cpp

# for dlc_objs
LOCAL_SRC_FILES += ./idcm/dlc/dlcstatus.cpp
LOCAL_SRC_FILES += ./idcm/dlc/downloadclient.cpp
LOCAL_SRC_FILES += ./idcm/dlc/parse_manifest.cpp

# for common_utils_objs
LOCAL_SRC_FILES += ./common/utils.cpp
LOCAL_SRC_FILES += ./common/hh_helper.cpp
LOCAL_SRC_FILES += ./3rdparty/hash-library/md5.cpp
LOCAL_SRC_FILES += ./3rdparty/PPK_ASSERT/src/ppk_assert.cpp

# for https_download_11_objs
LOCAL_SRC_FILES += ./common/https_download.cpp

# for fmt_objs
LOCAL_SRC_FILES += ./3rdparty/fmt/src/format.cc
LOCAL_SRC_FILES += ./3rdparty/fmt/src/os.cc

LOCAL_SRC_FILES += ./3rdparty/android-ifaddrs/ifaddrs.c

LOCAL_SHARED_LIBRARIES := openssl_ssl
LOCAL_SHARED_LIBRARIES += openssl_crypto
LOCAL_SHARED_LIBRARIES += boost_filesystem
LOCAL_SHARED_LIBRARIES += boost_system
LOCAL_MODULE := dlc
LOCAL_CPPFLAGS := -stdlib=libc++
# The file is for NDK build environment
LOCAL_LDLIBS := -llog
NDK_APP_DST_DIR := ../install/android/$(APP_OPTIM)/bin/$(TARGET_ARCH_ABI)
include $(BUILD_EXECUTABLE)

# build https-server
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DANDROID_FLATORM -D_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM -DCINATRA_ENABLE_SSL
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_C_INCLUDES := ./3rdparty
LOCAL_C_INCLUDES += ./3rdparty/fmt/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/third/msgpack/include
LOCAL_C_INCLUDES += ./3rdparty/rest_rpc/include
LOCAL_C_INCLUDES += ./3rdparty/cinatra-modified/include
LOCAL_C_INCLUDES += ./3rdparty/spdlog/include
LOCAL_C_INCLUDES += /media/walter/a76b8e08-5286-4747-b6ad-f631e4e0de29/android/external/boringssl/src/include
LOCAL_C_INCLUDES += /home/walter/workspace/work/fota/android/Boost-for-Android/build/out/arm64-v8a/include/boost-1_73

LOCAL_SRC_FILES := ./https-server/server.cpp
LOCAL_SRC_FILES += ./common/utils.cpp
LOCAL_SRC_FILES += ./3rdparty/hash-library/md5.cpp
LOCAL_MODULE := https-server
LOCAL_SHARED_LIBRARIES := openssl_ssl
LOCAL_SHARED_LIBRARIES += openssl_crypto
LOCAL_SHARED_LIBRARIES += boost_filesystem
LOCAL_SHARED_LIBRARIES += boost_system
LOCAL_LDLIBS := -llog
NDK_APP_DST_DIR := ../install/android/$(APP_OPTIM)/bin/$(TARGET_ARCH_ABI)
include $(BUILD_EXECUTABLE)