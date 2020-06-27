add_compile_options(-std=c++17)

add_executable(fake-dmc fake-dmc.cpp
            $<TARGET_OBJECTS:fmt_objs>
)
target_link_libraries(fake-dmc msg-passer pthread)
install(TARGETS fake-dmc RUNTIME DESTINATION bin)
