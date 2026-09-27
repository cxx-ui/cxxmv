
cmake_minimum_required(VERSION 3.20)
project(cxxmv)

include(CTest)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost CONFIG REQUIRED COMPONENTS unit_test_framework)


add_library(cxxmv INTERFACE)
target_include_directories(cxxmv INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(cxxmv INTERFACE Boost::headers)

add_subdirectory(examples)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
