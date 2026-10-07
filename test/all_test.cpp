// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all_test.cpp
/// Contains unit tests for the all projection.

#include "cxxmv/observable.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/all.hpp>
#include <cxxmv/basic_model.hpp>
#include <type_traits>


BOOST_AUTO_TEST_SUITE(all_test)


/// Tests changing model via all projection of model reference
BOOST_AUTO_TEST_CASE(all_ref_model) {
    mv::basic_model<int> mdl{100};
    auto mdl2 = mdl | mv::all;

    using all_t = std::decay_t<decltype(mdl2)>;

    static_assert(mv::model_of<all_t, int>);
    static_assert(mv::borrowed_observable<all_t>);
    static_assert(std::move_constructible<all_t>);
    static_assert(std::copy_constructible<all_t>);

    BOOST_CHECK_EQUAL(mdl2.get(), 100);

    bool changed_called = false;
    mdl2.changed().connect([&changed_called, &mdl, &mdl2] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl.get(), 200);
        BOOST_CHECK_EQUAL(mdl2.get(), 200);
    });

    mdl2.mut().ref() = 200;

    BOOST_CHECK_EQUAL(mdl.get(), 200);
    BOOST_CHECK_EQUAL(mdl2.get(), 200);

    BOOST_CHECK(changed_called);
}


/// Tests connecting to all projection via temporary ref object
BOOST_AUTO_TEST_CASE(all_ref_borrowed) {
    mv::basic_model<int> mdl{100};
    auto mdl2 = mdl | mv::all;

    BOOST_CHECK_EQUAL(mdl2.get(), 100);

    bool changed_called = false;
    (mdl | mv::all).changed().connect([&changed_called, &mdl, &mdl2] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl.get(), 200);
        BOOST_CHECK_EQUAL(mdl2.get(), 200);
    });

    mdl2.mut().ref() = 200;

    BOOST_CHECK_EQUAL(mdl.get(), 200);
    BOOST_CHECK_EQUAL(mdl2.get(), 200);

    BOOST_CHECK(changed_called);
}


/// Tests all projection of temporary model
BOOST_AUTO_TEST_CASE(all_temporary_model) {
    auto mdl = mv::basic_model<int>{100} | mv::all;

    using all_t = std::decay_t<decltype(mdl)>;

    static_assert(mv::model_of<all_t, int>);
    static_assert(!mv::borrowed_observable<all_t>);
    static_assert(std::move_constructible<all_t>);
    static_assert(!std::copy_constructible<all_t>);

    BOOST_CHECK_EQUAL(mdl.get(), 100);

    bool changed_called = false;
    mdl.changed().connect([&changed_called, &mdl] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl.get(), 200);
    });

    mdl.mut().ref() = 200;

    BOOST_CHECK_EQUAL(mdl.get(), 200);

    BOOST_CHECK(changed_called);
}


BOOST_AUTO_TEST_SUITE_END()
