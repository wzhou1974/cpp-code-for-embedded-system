find_package( Boost COMPONENTS system filesystem REQUIRED )

add_definitions(-DUSE_BOOST_FILESYSTEM)
add_compile_options(-std=c++11 -DCPPHTTPLIB_OPENSSL_SUPPORT)

add_library(updaterwrapper_objs OBJECT
            updaterwrapper.cpp
)