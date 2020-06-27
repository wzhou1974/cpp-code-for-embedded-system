include_directories(
	"../3rdparty/rest_rpc/third/msgpack/include"
	"../3rdparty/rest_rpc/include"
)

add_definitions(-DUSE_BOOST_FILESYSTEM)
add_compile_options(-std=c++11)

add_executable(mock-selfinstaller mock_selfinstaller.cpp
			$<TARGET_OBJECTS:updaterwrapper_objs>
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:https_download_11_objs>
)
target_link_libraries(mock-selfinstaller boost_filesystem boost_system ssl crypto pthread)
install(TARGETS mock-selfinstaller RUNTIME DESTINATION bin)