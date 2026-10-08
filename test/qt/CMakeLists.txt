
add_executable(cxxmv-qt-test
               range_model_test.cpp
               single_selection_model_test.cpp
               test.cpp)

target_link_libraries(cxxmv-qt-test PRIVATE cxxmv-qt Boost::unit_test_framework)

add_test(NAME cxxmv-qt-test COMMAND cxxmv-qt-test)
