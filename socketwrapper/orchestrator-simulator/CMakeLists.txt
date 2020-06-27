add_executable(orch-simulator orch-simulator.cpp
            $<TARGET_OBJECTS:common_utils_objs>
            $<TARGET_OBJECTS:fmt_objs>

)
target_link_libraries(orch-simulator boost_filesystem boost_system pthread selfupdater-proxy)
install(TARGETS orch-simulator RUNTIME DESTINATION bin)