include_directories(
	"../3rdparty/rest_rpc/third/msgpack/include"
	"../3rdparty/rest_rpc/include"
)

add_compile_options(-std=c++11)

add_library(selfupdater-proxy SHARED selfupdater-proxy.cpp
			$<TARGET_OBJECTS:common_utils_objs>	
)
set_target_properties(selfupdater-proxy PROPERTIES VERSION 1.0 SOVERSION 1)

add_library(selfupdater-proxy-static STATIC selfupdater-proxy.cpp
			$<TARGET_OBJECTS:common_utils_objs>
)
set_target_properties(selfupdater-proxy-static PROPERTIES OUTPUT_NAME "selfupdater-proxy")
install(TARGETS selfupdater-proxy selfupdater-proxy-static DESTINATION lib)

set_target_properties(selfupdater-proxy PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(selfupdater-proxy-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_executable(test-selfupdater-proxy test_main.cpp)
target_link_libraries(test-selfupdater-proxy selfupdater-proxy boost_filesystem boost_system pthread ssl crypto)
install(TARGETS test-selfupdater-proxy RUNTIME DESTINATION bin)
