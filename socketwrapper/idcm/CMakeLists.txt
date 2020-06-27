include_directories(
	"../3rdparty/rest_rpc/third/msgpack/include"
	"../3rdparty/rest_rpc/include"
)

add_compile_options(-std=c++14)

add_subdirectory(dlc)
add_subdirectory(dmc)
