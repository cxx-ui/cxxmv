// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file element_test.cpp
/// Contains unit tests for the range element class.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/ranges/element.hpp>
#include <cxxmv/ranges/ref_projection.hpp>
#include <cxxmv/ranges/ref_transform.hpp>
#include <cxxmv/ranges/transform.hpp>
#include <cxxmv/vector.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


namespace {

/// User with public name fields
struct user {
    std::string first_name;
    std::string last_name;
};

/// Returns reference to first name of user
auto get_first_name = [](auto && u) -> auto & { return u.first_name; };

}


BOOST_AUTO_TEST_SUITE(element_test)


/// Tests construction of vector element model
BOOST_AUTO_TEST_CASE(vector_ctor) {
    using element_t = mv::ranges::element<mv::ranges::all_t<mv::vector<int> &>>;

    static_assert(mv::model_of<element_t, int>);
    static_assert(mv::nullable_observable_as<element_t, int>);

    mv::vector<int> vec{1, 2, 3};

    mv::ranges::element elem{vec, vec.begin() + 1};
    BOOST_CHECK(!elem.is_null());
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.get(), 2);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 1);

    mv::ranges::element null_elem{vec};
    BOOST_CHECK(null_elem.is_null());
}


/// Tests vector element model when elements are inserted into vector
BOOST_AUTO_TEST_CASE(vector_insert) {
    mv::vector<int> vec{1, 2, 3};
    mv::ranges::element elem{vec, vec.begin() + 1};

    int before_changed_count = 0;
    int after_changed_count = 0;
    int after_inserted_count = 0;

    elem.before_changed().connect([&] { ++before_changed_count; });
    elem.after_changed().connect([&] { ++after_changed_count; });

    vec.after_inserted().connect([&](auto && ...) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(*elem, 2);
    });

    vec.insert(vec.cbegin(), 4);
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 2);

    std::vector<int> vals{5, 6};
    vec.insert(vec.cbegin() + 2, vals.begin(), vals.end());
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 4);

    vec.emplace(vec.cbegin(), 7);
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 5);

    vec.insert(vec.cend(), 8);
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 5);

    BOOST_CHECK_EQUAL(after_inserted_count, 4);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests vector element model when elements are erased from vector
BOOST_AUTO_TEST_CASE(vector_erase) {
    mv::vector<int> vec{1, 2, 3, 4, 5};
    mv::ranges::element elem{vec, vec.begin() + 2};

    int before_changed_count = 0;
    int after_changed_count = 0;
    int after_erased_count = 0;

    elem.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK(!elem.is_null());
        BOOST_CHECK_EQUAL(*elem, 3);
    });

    elem.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK(elem.is_null());
    });

    vec.after_erased().connect([&](auto && ...) {
        ++after_erased_count;
        BOOST_CHECK(after_erased_count < 3 || elem.is_null());
        BOOST_CHECK(elem.is_null() || *elem == 3);
    });

    vec.erase(vec.cbegin() + 3, vec.cend());
    BOOST_CHECK_EQUAL(*elem, 3);

    vec.erase(vec.cbegin(), vec.cbegin() + 1);
    BOOST_CHECK_EQUAL(*elem, 3);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 1);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.erase(vec.cbegin(), vec.cbegin() + 2);
    BOOST_CHECK(elem.is_null());
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    vec.insert(vec.cbegin(), 6);
    vec.clear();
    BOOST_CHECK(elem.is_null());
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests vector element model when elements are moved in vector
BOOST_AUTO_TEST_CASE(vector_move) {
    mv::vector<int> vec{0, 1, 2, 3, 4, 5};
    mv::ranges::element elem{vec, vec.begin() + 2};

    int before_changed_count = 0;
    int after_changed_count = 0;
    int after_moved_count = 0;

    elem.before_changed().connect([&] { ++before_changed_count; });
    elem.after_changed().connect([&] { ++after_changed_count; });

    vec.after_moved().connect([&](size_t, size_t, size_t) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(*elem, 2);
    });

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin() + 5);
    BOOST_CHECK_EQUAL(*elem, 2);

    vec.move(vec.cbegin() + 3, vec.cbegin() + 5, vec.cbegin());
    BOOST_CHECK_EQUAL(*elem, 2);

    vec.move(vec.cbegin(), vec.cbegin() + 1, vec.cbegin() + 4);
    BOOST_CHECK_EQUAL(*elem, 2);

    vec.move(vec.cbegin() + 2, vec.cbegin() + 4, vec.cbegin());
    BOOST_CHECK_EQUAL(*elem, 2);

    vec.move(vec.cbegin() + 5, vec.cbegin() + 6, vec.cbegin() + 3);
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.iterator() - vec.begin(), 2);

    BOOST_CHECK_EQUAL(after_moved_count, 5);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests changed signals of vector element model when elements are changed in vector
BOOST_AUTO_TEST_CASE(vector_change) {
    mv::vector<int> vec{1, 2, 3};
    mv::ranges::element elem{vec, vec.begin() + 1};

    int before_changed_count = 0;
    int after_changed_count = 0;
    int vec_after_changed_count = 0;

    elem.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(vec_after_changed_count, 2);
        BOOST_CHECK_EQUAL(*elem, 2);
    });

    elem.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(vec_after_changed_count, 2);
        BOOST_CHECK_EQUAL(*elem, 5);
    });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++vec_after_changed_count;
        BOOST_CHECK(idx != 1 || after_changed_count == 1);
    });

    vec.mut(0) = 4;
    vec.mut(2) = 6;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.mut(1) = 5;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*elem, 5);
}


/// Tests mutating vector element through element model
BOOST_AUTO_TEST_CASE(vector_mut) {
    mv::vector<int> vec{1, 2, 3};
    mv::ranges::element elem{vec, vec.begin() + 1};

    int before_changed_count = 0;
    int after_changed_count = 0;
    int vec_after_changed_count = 0;

    elem.before_changed().connect([&] { ++before_changed_count; });
    elem.after_changed().connect([&] { ++after_changed_count; });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++vec_after_changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
    });

    elem.mut() = 5;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(vec_after_changed_count, 1);

    std::vector<int> expected{1, 5, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
}


/// Tests setting iterator of vector element model
BOOST_AUTO_TEST_CASE(vector_set) {
    mv::vector<int> vec{1, 2, 3};
    mv::ranges::element elem{vec};

    int before_changed_count = 0;
    int after_changed_count = 0;
    int expected = 0;

    elem.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, after_changed_count + 1);
    });

    elem.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK(elem.is_null() || *elem == expected);
    });

    expected = 3;
    elem.set(vec.begin() + 2);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK(!elem.is_null());
    BOOST_CHECK_EQUAL(*elem, 3);

    expected = 1;
    elem.set(vec.begin());
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(*elem, 1);

    vec.insert(vec.cbegin(), 0);
    BOOST_CHECK_EQUAL(*elem, 1);
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);

    expected = 10;
    vec.mut(1) = 10;
    BOOST_CHECK_EQUAL(before_changed_count, 3);
    BOOST_CHECK_EQUAL(after_changed_count, 3);
    BOOST_CHECK_EQUAL(*elem, 10);

    elem.set({});
    BOOST_CHECK_EQUAL(before_changed_count, 4);
    BOOST_CHECK_EQUAL(after_changed_count, 4);
    BOOST_CHECK(elem.is_null());

    vec.mut(1) = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 4);
    BOOST_CHECK_EQUAL(after_changed_count, 4);
}


/// Tests element model of all projection of model reference
BOOST_AUTO_TEST_CASE(all_ref_model) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    using element_t = mv::ranges::element<std::decay_t<decltype(vec2)>>;

    static_assert(mv::model_of<element_t, int>);
    static_assert(mv::nullable_observable_as<element_t, int>);

    element_t elem{vec2, vec.begin() + 1};
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


/// Tests element model of ref projection of all projection of temporary range model
BOOST_AUTO_TEST_CASE(all_temporary_model) {
    auto vec = mv::vector<int>{1, 2, 3} | mv::ranges::all;

    mv::ranges::element elem{mv::ranges::ref_projection{vec}, vec.begin() + 1};
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


/// Tests setting iterator of element model of all projection of model reference
BOOST_AUTO_TEST_CASE(all_ref_model_set) {
    mv::vector<int> vec{1, 2, 3};
    auto vec2 = vec | mv::ranges::all;

    mv::ranges::element elem{vec2};
    BOOST_CHECK(elem.is_null());

    int before_changed_count = 0;
    elem.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    elem.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    elem.set(vec.begin() + 2);
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


/// Tests element model of transform projection
BOOST_AUTO_TEST_CASE(transform) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto names = vec | mv::ranges::transform(get_fn, set_fn);

    using element_t = mv::ranges::element<std::decay_t<decltype(names)>>;

    static_assert(mv::model_of<element_t, std::string>);
    static_assert(mv::nullable_observable_as<element_t, std::string>);

    element_t name{names, vec.begin() + 1};
    BOOST_CHECK(!name.is_null());
    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    vec.insert(vec.cbegin(), test_user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    name.mut() = std::string{"Alice"};
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(vec[0].first_name(), "Alice");
    BOOST_CHECK_EQUAL(vec[0].last_name(), "Doe");

    vec.erase(vec.cbegin(), vec.cbegin() + 1);
    BOOST_CHECK(name.is_null());
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
}


/// Tests setting iterator of element model of transform projection
BOOST_AUTO_TEST_CASE(transform_set) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    mv::ranges::element name{names};
    BOOST_CHECK(name.is_null());

    int before_changed_count = 0;
    name.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    name.set(vec.begin() + 2);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Bob");

    vec.insert(vec.cbegin(), test_user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Bob");
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    name.set({});
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK(name.is_null());
}


/// Tests element model of transform projection without set function
BOOST_AUTO_TEST_CASE(transform_read_only) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    using element_t = mv::ranges::element<std::decay_t<decltype(names)>>;

    static_assert(mv::nullable_observable_as<element_t, std::string>);
    static_assert(!mv::model<element_t>);

    element_t name{names, vec.begin() + 1};
    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    vec.mut(0) = test_user{"Tom", "Green"};
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.mut(1) = test_user{"Alice", "White"};
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
}


/// Tests element model of ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    using element_t = mv::ranges::element<std::decay_t<decltype(names)>>;

    static_assert(mv::model_of<element_t, std::string>);
    static_assert(mv::nullable_observable_as<element_t, std::string>);
    static_assert(std::is_same_v<decltype(std::declval<element_t>().get()), const std::string &>);

    element_t name{names, vec.begin() + 1};
    BOOST_CHECK(!name.is_null());
    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    vec.insert(vec.cbegin(), user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    name.mut() = "Alice";
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(vec[0].first_name, "Alice");
    BOOST_CHECK_EQUAL(vec[0].last_name, "Doe");

    vec.erase(vec.cbegin(), vec.cbegin() + 1);
    BOOST_CHECK(name.is_null());
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
}


/// Tests setting iterator of element model of ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform_set) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    mv::ranges::element name{names};
    BOOST_CHECK(name.is_null());

    int before_changed_count = 0;
    name.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    name.set(vec.begin() + 2);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Bob");

    vec.insert(vec.cbegin(), user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Bob");
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    name.set({});
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK(name.is_null());
}


/// Tests element model of ref projection of ref transform projection of temporary vector
BOOST_AUTO_TEST_CASE(ref_transform_temporary_base) {
    auto names = mv::vector<user>{{"John", "Smith"}, {"Jane", "Doe"}}
               | mv::ranges::ref_transform(get_first_name);

    mv::ranges::element name{mv::ranges::ref_projection{names}, names.begin() + 1};
    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&before_changed_count] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&after_changed_count] { ++after_changed_count; });

    names.mut(0) = "Tom";
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    name.mut() = "Alice";
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(names.cbegin()[1], "Alice");
}


BOOST_AUTO_TEST_SUITE_END()
