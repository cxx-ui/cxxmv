// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all_range_test.cpp
/// Contains unit tests for the all projection of range models.

#include <boost/test/unit_test.hpp>
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/vector.hpp>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>


BOOST_AUTO_TEST_SUITE(all_range_test)


/// Tests changing range model via all projection of model reference
BOOST_AUTO_TEST_CASE(all_ref_model) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    using all_t = std::decay_t<decltype(vec2)>;

    static_assert(mv::ranges::model<all_t, int>);
    static_assert(mv::ranges::borrowed_observable<all_t>);
    static_assert(std::move_constructible<all_t>);
    static_assert(std::copy_constructible<all_t>);

    std::vector<int> expected{1, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(std::as_const(vec2).begin(), std::as_const(vec2).end(),
                                  expected.begin(), expected.end());

    bool changed_called = false;
    vec2.after_changed.connect([&changed_called, &vec, &vec2](size_t idx) {
        changed_called = true;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(vec[1], 20);
        BOOST_CHECK_EQUAL(std::as_const(vec2).begin()[1], 20);
    });

    vec2.mut(1) = 20;

    BOOST_CHECK_EQUAL(vec[1], 20);
    BOOST_CHECK_EQUAL(std::as_const(vec2).begin()[1], 20);

    BOOST_CHECK(changed_called);
}


/// Tests connecting to all projection via temporary ref object
BOOST_AUTO_TEST_CASE(all_ref_borrowed) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    bool changed_called = false;
    (vec | mv::ranges::all).after_changed.connect([&changed_called, &vec, &vec2](size_t idx) {
        changed_called = true;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(vec[1], 20);
        BOOST_CHECK_EQUAL(std::as_const(vec2).begin()[1], 20);
    });

    (vec | mv::ranges::all).mut(1) = 20;

    BOOST_CHECK_EQUAL(vec[1], 20);
    BOOST_CHECK_EQUAL(std::as_const(vec2).begin()[1], 20);

    BOOST_CHECK(changed_called);

    // checking that std::ranges::begin can be called on temporary projection
    auto it = std::ranges::begin(vec | mv::ranges::all);
    BOOST_CHECK(it == vec.begin());
    BOOST_CHECK_EQUAL(static_cast<int>(*it), 1);
}


/// Tests all projection of temporary range model
BOOST_AUTO_TEST_CASE(all_temporary_model) {
    auto vec = mv::vector<int>{1, 2, 3} | mv::ranges::all;

    using all_t = std::decay_t<decltype(vec)>;

    static_assert(mv::ranges::model<all_t, int>);
    static_assert(!mv::ranges::borrowed_observable<all_t>);
    static_assert(std::move_constructible<all_t>);
    static_assert(!std::copy_constructible<all_t>);

    std::vector<int> expected{1, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(std::as_const(vec).begin(), std::as_const(vec).end(),
                                  expected.begin(), expected.end());

    bool changed_called = false;
    vec.after_changed.connect([&changed_called, &vec](size_t idx) {
        changed_called = true;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(std::as_const(vec).begin()[1], 20);
    });

    vec.mut(1) = 20;

    BOOST_CHECK_EQUAL(std::as_const(vec).begin()[1], 20);

    BOOST_CHECK(changed_called);
}


BOOST_AUTO_TEST_SUITE_END()
