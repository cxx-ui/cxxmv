
add_executable(employee_list_example main.cpp)
target_compile_options(employee_list_example PRIVATE -fconcepts-diagnostics-depth=20)
target_link_libraries(employee_list_example PRIVATE cxxmv-qt)
