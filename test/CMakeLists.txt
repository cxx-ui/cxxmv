
add_executable(cxxmv-test
               basic_model_test.cpp
               test.cpp
               transform_test.cpp
               zip_transform_test.cpp)

target_link_libraries(cxxmv-test PRIVATE cxxmv Boost::unit_test_framework)

add_test(NAME cxxmv-test COMMAND cxxmv-test)
