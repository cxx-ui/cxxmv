// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file zip_transform_test.cpp
/// Contains unit tests for the zip_transform projection.

#include "cxxmv/observable.hpp"
#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <concepts>
#include <cxxmv/all.hpp>
#include <cxxmv/basic_model.hpp>
#include <cxxmv/zip_transform.hpp>


BOOST_AUTO_TEST_SUITE(zip_transform_test)


/// Tests getting value of zip transform projection
BOOST_AUTO_TEST_CASE(zip_transform_int_float) {
    mv::basic_model<int> mdl1{100};
    mv::basic_model<float> mdl2{0.5f};

    auto get_fn = [](int x, float y) { return static_cast<float>(x) + y; };
    auto zip = mv::zip_transform(get_fn, mdl1, mdl2);

    using zip_t = std::decay_t<decltype(zip)>;

    static_assert(mv::projectable_observable_as<zip_t, float>);
    static_assert(std::copy_constructible<zip_t>);
    static_assert(std::move_constructible<zip_t>);

    BOOST_CHECK_EQUAL(zip.get(), 100.5f);
    BOOST_CHECK_EQUAL(*zip, 100.5f);
}


/// Tests that zip transform projection pass changes to base model
BOOST_AUTO_TEST_CASE(zip_transform_base_changed) {
    mv::basic_model<test_user> mdl1{"first", "last"};
    mv::basic_model<std::string> mdl2{"Mr."};

    auto get_fn = [](const test_user & u, const std::string & title) {
        return title + " " + u.last_name();
    };
    mv::zip_transform_projection zip{get_fn, mdl1 | mv::all, mdl2 | mv::all};

    bool changed_called = false;
    zip.changed.connect([&changed_called, &zip] {
        changed_called = true;
        BOOST_CHECK_EQUAL(zip.get(), "Dr. last");
    });

    mdl2.mut().ref() = "Dr.";

    BOOST_CHECK_EQUAL(zip.get(), "Dr. last");

    BOOST_CHECK(changed_called);
}


/// Tests conversion of zip transform projection to value
BOOST_AUTO_TEST_CASE(zip_transform_deref_convert) {
    mv::basic_model<test_user> mdl1{"first", "last"};
    mv::basic_model<std::string> mdl2{"Mr."};

    auto get_fn = [](const test_user & u, const std::string & title) {
        return title + " " + u.last_name();
    };
    mv::zip_transform_projection zip{get_fn, mdl1 | mv::all, mdl2 | mv::all};

    std::string val = *zip;
    BOOST_CHECK_EQUAL(val, "Mr. last");
}


/// Tests conversion of zip transform projection to value in function call
BOOST_AUTO_TEST_CASE(zip_transform_deref_convert_call) {
    mv::basic_model<test_user> mdl1{"first", "last"};
    mv::basic_model<std::string> mdl2{"Mr."};

    auto get_fn = [](const test_user & u, const std::string & title) {
        return title + " " + u.last_name();
    };
    mv::zip_transform_projection zip{get_fn, mdl1 | mv::all, mdl2 | mv::all};

    auto call_func = [](const std::string & val) {
        BOOST_CHECK_EQUAL(val, "Mr. last");
    };

    call_func(*zip);
}


BOOST_AUTO_TEST_SUITE_END()
