include_directories(
#	../3rdparty/asyncplusplus/include
)

add_compile_options(-std=c++17 -DCPPHTTPLIB_OPENSSL_SUPPORT)
add_library(dlc_objs OBJECT
	parse_manifest.cpp
	dlcstatus.cpp
	downloadclient.cpp
)

add_executable(dlc main.cpp
			$<TARGET_OBJECTS:dlc_objs>
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:https_download_11_objs>
			$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(dlc pthread ssl crypto stdc++fs boost_filesystem boost_system)
install(TARGETS dlc RUNTIME DESTINATION bin)

add_executable(unittest_downloadclient test_downloadclient.cpp
			$<TARGET_OBJECTS:dlc_objs>
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:https_download_11_objs>
			$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_downloadclient pthread ssl crypto stdc++fs boost_filesystem boost_system)
install(TARGETS unittest_downloadclient RUNTIME DESTINATION bin)

add_executable(unittest_downloadclient2 test_downloadclient2.cpp
			$<TARGET_OBJECTS:dlc_objs>
			$<TARGET_OBJECTS:common_utils_objs>
			$<TARGET_OBJECTS:https_download_11_objs>				
			$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_downloadclient2 pthread ssl crypto stdc++fs boost_filesystem boost_system)
install(TARGETS unittest_downloadclient2 RUNTIME DESTINATION bin)
