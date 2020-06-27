add_compile_options(-std=c++14)

add_library(msg-passer SHARED msg-passer.cpp )
set_target_properties(msg-passer PROPERTIES VERSION 1.0 SOVERSION 1)
target_link_libraries(msg-passer boost_filesystem boost_system)

add_library(msg-passer-static STATIC msg-passer.cpp)
set_target_properties(msg-passer-static PROPERTIES OUTPUT_NAME "msg-passer")
target_link_libraries(msg-passer-static libboost_filesystem.a libboost_system.a)

install(TARGETS msg-passer msg-passer-static DESTINATION lib)