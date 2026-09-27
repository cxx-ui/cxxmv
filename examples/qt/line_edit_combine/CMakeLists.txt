
add_executable(line_edit_combine_example main.cpp)
target_compile_options(line_edit_combine_example PRIVATE -fconcepts-diagnostics-depth=20)
target_link_libraries(line_edit_combine_example PRIVATE cxxmv-qt)
