// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file proxy_test.cpp
/// Contains unit tests for the proxy class.

#include <boost/test/unit_test.hpp>
#include <cxxmv/all.hpp>
#include <cxxmv/basic_model.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/proxy.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/vector.hpp>
#include <utility>


BOOST_AUTO_TEST_SUITE(proxy_test)


/// Tests construction of proxy for model reference
BOOST_AUTO_TEST_CASE(ctor) {
    using proxy_t = mv::proxy<mv::all_t<mv::basic_model<int> &>>;

    static_assert(mv::model_of<proxy_t, int>);
    static_assert(!mv::nullable_observable<proxy_t>);

    mv::basic_model<int> mdl{1};
    mv::proxy prx{mdl};
    BOOST_CHECK_EQUAL(prx.get(), 1);
    BOOST_CHECK_EQUAL(*prx, 1);
}


/// Tests forwarding of changed signals of projection
BOOST_AUTO_TEST_CASE(changed) {
    mv::basic_model<int> mdl{1};
    mv::proxy prx{mdl};

    int before_changed_count = 0;
    prx.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(*prx, 1);
    });

    int after_changed_count = 0;
    prx.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(*prx, 2);
    });

    mdl.mut() = 2;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests mutating projection value through proxy
BOOST_AUTO_TEST_CASE(mut) {
    mv::basic_model<int> mdl{1};
    mv::proxy prx{mdl};

    int before_changed_count = 0;
    prx.before_changed().connect([&] { ++before_changed_count; });

    int after_changed_count = 0;
    prx.after_changed().connect([&] { ++after_changed_count; });

    prx.mut() = 2;
    BOOST_CHECK_EQUAL(mdl.get(), 2);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests setting projection
BOOST_AUTO_TEST_CASE(set) {
    mv::basic_model<int> mdl{1};
    mv::basic_model<int> mdl2{2};
    mv::proxy prx{mdl};

    int before_value = 1;
    int after_value = 2;

    int before_changed_count = 0;
    prx.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(*prx, before_value);
    });

    int after_changed_count = 0;
    prx.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(*prx, after_value);
    });

    prx.set(mdl2);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*prx, 2);

    mdl.mut() = 10;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    before_value = 2;
    after_value = 3;
    prx.mut() = 3;
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(mdl.get(), 10);
    BOOST_CHECK_EQUAL(mdl2.get(), 3);
}


/// Tests proxy for read only transform projection
BOOST_AUTO_TEST_CASE(read_only) {
    mv::basic_model<int> mdl{1};
    mv::basic_model<int> mdl2{2};

    auto get_fn = [](int v) { return v * 10; };
    mv::proxy prx{mdl | mv::transform(get_fn)};

    static_assert(mv::observable_as<decltype(prx), int>);
    static_assert(!mv::model<decltype(prx)>);

    BOOST_CHECK_EQUAL(*prx, 10);

    int after_changed_count = 0;
    prx.after_changed().connect([&] { ++after_changed_count; });

    prx.set(mdl2 | mv::transform(get_fn));
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*prx, 20);

    mdl2.mut() = 3;
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(*prx, 30);
}


/// Tests proxy for nullable projection
BOOST_AUTO_TEST_CASE(nullable) {
    using proxy_t = mv::proxy<mv::vector<int>::iterator>;

    static_assert(mv::model_of<proxy_t, int>);
    static_assert(mv::nullable_observable_as<proxy_t, int>);

    mv::vector<int> vec{1, 2, 3};
    proxy_t prx;
    BOOST_CHECK(prx.is_null());

    int after_changed_count = 0;
    prx.after_changed().connect([&] { ++after_changed_count; });

    prx.set(vec.begin() + 1);
    BOOST_CHECK(!prx.is_null());
    BOOST_CHECK_EQUAL(*prx, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    vec.mut(0) = 10;
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    vec.mut(1) = 20;
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(*prx, 20);

    prx.set({});
    BOOST_CHECK(prx.is_null());
    BOOST_CHECK_EQUAL(after_changed_count, 3);

    vec.mut(1) = 30;
    BOOST_CHECK_EQUAL(after_changed_count, 3);
}


/// Tests move constructor
BOOST_AUTO_TEST_CASE(move_ctor) {
    mv::basic_model<int> mdl{1};
    mv::proxy prx{mdl};
    mv::proxy prx2{std::move(prx)};

    int after_changed_count = 0;
    prx2.after_changed().connect([&] { ++after_changed_count; });

    mdl.mut() = 2;
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*prx2, 2);
}


BOOST_AUTO_TEST_SUITE_END()
