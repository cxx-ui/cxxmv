
add_executable(cxxmv-test
               all_range_test.cpp
               all_test.cpp
               basic_model_test.cpp
               model_vector_test.cpp
               ref_transform_range_test.cpp
               ref_transform_test.cpp
               test.cpp
               transform_range_test.cpp
               transform_test.cpp
               vector_test.cpp
               zip_transform_test.cpp)

target_link_libraries(cxxmv-test PRIVATE cxxmv Boost::unit_test_framework)

add_test(NAME cxxmv-test COMMAND cxxmv-test)

if(CXXMV_ENABLE_QT)
    add_subdirectory(qt)
endif()
