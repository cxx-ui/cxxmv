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


static_assert(mv::model<mv::basic_model<int>, int>);
static_assert(mv::model<mv::basic_model<long>, int>);
static_assert(mv::model<mv::basic_model<std::string>, std::string>);


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


/// Tests assignment to model
BOOST_AUTO_TEST_CASE(assign) {
    mv::basic_model<int> mdl{100};

    bool changed_called = false;
    mdl.changed.connect([&changed_called, &mdl] {
        changed_called = true;
        BOOST_CHECK(*mdl == 200);
    });

    mdl.assign(200);

    BOOST_CHECK(*mdl == 200);

    BOOST_CHECK(changed_called);
}


/// Tests assignment to model via * operator
BOOST_AUTO_TEST_CASE(assign_deref) {
    mv::basic_model<int> mdl{100};

    bool changed_called = false;
    mdl.changed.connect([&changed_called, &mdl] {
        changed_called = true;
        BOOST_CHECK(*mdl == 200);
    });

    *mdl = 200;

    BOOST_CHECK(*mdl == 200);

    BOOST_CHECK(changed_called);
}


BOOST_AUTO_TEST_SUITE_END()
