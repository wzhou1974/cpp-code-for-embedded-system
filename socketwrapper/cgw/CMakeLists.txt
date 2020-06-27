include_directories(
	"../3rdparty/rest_rpc/third/msgpack/include"
	"../3rdparty/rest_rpc/include"
)

add_definitions(-DUSE_BOOST_FILESYSTEM)
add_compile_options(-std=c++11)

add_subdirectory(dlc-proxy)
add_subdirectory(self-updater-proxy)
