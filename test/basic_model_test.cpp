// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file basic_model_test.cpp
/// Contains unit tests for the basic_model class.

#include "test_user.hpp"
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <cxxmv/basic_model.hpp>


static_assert(mv::model_of<mv::basic_model<int>, int>);
static_assert(mv::model_of<mv::basic_model<std::string>, std::string>);


namespace {

}


BOOST_AUTO_TEST_SUITE(basic_model_test)


/// Tests default constructor
BOOST_AUTO_TEST_CASE(ctor_default) {
    mv::basic_model<int> mdl;
    BOOST_CHECK_EQUAL(mdl.get(), 0);
}


/// Tests construction from arguments
BOOST_AUTO_TEST_CASE(ctor_args) {
    mv::basic_model<test_user> mdl{"first", "last"};
    BOOST_CHECK_EQUAL(mdl->first_name(), "first");
    BOOST_CHECK_EQUAL(mdl->last_name(), "last");
}


/// Tests construction from initializer list
BOOST_AUTO_TEST_CASE(ctor_initializer_list) {
    mv::basic_model<std::vector<int>> mdl{1, 2, 3};
    BOOST_TEST(mdl.get() == (std::vector<int>{1, 2, 3}));
}


/// Tests assignment to model via * operator
BOOST_AUTO_TEST_CASE(assign_deref) {
    mv::basic_model<int> mdl{100};

    bool changed_called = false;
    mdl.changed.connect([&changed_called, &mdl] {
        changed_called = true;
        BOOST_CHECK(*mdl == 200);
    });

    mdl.mut().ref() = 200;

    BOOST_CHECK(*mdl == 200);

    BOOST_CHECK(changed_called);
}


/// Tests assignment to model via mutator
BOOST_AUTO_TEST_CASE(mut_assign) {
    mv::basic_model<int> mdl{100};

    int changed_count = 0;
    mdl.changed.connect([&changed_count, &mdl] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl.get(), 200);
    });

    mdl.mut() = 200;

    BOOST_CHECK_EQUAL(mdl.get(), 200);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


BOOST_AUTO_TEST_SUITE_END()
