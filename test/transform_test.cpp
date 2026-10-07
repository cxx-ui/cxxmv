// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file transform_test.cpp
/// Contains unit tests for the transform projection.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/basic_model.hpp>
#include <cxxmv/ref_ransform.hpp>
#include <cxxmv/transform.hpp>
#include <memory>
#include <type_traits>
#include <utility>


BOOST_AUTO_TEST_SUITE(transform_test)


BOOST_AUTO_TEST_CASE(transform_int_float) {
    mv::basic_model<int> mdl{100};

    auto get_fn = [](int x) { return static_cast<float>(x); };
    auto set_fn = [](int & x, float y) { x = static_cast<int>(y); };
    auto mdl2 = mdl | mv::transform(get_fn, set_fn);

    using transform_t = std::decay_t<decltype(mdl2)>;

    static_assert(mv::model_of<transform_t, float>);
    static_assert(std::move_constructible<transform_t>);
    static_assert(std::copy_constructible<transform_t>);

    bool changed_called = false;
    mdl2.changed().connect([&changed_called, &mdl, &mdl2] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl.get(), 200);
        BOOST_CHECK_EQUAL(mdl2.get(), 200.0f);
    });

    mdl2.mut().ref() = 200.0f;

    BOOST_CHECK_EQUAL(mdl.get(), 200);
    BOOST_CHECK_EQUAL(mdl2.get(), 200.0f);

    BOOST_CHECK(changed_called);
}


/// Tests assigning transform projection
BOOST_AUTO_TEST_CASE(transform_assign) {
    mv::basic_model<test_user> mdl{"first", "last"};

    auto get_fn = [](const test_user & u) { return u.last_name(); };
    auto set_fn = [](test_user & u, const std::string & last) { u.set_last_name(last); };
    auto mdl2 = mdl | mv::transform(get_fn, set_fn);

    bool changed_called = false;
    mdl2.changed().connect([&changed_called, &mdl, &mdl2] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl->first_name(), "first");
        BOOST_CHECK_EQUAL(mdl->last_name(), "new last");
        BOOST_CHECK_EQUAL(mdl2.get(), "new last");
    });

    mdl2.mut().ref() = "new last";

    BOOST_CHECK_EQUAL(mdl->first_name(), "first");
    BOOST_CHECK_EQUAL(mdl->last_name(), "new last");
    BOOST_CHECK_EQUAL(mdl2.get(), "new last");

    BOOST_CHECK(changed_called);
}


/// Tests conversion of transform projection to model value
BOOST_AUTO_TEST_CASE(transform_deref_convert) {
    mv::basic_model<test_user> mdl{"first", "last"};

    auto get_fn = [](const test_user & u) { return u.last_name(); };
    auto set_fn = [](test_user & u, const std::string & last) { u.set_last_name(last); };
    auto mdl2 = mdl | mv::transform(get_fn, set_fn);

    std::string val = *mdl2;
    BOOST_CHECK_EQUAL(val, "last");
}


/// Tests conversion of transform projection to model value in function call
BOOST_AUTO_TEST_CASE(transform_deref_convert_call) {
    mv::basic_model<test_user> mdl{"first", "last"};

    auto get_fn = [](const test_user & u) { return u.last_name(); };
    auto set_fn = [](test_user & u, const std::string & last) { u.set_last_name(last); };
    auto mdl2 = mdl | mv::transform(get_fn, set_fn);

    auto call_func = [](const std::string & val) {
        BOOST_CHECK_EQUAL(val, "last");
    };

    call_func(*mdl2);
}


/// Tests transform projection of temporary model
BOOST_AUTO_TEST_CASE(transform_temporary_model) {
    auto get_fn = [](int x) { return x + 1; };
    auto set_fn = [](int & x, int y) { x = y - 1; };
    auto mdl = mv::basic_model<int>{10} | mv::transform(get_fn, set_fn);

    BOOST_CHECK_EQUAL(mdl.get(), 11);

    bool changed_called = false;
    mdl.changed().connect([&changed_called, &mdl] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl.get(), 21);
    });

    mdl.mut().ref() = 21;

    BOOST_CHECK_EQUAL(mdl.get(), 21);
    BOOST_CHECK(changed_called);
}


/// Tests moving transform projection of temporary model
BOOST_AUTO_TEST_CASE(transform_temporary_model_move) {
    auto get_fn = [](int x) { return x + 1; };
    auto set_fn = [](int & x, int y) { x = y - 1; };
    auto mdl = mv::basic_model<int>{10} | mv::transform(get_fn, set_fn);
    auto mdl2 = std::move(mdl);

    BOOST_CHECK_EQUAL(mdl2.get(), 11);

    bool changed_called = false;
    mdl2.changed().connect([&changed_called, &mdl2] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl2.get(), 21);
    });

    mdl2.mut().ref() = 21;

    BOOST_CHECK_EQUAL(mdl2.get(), 21);
    BOOST_CHECK(changed_called);
}


/// Tests copying transform projection of model reference
BOOST_AUTO_TEST_CASE(transform_ref_model_copy) {
    mv::basic_model<int> mdl{10};

    auto get_fn = [](int x) { return x + 1; };
    auto set_fn = [](int & x, int y) { x = y - 1; };
    auto mdl2 = mdl | mv::transform(get_fn, set_fn);
    auto mdl3 = mdl2;

    BOOST_CHECK_EQUAL(mdl3.get(), 11);

    bool changed_called = false;
    mdl3.changed().connect([&changed_called, &mdl, &mdl2, &mdl3] {
        changed_called = true;
        BOOST_CHECK_EQUAL(mdl.get(), 20);
        BOOST_CHECK_EQUAL(mdl2.get(), 21);
        BOOST_CHECK_EQUAL(mdl3.get(), 21);
    });

    mdl3.mut().ref() = 21;

    BOOST_CHECK_EQUAL(mdl.get(), 20);
    BOOST_CHECK_EQUAL(mdl2.get(), 21);
    BOOST_CHECK_EQUAL(mdl3.get(), 21);
    BOOST_CHECK(changed_called);
}


/// Tests moving transform mutator
BOOST_AUTO_TEST_CASE(transform_mutator_move) {
    mv::basic_model<int> mdl{10};

    auto get_fn = [](int x) { return x + 1; };
    auto set_fn = [](int & x, int y) { x = y - 1; };
    auto mdl2 = mdl | mv::transform(get_fn, set_fn);

    int changed_count = 0;
    mdl2.changed().connect([&changed_count, &mdl, &mdl2] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl.get(), 20);
        BOOST_CHECK_EQUAL(mdl2.get(), 21);
    });

    {
        auto mut = mdl2.mut();
        BOOST_CHECK_EQUAL(mut.ref(), 11);

        mut.ref() = 21;

        auto mut2 = std::move(mut);
        BOOST_CHECK_EQUAL(mut2.ref(), 21);

        // model is not changed until mutator is destroyed
        BOOST_CHECK_EQUAL(mdl.get(), 10);
        BOOST_CHECK_EQUAL(changed_count, 0);
    }

    // value is assigned and signal is emitted only once by destroyed mutator
    BOOST_CHECK_EQUAL(mdl.get(), 20);
    BOOST_CHECK_EQUAL(mdl2.get(), 21);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


BOOST_AUTO_TEST_SUITE_END()
