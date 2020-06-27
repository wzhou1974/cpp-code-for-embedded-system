# build for c++11
#remove_definitions(-std=c++17)
add_definitions(-DUSE_BOOST_FILESYSTEM -DCPPHTTPLIB_OPENSSL_SUPPORT)

# for https_download
set_source_files_properties(../https_download.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
add_library(https_download_11_objs OBJECT
	../https_download.cpp
)
target_compile_options(https_download_11_objs PUBLIC -fPIC)

set_source_files_properties(../test_https_download.cpp PROPERTIES COMPILE_FLAGS -std=c++11 COMPILE_FLAGS -DUSE_BOOST_FILESYSTEM)
add_executable(unittest_https_download_11 ../test_https_download.cpp
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:https_download_11_objs>
			$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_https_download_11 boost_filesystem boost_system pthread ssl crypto)
install(TARGETS unittest_https_download_11 RUNTIME DESTINATION bin)

# for curl_downliad
set_source_files_properties(../../3rdparty/picohttpparser/picohttpparser.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
add_library(picohttpparser_objs OBJECT
			../../3rdparty/picohttpparser/picohttpparser.cpp
)
target_compile_options(picohttpparser_objs PUBLIC -fPIC)

set_source_files_properties(../curl_download.cpp PROPERTIES COMPILE_FLAGS -std=c++11)

add_library(curl_download_11_objs OBJECT
			../curl_download.cpp			
)
target_compile_options(curl_download_11_objs PUBLIC -fPIC)

set_source_files_properties(../test_curl_download.cpp PROPERTIES COMPILE_FLAGS -std=c++11 COMPILE_FLAGS -DUSE_BOOST_FILESYSTEM)
add_executable(unittest_curl_download_11 ../test_curl_download.cpp
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:curl_download_11_objs>
			$<TARGET_OBJECTS:fmt_objs>
			$<TARGET_OBJECTS:curlite_objs>	
			$<TARGET_OBJECTS:picohttpparser_objs>
)
target_link_libraries(unittest_curl_download_11 boost_filesystem boost_system pthread curl ssl crypto z)
install(TARGETS unittest_curl_download_11 RUNTIME DESTINATION bin)

