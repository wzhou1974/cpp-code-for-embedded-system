add_compile_options(-std=c++14 -DCINATRA_ENABLE_SSL -DCPPHTTPLIB_OPENSSL_SUPPORT)

add_executable(unittest_download_1M50M700M test_download_1M50M700M.cpp
				$<TARGET_OBJECTS:dlc_objs>
				$<TARGET_OBJECTS:common_utils_objs>
				$<TARGET_OBJECTS:async++_objs>
				$<TARGET_OBJECTS:https_download_11_objs>
				$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_download_1M50M700M pthread ssl crypto boost_filesystem boost_system)
install(TARGETS unittest_download_1M50M700M RUNTIME DESTINATION bin)

add_executable(unittest_download_700M1G test_download_700M1G.cpp
				$<TARGET_OBJECTS:dlc_objs>
				$<TARGET_OBJECTS:common_utils_objs>
				$<TARGET_OBJECTS:async++_objs>
				$<TARGET_OBJECTS:https_download_11_objs>
				$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_download_700M1G pthread ssl crypto boost_filesystem boost_system)
install(TARGETS unittest_download_700M1G RUNTIME DESTINATION bin)

add_executable(unittest_download_url test_download_url.cpp
				$<TARGET_OBJECTS:dlc_objs>
				$<TARGET_OBJECTS:async++_objs>
				$<TARGET_OBJECTS:https_download_11_objs>
				$<TARGET_OBJECTS:common_utils_objs>
				$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_download_url pthread ssl crypto boost_filesystem boost_system)
install(TARGETS unittest_download_url RUNTIME DESTINATION bin)

add_executable(unittest_L1_parsing test_L1_parsing.cpp
				$<TARGET_OBJECTS:dlc_objs>
				$<TARGET_OBJECTS:common_utils_objs>
				$<TARGET_OBJECTS:async++_objs>
				$<TARGET_OBJECTS:https_download_11_objs>
				$<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(unittest_L1_parsing pthread ssl crypto boost_filesystem boost_system)
install(TARGETS unittest_L1_parsing RUNTIME DESTINATION bin)

add_executable(fake-dlc fake-dlc.cpp
				$<TARGET_OBJECTS:common_utils_objs>
)
target_link_libraries(fake-dlc pthread ssl crypto boost_filesystem boost_system)
install(TARGETS fake-dlc RUNTIME DESTINATION bin)