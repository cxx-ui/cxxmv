// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file vector_test.cpp
/// Contains unit tests for the vector model.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/signals.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/vector.hpp>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>


BOOST_AUTO_TEST_SUITE(vector_test)


/// Tests default constructor
BOOST_AUTO_TEST_CASE(ctor_default) {
    mv::vector<int> vec;

    BOOST_CHECK(vec.empty());
    BOOST_CHECK_EQUAL(vec.size(), 0);
    BOOST_CHECK(vec.begin() == vec.end());

    BOOST_CHECK(std::ranges::empty(vec));
    BOOST_CHECK_EQUAL(std::ranges::size(vec), 0);
    BOOST_CHECK(std::ranges::begin(vec) == std::ranges::end(vec));
}


/// Tests construction from initializer list
BOOST_AUTO_TEST_CASE(ctor_initializer_list) {
    mv::vector<int> vec{1, 2, 3};

    BOOST_CHECK(!vec.empty());
    BOOST_CHECK_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(std::ranges::size(vec), 3);

    std::vector<int> expected{1, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(vec[0], 1);
    BOOST_CHECK_EQUAL(vec[1], 2);
    BOOST_CHECK_EQUAL(vec[2], 3);
}


/// Tests inserting single element
BOOST_AUTO_TEST_CASE(insert_single) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx], 10);

        // vector is already modified
        std::vector<int> expected{1, 10, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.insert(vec.begin() + 1, 10);

    std::vector<int> expected{1, 10, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests inserting range of elements
BOOST_AUTO_TEST_CASE(insert_range) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 3);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 3);

        // idx is the index of the first inserted element
        std::vector<int> inserted{10, 20, 30};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin() + idx, vec.cbegin() + idx + count,
                                      inserted.begin(), inserted.end());

        // vector is already modified
        std::vector<int> expected{1, 10, 20, 30, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    std::vector<int> vals{10, 20, 30};
    vec.insert(vec.begin() + 1, vals.begin(), vals.end());

    std::vector<int> expected{1, 10, 20, 30, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests appending element to the end of vector
BOOST_AUTO_TEST_CASE(push_back) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 3);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 3);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx], 10);

        // vector is already modified
        std::vector<int> expected{1, 2, 3, 10};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.push_back(10);

    std::vector<int> expected{1, 2, 3, 10};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests constructing element in place at specified position
BOOST_AUTO_TEST_CASE(emplace) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        BOOST_CHECK_EQUAL(vec.size(), 2);
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx].first_name(), "Bob");
        BOOST_CHECK_EQUAL(vec[idx].last_name(), "Brown");

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    auto it = vec.emplace(vec.begin() + 1, "Bob", "Brown");

    BOOST_CHECK_EQUAL(std::distance(vec.begin(), it), 1);

    BOOST_REQUIRE_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(vec[0].first_name(), "John");
    BOOST_CHECK_EQUAL(vec[0].last_name(), "Smith");
    BOOST_CHECK_EQUAL(vec[1].first_name(), "Bob");
    BOOST_CHECK_EQUAL(vec[1].last_name(), "Brown");
    BOOST_CHECK_EQUAL(vec[2].first_name(), "Jane");
    BOOST_CHECK_EQUAL(vec[2].last_name(), "Doe");

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests constructing element in place at the end of vector
BOOST_AUTO_TEST_CASE(emplace_back) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 2);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        BOOST_CHECK_EQUAL(vec.size(), 2);
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 2);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx].first_name(), "Bob");
        BOOST_CHECK_EQUAL(vec[idx].last_name(), "Brown");

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    auto it = vec.emplace_back("Bob", "Brown");

    BOOST_CHECK_EQUAL(std::distance(vec.begin(), it), 2);

    BOOST_REQUIRE_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(vec[0].first_name(), "John");
    BOOST_CHECK_EQUAL(vec[0].last_name(), "Smith");
    BOOST_CHECK_EQUAL(vec[1].first_name(), "Jane");
    BOOST_CHECK_EQUAL(vec[1].last_name(), "Doe");
    BOOST_CHECK_EQUAL(vec[2].first_name(), "Bob");
    BOOST_CHECK_EQUAL(vec[2].last_name(), "Brown");

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests erasing range of elements
BOOST_AUTO_TEST_CASE(erase) {
    mv::vector<int> vec{1, 2, 3, 4, 5};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_erased().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++before_erased_count;
        BOOST_CHECK_EQUAL(after_erased_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the first element being erased
        std::vector<int> erased{2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin() + idx, vec.cbegin() + idx + count,
                                      erased.begin(), erased.end());

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3, 4, 5};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_erased().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the element following erased ones
        BOOST_CHECK_EQUAL(vec[idx], 4);

        // vector is already modified
        std::vector<int> expected{1, 4, 5};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.erase(vec.begin() + 1, vec.begin() + 3);

    std::vector<int> expected{1, 4, 5};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 1);
    BOOST_CHECK_EQUAL(after_erased_count, 1);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests erasing all elements
BOOST_AUTO_TEST_CASE(clear) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_erased().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++before_erased_count;
        BOOST_CHECK_EQUAL(after_erased_count, 0);
        BOOST_CHECK_EQUAL(idx, 0);
        BOOST_CHECK_EQUAL(count, 3);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_erased().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(idx, 0);
        BOOST_CHECK_EQUAL(count, 3);

        // vector is already empty
        BOOST_CHECK(vec.empty());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.clear();

    BOOST_CHECK(vec.empty());
    BOOST_CHECK_EQUAL(vec.size(), 0);
    BOOST_CHECK(vec.begin() == vec.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 1);
    BOOST_CHECK_EQUAL(after_erased_count, 1);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests changing element via iterator
BOOST_AUTO_TEST_CASE(iterator_assign) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(vec[idx], 2);
    });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(vec[idx], 10);
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });

    auto it = vec.begin() + 1;
    it.mut() = 10;

    BOOST_CHECK_EQUAL(*it, 10);

    std::vector<int> expected{1, 10, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests changing element via mut method
BOOST_AUTO_TEST_CASE(mut_assign) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(vec[idx], 2);
    });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(vec[idx], 10);
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });

    vec.mut(1) = 10;

    BOOST_CHECK_EQUAL(vec.at(1), 10);

    std::vector<int> expected{1, 10, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests that inserting empty range doesn't emit signals
BOOST_AUTO_TEST_CASE(insert_empty_range) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    std::vector<int> vals;
    vec.insert(vec.begin() + 1, vals.begin(), vals.end());

    std::vector<int> expected{1, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests that erasing empty range doesn't emit signals
BOOST_AUTO_TEST_CASE(erase_empty_range) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.erase(vec.begin() + 1, vec.begin() + 1);

    std::vector<int> expected{1, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests that clearing empty vector doesn't emit signals
BOOST_AUTO_TEST_CASE(clear_empty) {
    mv::vector<int> vec;

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.clear();

    BOOST_CHECK(vec.empty());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests that all operations compile for vector of unique pointers
BOOST_AUTO_TEST_CASE(unique_ptr_elements) {
    mv::vector<std::unique_ptr<test_user>> vec;

    vec.push_back(std::make_unique<test_user>("John", "Smith"));
    vec.insert(vec.begin(), std::make_unique<test_user>("Jane", "Doe"));
    vec.emplace(vec.begin() + 1, std::make_unique<test_user>("Bob", "Brown"));
    vec.emplace_back(std::make_unique<test_user>("Alice", "White"));

    std::vector<std::unique_ptr<test_user>> users;
    users.push_back(std::make_unique<test_user>("Tom", "Green"));
    vec.insert(vec.end(), std::make_move_iterator(users.begin()), std::make_move_iterator(users.end()));

    BOOST_REQUIRE_EQUAL(vec.size(), 5);
    BOOST_CHECK(!vec.empty());
    BOOST_CHECK_EQUAL(std::distance(vec.cbegin(), vec.cend()), 5);
    BOOST_CHECK_EQUAL(std::as_const(vec)[0]->first_name(), "Jane");
    BOOST_CHECK_EQUAL(std::as_const(vec).at(1)->first_name(), "Bob");

    vec.begin().mut() = std::make_unique<test_user>("Ann", "Black");
    vec.mut(1) = std::make_unique<test_user>("Sam", "Grey");
    BOOST_CHECK_EQUAL(std::as_const(vec)[0]->first_name(), "Ann");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1]->first_name(), "Sam");

    vec.erase(vec.begin(), vec.begin() + 2);
    BOOST_CHECK_EQUAL(vec.size(), 3);

    vec.clear();
    BOOST_CHECK(vec.empty());
}


/// Tests moving elements to position after them
BOOST_AUTO_TEST_CASE(move_forward) {
    mv::vector<int> vec{1, 2, 3, 4, 5};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++before_moved_count;
        BOOST_CHECK_EQUAL(after_moved_count, 0);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3, 4, 5};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.after_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);

        // vector is already modified
        std::vector<int> expected{1, 4, 5, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cend());

    std::vector<int> expected{1, 4, 5, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 1);
    BOOST_CHECK_EQUAL(after_moved_count, 1);
}


/// Tests moving elements to position before them
BOOST_AUTO_TEST_CASE(move_backward) {
    mv::vector<int> vec{1, 2, 3, 4, 5};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++before_moved_count;
        BOOST_CHECK_EQUAL(after_moved_count, 0);
        BOOST_CHECK_EQUAL(first_idx, 3);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 1);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3, 4, 5};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.after_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 3);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 1);

        // vector is already modified
        std::vector<int> expected{1, 4, 5, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.move(vec.cbegin() + 3, vec.cend(), vec.cbegin() + 1);

    std::vector<int> expected{1, 4, 5, 2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 1);
    BOOST_CHECK_EQUAL(after_moved_count, 1);
}


/// Tests that moves which don't change vector don't emit signals
BOOST_AUTO_TEST_CASE(move_noop) {
    mv::vector<int> vec{1, 2, 3, 4, 5};

    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    // empty range
    vec.move(vec.cbegin() + 1, vec.cbegin() + 1, vec.cend());

    // destination is the first moved element
    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cbegin() + 1);

    // destination is the element after moved ones
    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cbegin() + 3);

    std::vector<int> expected{1, 2, 3, 4, 5};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests reading and mutating vector elements by iterator
BOOST_AUTO_TEST_CASE(iterator_get_mut) {
    mv::vector<int> vec{1, 2, 3};
    auto it = vec.begin() + 1;
    BOOST_CHECK_EQUAL(vec.get(it), 2);

    int after_changed_count = 0;
    vec.after_changed().connect([&](const auto & changed) {
        size_t idx = changed - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(idx, 2);
    });

    vec.insert(vec.cbegin(), 0);
    BOOST_CHECK_EQUAL(it - vec.begin(), 2);
    BOOST_CHECK_EQUAL(vec.get(it), 2);

    vec.mut(it) = 20;
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(vec[2], 20);
    BOOST_CHECK_EQUAL(vec.get(it), 20);
}


/// Tests that iterators keep pointing to the same elements after structural changes
BOOST_AUTO_TEST_CASE(iterator_stability) {
    mv::vector<int> vec{0, 1, 2, 3, 4, 5};
    auto it = vec.begin() + 2;
    auto cit = vec.cbegin() + 2;

    vec.insert(vec.cbegin(), 10);
    BOOST_CHECK(it == vec.begin() + 3);
    BOOST_CHECK(cit == vec.cbegin() + 3);

    std::vector<int> vals{20, 30};
    vec.insert(vec.cbegin() + 1, vals.begin(), vals.end());
    BOOST_CHECK(it == vec.begin() + 5);

    vec.erase(vec.cbegin(), vec.cbegin() + 4);
    BOOST_CHECK(it == vec.begin() + 1);

    vec.move(vec.cbegin() + 1, vec.cbegin() + 2, vec.cend());
    BOOST_CHECK(it == vec.end() - 1);

    vec.move(vec.cend() - 1, vec.cend(), vec.cbegin());
    BOOST_CHECK(it == vec.begin());
    BOOST_CHECK(cit == vec.cbegin());

    BOOST_CHECK_EQUAL(*it, 2);
    BOOST_CHECK_EQUAL(*cit, 2);
}


/// Tests iterator as model of vector element
BOOST_AUTO_TEST_CASE(iterator_model) {
    static_assert(mv::model_of<mv::vector<int>::iterator, int>);
    static_assert(mv::nullable_observable_as<mv::vector<int>::iterator, int>);

    mv::vector<int> vec{1, 2, 3};
    auto it = vec.begin() + 1;

    BOOST_CHECK(!it.is_null());
    BOOST_CHECK_EQUAL(it.get(), 2);
    BOOST_CHECK_EQUAL(mv::get(it), 2);

    int before_changed_count = 0;
    int after_changed_count = 0;

    mv::scoped_signal_connection before_con = it.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count + 1, before_changed_count);
    });

    mv::scoped_signal_connection after_con = it.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, before_changed_count);
    });

    // changes of other elements are not reported
    vec.mut(0) = 10;
    vec.mut(2) = 30;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    // iterator keeps pointing to element after structural changes
    vec.insert(vec.cbegin(), 0);
    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(it.get(), 2);

    vec.mut(0) = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(it.get(), 20);

    it.mut() = 21;
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(vec[0], 21);
}


/// Tests values of element in before and after changed signals of iterator
BOOST_AUTO_TEST_CASE(iterator_model_signal_values) {
    mv::vector<int> vec{1, 2, 3};
    auto it = vec.begin() + 1;

    int before_changed_count = 0;
    int after_changed_count = 0;

    mv::scoped_signal_connection before_con = it.before_changed().connect([&] {
        ++before_changed_count;

        // element is not modified yet
        BOOST_CHECK_EQUAL(it.get(), 2);
    });

    mv::scoped_signal_connection after_con = it.after_changed().connect([&] {
        ++after_changed_count;

        // element is already modified
        BOOST_CHECK_EQUAL(it.get(), 20);
    });

    it.mut() = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests const iterator as observable of vector element
BOOST_AUTO_TEST_CASE(const_iterator_observable) {
    static_assert(mv::nullable_observable_as<mv::vector<int>::const_iterator, int>);
    static_assert(!mv::model<mv::vector<int>::const_iterator>);

    mv::vector<int> vec{1, 2, 3};
    auto cit = vec.cbegin() + 1;

    BOOST_CHECK(!cit.is_null());
    BOOST_CHECK_EQUAL(cit.get(), 2);

    int before_changed_count = 0;
    int after_changed_count = 0;

    mv::scoped_signal_connection before_con = cit.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(cit.get(), 2);
    });

    mv::scoped_signal_connection after_con = cit.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(cit.get(), 20);
    });

    vec.mut(0) = 10;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    // changing element via mutable iterator is reported to const iterator
    (vec.begin() + 1).mut() = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests disconnecting from signals of iterator
BOOST_AUTO_TEST_CASE(iterator_model_disconnect) {
    mv::vector<int> vec{1, 2, 3};
    auto it = vec.begin() + 1;

    int after_changed_count = 0;

    {
        mv::scoped_signal_connection con = it.after_changed().connect([&] {
            ++after_changed_count;
        });

        it.mut() = 20;
        BOOST_CHECK_EQUAL(after_changed_count, 1);
    }

    it.mut() = 30;
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests null iterators
BOOST_AUTO_TEST_CASE(iterator_null) {
    mv::vector<int> vec{1, 2, 3};

    BOOST_CHECK(mv::vector<int>::iterator{}.is_null());
    BOOST_CHECK(mv::vector<int>::const_iterator{}.is_null());
    BOOST_CHECK(vec.end().is_null());
    BOOST_CHECK(vec.cend().is_null());
    BOOST_CHECK(mv::is_null(mv::vector<int>::iterator{}));
}


/// Tests transform projection of iterator
BOOST_AUTO_TEST_CASE(iterator_transform) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}};
    auto it = vec.begin() + 1;

    auto name = it | mv::transform(
        [](const test_user & u) { return u.first_name(); },
        [](test_user & u, const std::string & val) { u.set_first_name(val); });

    static_assert(mv::model_of<decltype(name), std::string>);
    BOOST_CHECK_EQUAL(name.get(), "Jane");

    int after_changed_count = 0;
    mv::scoped_signal_connection con = name.after_changed().connect([&] {
        ++after_changed_count;
    });

    vec.mut(0) = test_user{"Tom", "Green"};
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    name.mut() = std::string{"Alice"};
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(vec[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(vec[1].last_name(), "Doe");
}


/// Tests transform projection with temporary iterator
BOOST_AUTO_TEST_CASE(iterator_transform_temporary) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}};

    auto make_name = [&vec] {
        auto it = vec.begin() + 1;
        return it | mv::transform(
            [](const test_user & u) { return u.first_name(); },
            [](test_user & u, const std::string & val) { u.set_first_name(val); });
    };

    auto name = make_name();
    BOOST_CHECK_EQUAL(name.get(), "Jane");

    int after_changed_count = 0;
    mv::scoped_signal_connection con = name.after_changed().connect([&] {
        ++after_changed_count;
    });

    vec.mut(1) = test_user{"Kate", "Black"};
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(name.get(), "Kate");

    name.mut() = std::string{"Alice"};
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(vec[1].first_name(), "Alice");
}


/// Tests transform projection with temporary const iterator
BOOST_AUTO_TEST_CASE(const_iterator_transform_temporary) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}};

    auto make_name = [&vec] {
        auto cit = vec.cbegin() + 1;
        return cit | mv::transform([](const test_user & u) { return u.first_name(); });
    };

    auto name = make_name();
    BOOST_CHECK_EQUAL(name.get(), "Jane");

    int after_changed_count = 0;
    mv::scoped_signal_connection con = name.after_changed().connect([&] {
        ++after_changed_count;
    });

    vec.mut(1) = test_user{"Kate", "Black"};
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(name.get(), "Kate");
}


BOOST_AUTO_TEST_SUITE_END()
