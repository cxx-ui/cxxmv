
add_executable(employee_list_example
               employee.cpp
               main.cpp)

target_link_libraries(employee_list_example PRIVATE cxxmv-qt)
