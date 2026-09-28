
add_executable(contact_list_example main.cpp)
target_compile_options(contact_list_example PRIVATE -fconcepts-diagnostics-depth=20)
target_link_libraries(contact_list_example PRIVATE cxxmv-qt)
