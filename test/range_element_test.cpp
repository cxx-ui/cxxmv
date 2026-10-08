// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file range_element_test.cpp
/// Contains unit tests for the element projection of range models.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/projection.hpp>
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/ranges/element.hpp>
#include <cxxmv/ranges/ref_transform.hpp>
#include <cxxmv/ranges/transform.hpp>
#include <cxxmv/signals.hpp>
#include <cxxmv/vector.hpp>
#include <string>
#include <type_traits>


BOOST_AUTO_TEST_SUITE(range_element_test)


/// Tests element projection of vector element
BOOST_AUTO_TEST_CASE(vector_element) {
    mv::vector<int> vec{1, 2, 3};
    auto elem = vec | mv::ranges::element(vec.begin() + 1);

    using elem_t = std::decay_t<decltype(elem)>;
    static_assert(mv::model_projection<elem_t, int>);
    static_assert(mv::nullable_observable_as<elem_t, int>);

    BOOST_CHECK(!elem.is_null());
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(elem.get(), 2);

    int before_changed_count = 0;
    elem.before_changed().connect([&] { ++before_changed_count; });

    int after_changed_count = 0;
    elem.after_changed().connect([&] { ++after_changed_count; });

    vec.mut(0) = 10;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.insert(vec.cbegin(), 0);
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(*elem, 2);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.mut(0) = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*elem, 20);

    elem.mut() = 30;
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(vec[0], 30);
}


/// Tests creating element projection with all forms of element adaptor
BOOST_AUTO_TEST_CASE(adaptor_forms) {
    mv::vector<int> vec{1, 2, 3};
    auto it = vec.begin() + 2;

    auto elem1 = vec | mv::ranges::element(it);
    auto elem2 = mv::ranges::element(vec, it);
    auto elem3 = vec | mv::ranges::all | mv::ranges::element(it);

    static_assert(std::is_same_v<decltype(elem1), decltype(elem2)>);
    static_assert(std::is_same_v<decltype(elem1), decltype(elem3)>);

    BOOST_CHECK_EQUAL(*elem1, 3);
    BOOST_CHECK_EQUAL(*elem2, 3);
    BOOST_CHECK_EQUAL(*elem3, 3);

    elem2.mut() = 30;
    BOOST_CHECK_EQUAL(*elem1, 30);
    BOOST_CHECK_EQUAL(*elem3, 30);
}


/// Tests element projection of transform projection with set function
BOOST_AUTO_TEST_CASE(transform_element) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto name = vec | mv::ranges::transform(get_fn, set_fn)
                    | mv::ranges::element(vec.begin() + 1);

    using name_t = std::decay_t<decltype(name)>;
    static_assert(mv::model_projection<name_t, std::string>);

    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&] { ++after_changed_count; });

    vec.insert(vec.cbegin(), test_user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    name.mut() = std::string{"Alice"};
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(vec[2].first_name(), "Alice");
    BOOST_CHECK_EQUAL(vec[2].last_name(), "Doe");
}


/// Tests element projection of transform projection without set function
BOOST_AUTO_TEST_CASE(transform_element_read_only) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto name = vec | mv::ranges::transform(get_fn) | mv::ranges::element(vec.begin() + 1);

    using name_t = std::decay_t<decltype(name)>;
    static_assert(mv::observable_projection_as<name_t, std::string>);
    static_assert(!mv::model<name_t>);

    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&] { ++after_changed_count; });

    vec.mut(1) = test_user{"Alice", "White"};
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
}


/// Tests element projection of ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform_element) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_first_name = [](auto && u) -> auto & { return u.first_name(); };
    auto name = vec | mv::ranges::ref_transform(get_first_name)
                    | mv::ranges::element(vec.begin() + 1);

    using name_t = std::decay_t<decltype(name)>;
    static_assert(mv::model_projection<name_t, std::string>);
    static_assert(std::is_same_v<decltype(name.get()), const std::string &>);

    BOOST_CHECK_EQUAL(*name, "Jane");

    int before_changed_count = 0;
    name.before_changed().connect([&] { ++before_changed_count; });

    int after_changed_count = 0;
    name.after_changed().connect([&] { ++after_changed_count; });

    name.mut() = "Alice";
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(vec[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(vec[1].last_name(), "Doe");
}


/// Tests element projection constructed with null iterator
BOOST_AUTO_TEST_CASE(null_element) {
    mv::vector<int> vec{1, 2, 3};
    auto elem = mv::ranges::element(vec, mv::vector<int>::iterator{});

    BOOST_CHECK(elem.is_null());

    int before_changed_count = 0;
    elem.before_changed().connect([&] { ++before_changed_count; });

    int after_changed_count = 0;
    elem.after_changed().connect([&] { ++after_changed_count; });

    vec.mut(0) = 10;
    vec.mut(2) = 30;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests that connection to changed signal of element projection outlives projection
BOOST_AUTO_TEST_CASE(element_copy) {
    mv::vector<int> vec{1, 2, 3};

    int before_changed_count = 0;
    int after_changed_count = 0;
    mv::scoped_signal_connection before_con;
    mv::scoped_signal_connection after_con;

    {
        auto elem = vec | mv::ranges::element(vec.begin() + 1);
        auto copy = elem;
        BOOST_CHECK_EQUAL(*copy, 2);
        before_con = copy.before_changed().connect([&] { ++before_changed_count; });
        after_con = copy.after_changed().connect([&] { ++after_changed_count; });
    }

    vec.insert(vec.cbegin(), 0);
    vec.mut(2) = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);

    before_con.disconnect();
    after_con.disconnect();
    vec.mut(2) = 30;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


BOOST_AUTO_TEST_SUITE_END()
