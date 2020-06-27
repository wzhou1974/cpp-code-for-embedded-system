APP_OPTIM := release
APP_ABI := arm64-v8a #armeabi-v7a
#APP_STL := c++_shared
APP_STL := c++_static
APP_CPPFLAGS := -frtti -fexceptions -O2 -std=c++2a -DANDROID_FLATORM -Wnon-virtual-dtor
APP_PLATFORM := android-21
APP_BUILD_SCRIPT := Android-release.mk
