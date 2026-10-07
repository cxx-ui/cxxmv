// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all_range_test.cpp
/// Contains unit tests for the all projection of range models.

#include <boost/test/unit_test.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/ranges/element_model.hpp>
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
    vec2.after_changed().connect([&changed_called, &vec, &vec2](size_t idx) {
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
    (vec | mv::ranges::all).after_changed().connect([&changed_called, &vec, &vec2](size_t idx) {
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
    vec.after_changed().connect([&changed_called, &vec](size_t idx) {
        changed_called = true;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(std::as_const(vec).begin()[1], 20);
    });

    vec.mut(1) = 20;

    BOOST_CHECK_EQUAL(std::as_const(vec).begin()[1], 20);

    BOOST_CHECK(changed_called);
}


/// Tests element model of all projection of model reference
BOOST_AUTO_TEST_CASE(all_ref_model_element) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    using all_t = std::decay_t<decltype(vec2)>;
    using element_t = mv::ranges::element_model<all_t>;

    static_assert(mv::model_of<element_t, int>);
    static_assert(mv::nullable_observable_as<element_t, int>);

    element_t elem{vec.handle_at(1)};
    BOOST_CHECK_EQUAL(*elem, 2);

    int before_changed_count = 0;
    elem.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    elem.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    vec.insert(vec.cbegin(), 0);
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    elem.mut() = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(vec[2], 20);
    BOOST_CHECK_EQUAL(std::as_const(vec2).begin()[2], 20);

    vec.erase(vec.cbegin() + 2, vec.cbegin() + 3);
    BOOST_CHECK(elem.is_null());
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
}


/// Tests element model of all projection of temporary range model
BOOST_AUTO_TEST_CASE(all_temporary_model_element) {
    auto vec = mv::vector<int>{1, 2, 3} | mv::ranges::all;

    using all_t = std::decay_t<decltype(vec)>;
    using element_t = mv::ranges::element_model<all_t>;

    static_assert(mv::model_of<element_t, int>);
    static_assert(mv::nullable_observable_as<element_t, int>);

    element_t elem{vec.handle_at(1)};
    BOOST_CHECK_EQUAL(*elem, 2);

    int before_changed_count = 0;
    elem.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    elem.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    vec.mut(0) = 10;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.mut(1) = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*elem, 20);

    elem.mut() = 30;
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(std::as_const(vec).begin()[1], 30);
}


/// Tests setting handle of element model of all projection of model reference
BOOST_AUTO_TEST_CASE(all_ref_model_element_set) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    mv::ranges::element_model<std::decay_t<decltype(vec2)>> elem{};
    BOOST_CHECK(elem.is_null());

    int before_changed_count = 0;
    elem.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    elem.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    elem.set(vec.handle_at(2));
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*elem, 3);

    vec.insert(vec.cbegin(), 0);
    BOOST_CHECK_EQUAL(*elem, 3);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    elem.set({});
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK(elem.is_null());
}


/// Tests reading and mutating elements by index and handle via all projection
/// of model reference
BOOST_AUTO_TEST_CASE(all_ref_model_handle) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    using all_t = std::decay_t<decltype(vec2)>;
    static_assert(mv::ranges::observable_with_handle<all_t>);
    static_assert(mv::ranges::model_with_handle<all_t>);

    BOOST_CHECK_EQUAL(vec2.get(1), 2);

    auto h = vec2.handle_at(1);
    BOOST_CHECK_EQUAL(vec2.get(h), 2);

    vec.insert(vec.cbegin(), 0);
    BOOST_CHECK_EQUAL(vec2.get(h), 2);
    BOOST_CHECK_EQUAL(vec2.get(2), 2);

    vec2.mut(h) = 20;
    BOOST_CHECK_EQUAL(vec[2], 20);
    BOOST_CHECK_EQUAL(vec2.get(h), 20);
}


/// Tests reading and mutating elements by index and handle via all projection
/// of temporary range model
BOOST_AUTO_TEST_CASE(all_temporary_model_handle) {
    auto vec = mv::vector<int>{1, 2, 3} | mv::ranges::all;

    using all_t = std::decay_t<decltype(vec)>;
    static_assert(mv::ranges::observable_with_handle<all_t>);
    static_assert(mv::ranges::model_with_handle<all_t>);

    BOOST_CHECK_EQUAL(vec.get(1), 2);

    auto h = vec.handle_at(1);
    BOOST_CHECK_EQUAL(vec.get(h), 2);

    vec.mut(0) = 10;
    vec.mut(h) = 20;
    BOOST_CHECK_EQUAL(vec.get(0), 10);
    BOOST_CHECK_EQUAL(vec.get(1), 20);
    BOOST_CHECK_EQUAL(vec.get(h), 20);
}


BOOST_AUTO_TEST_SUITE_END()
