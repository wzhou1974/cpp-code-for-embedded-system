include_directories(
	"../3rdparty/rest_rpc/third/msgpack/include"
	"../3rdparty/rest_rpc/include"
)

add_compile_options(-std=c++11)

add_library(dlc-proxy SHARED dlc-proxy.cpp
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:https_download_11_objs>
)
set_target_properties(dlc-proxy PROPERTIES VERSION 1.0 SOVERSION 1)

add_library(dlc-proxy-static STATIC dlc-proxy.cpp
			$<TARGET_OBJECTS:common_utils_objs>
		
)
set_target_properties(dlc-proxy-static PROPERTIES OUTPUT_NAME "dlc-proxy")
install(TARGETS dlc-proxy dlc-proxy-static DESTINATION lib)

set_target_properties(dlc-proxy PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(dlc-proxy-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# test-dlc-proxy
add_executable(test-dlc-proxy test_main.cpp
			$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(test-dlc-proxy dlc-proxy boost_filesystem boost_system pthread ssl crypto)
install(TARGETS test-dlc-proxy RUNTIME DESTINATION bin)
