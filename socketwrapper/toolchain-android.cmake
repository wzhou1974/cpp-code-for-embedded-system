set(CMAKE_SYSTEM_NAME Android)

set(TOOLS /home/walter/workspace/work/fota/android/aarch64-linux-android) 
set(CMAKE_C_COMPILER ${TOOLS}/bin/aarch64-linux-android21-clang)
set(CMAKE_CXX_COMPILER ${TOOLS}/bin/aarch64-linux-android21-clang++)

set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)


set(CGW_SYSROOT /home/walter/workspace/work/fota/android/aarch64-linux-android/sysroot)

set(CMAKE_FIND_ROOT_PATH ${CGW_SYSROOT})
set(CMAKE_SYSROOT ${CGW_SYSROOT})


