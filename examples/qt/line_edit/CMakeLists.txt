
add_executable(line_edit_example main.cpp)
target_compile_options(line_edit_example PRIVATE -fconcepts-diagnostics-depth=20)
target_link_libraries(line_edit_example PRIVATE cxxmv-qt)
