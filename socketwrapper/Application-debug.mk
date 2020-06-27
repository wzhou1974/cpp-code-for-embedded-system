APP_OPTIM := debug
APP_ABI := arm64-v8a #armeabi-v7a
#APP_STL := c++_shared
APP_STL := c++_static
APP_CPPFLAGS := -frtti -fexceptions -std=c++2a -g -DANDROID_FLATORM -Wnon-virtual-dtor
APP_PLATFORM := android-27
APP_BUILD_SCRIPT := Android-debug.mk
