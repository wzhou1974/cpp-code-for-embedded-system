include_directories(
	../3rdparty/asyncplusplus/include
	../3rdparty/asyncplusplus/src
	../3rdparty/fmt/include
)

add_compile_options(-std=c++11)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE)

add_library(async++_objs OBJECT
	../3rdparty/asyncplusplus/src/scheduler.cpp
	../3rdparty/asyncplusplus/src/threadpool_scheduler.cpp
)

add_library(fmt_objs OBJECT
	../3rdparty/fmt/src/format.cc
	../3rdparty/fmt/src/os.cc
)

add_library(common_utils_objs OBJECT
	utils.cpp
	hh_helper.cpp
	../3rdparty/hash-library/md5.cpp
	../3rdparty/PPK_ASSERT/src/ppk_assert.cpp
)

add_library(curlite_objs OBJECT
	../3rdparty/curlite/curlite.cpp
)

target_compile_options(common_utils_objs PUBLIC -fPIC)
add_subdirectory(c++11)
#add_subdirectory(c++17)
