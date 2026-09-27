
cmake_minimum_required(VERSION 3.20)
project(cxxmv)

include(CTest)

option(CXXMV_ENABLE_QT "Enable Qt implementation" ON)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost CONFIG REQUIRED COMPONENTS unit_test_framework)


add_library(cxxmv INTERFACE)
target_include_directories(cxxmv INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(cxxmv INTERFACE Boost::headers)

if(CXXMV_ENABLE_QT)
    find_package(Qt6 COMPONENTS "Widgets")
    add_library(cxxmv-qt INTERFACE)
    target_link_libraries(cxxmv-qt INTERFACE cxxmv Qt6::Widgets)
endif()

add_subdirectory(examples)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
