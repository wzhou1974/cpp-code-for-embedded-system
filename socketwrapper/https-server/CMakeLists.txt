include_directories(
	"../3rdparty/cinatra-modified/include"
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX17)
#    add_compile_options(-std=c++17 -DCINATRA_ENABLE_SSL -DCPPHTTPLIB_OPENSSL_SUPPORT)
elseif(COMPILER_SUPPORTS_CXX14)
#    add_compile_options(-std=c++14 -DCINATRA_ENABLE_SSL -DCPPHTTPLIB_OPENSSL_SUPPORT)
else()
#    add_compile_options(-std=c++11 -DCINATRA_ENABLE_SSL -DCPPHTTPLIB_OPENSSL_SUPPORT)
endif()

# https and http server will be running on IDCM or PC, clang++ on IDCM and g++ on PC
# both support c++17 feature.
add_compile_options(-std=c++17)

set_source_files_properties(https-server.cpp PROPERTIES COMPILE_FLAGS -DCINATRA_ENABLE_SSL)
add_executable(https-server https-server.cpp
    $<TARGET_OBJECTS:common_utils_objs>
)
target_link_libraries(https-server pthread ssl crypto boost_filesystem boost_system)
install(TARGETS https-server RUNTIME DESTINATION bin)

add_executable(http-server http-server.cpp
    $<TARGET_OBJECTS:common_utils_objs>
)
target_link_libraries(http-server pthread boost_filesystem boost_system)
install(TARGETS http-server RUNTIME DESTINATION bin)


#add_executable(test-cinatra-download test-cinatra-download.cpp
#            $<TARGET_OBJECTS:common_utils_objs>
#            $<TARGET_OBJECTS:fmt_objs>
#)
#target_link_libraries(test-cinatra-download stdc++fs pthread ssl crypto)
#install(TARGETS test-cinatra-download RUNTIME DESTINATION bin)

add_executable(test-httplib-download test-httplib-download.cpp
            $<TARGET_OBJECTS:common_utils_objs>
            $<TARGET_OBJECTS:https_download_11_objs>
            $<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(test-httplib-download pthread ssl crypto boost_filesystem boost_system)
install(TARGETS test-httplib-download RUNTIME DESTINATION bin)

#set_source_files_properties(test-filesize.cpp PROPERTIES COMPILE_FLAGS -DCPPHTTPLIB_OPENSSL_SUPPORT)
#add_executable(test-filesize test-filesize.cpp
#            $<TARGET_OBJECTS:https_download_11_objs>
#            $<TARGET_OBJECTS:fmt_objs>
#)
#target_link_libraries(test-filesize stdc++fs pthread ssl crypto)
#install(TARGETS test-filesize RUNTIME DESTINATION bin)