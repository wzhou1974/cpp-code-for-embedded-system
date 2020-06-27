set(CMAKE_SYSTEM_NAME Linux)


set(TOOLS /home/walter/workspace/work/fota/cgw/sysroots/x86_64-pokysdk-linux) 
set(CMAKE_C_COMPILER ${TOOLS}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)


set(CGW_SYSROOT /home/walter/workspace/work/fota/cgw/sysroots/aarch64-poky-linux)

set(CMAKE_FIND_ROOT_PATH ${CGW_SYSROOT} /home/walter/workspace/work/fota/cgw/sdk_20190919)
set(CMAKE_SYSROOT ${CGW_SYSROOT})


