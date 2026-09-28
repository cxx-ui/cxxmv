// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file vector_test.cpp
/// Contains unit tests for the vector model.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/vector.hpp>
#include <iterator>
#include <memory>
#include <ranges>
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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_inserted.connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx], 10);

        // vector is already modified
        std::vector<int> expected{1, 10, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 3);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_inserted.connect([&](size_t idx, size_t count) {
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

    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 3);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_inserted.connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 3);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx], 10);

        // vector is already modified
        std::vector<int> expected{1, 2, 3, 10};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        BOOST_CHECK_EQUAL(vec.size(), 2);
    });

    vec.after_inserted.connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx].first_name(), "Bob");
        BOOST_CHECK_EQUAL(vec[idx].last_name(), "Brown");

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

    auto it = vec.emplace(vec.begin() + 1, "Bob", "Brown");

    BOOST_CHECK_EQUAL(std::distance(vec.cbegin(), it), 1);

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 2);
        BOOST_CHECK_EQUAL(count, 1);

        // vector is not modified yet
        BOOST_CHECK_EQUAL(vec.size(), 2);
    });

    vec.after_inserted.connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 2);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx].first_name(), "Bob");
        BOOST_CHECK_EQUAL(vec[idx].last_name(), "Brown");

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

    auto it = vec.emplace_back("Bob", "Brown");

    BOOST_CHECK_EQUAL(std::distance(vec.cbegin(), it), 2);

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_erased.connect([&](size_t idx, size_t count) {
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

    vec.after_erased.connect([&](size_t idx, size_t count) {
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

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_erased.connect([&](size_t idx, size_t count) {
        ++before_erased_count;
        BOOST_CHECK_EQUAL(after_erased_count, 0);
        BOOST_CHECK_EQUAL(idx, 0);
        BOOST_CHECK_EQUAL(count, 3);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    });

    vec.after_erased.connect([&](size_t idx, size_t count) {
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(idx, 0);
        BOOST_CHECK_EQUAL(count, 3);

        // vector is already empty
        BOOST_CHECK(vec.empty());
    });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_changed.connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(vec[idx], 2);
    });

    vec.after_changed.connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(vec[idx], 10);
    });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });

    auto it = vec.begin() + 1;
    *it = 10;

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


/// Tests changing element via non-const at method
BOOST_AUTO_TEST_CASE(at_assign) {
    mv::vector<int> vec{1, 2, 3};

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_changed.connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(vec[idx], 2);
    });

    vec.after_changed.connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(vec[idx], 10);
    });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });

    vec.at(1) = 10;

    BOOST_CHECK_EQUAL(vec.at(1), 10);

    std::vector<int> expected{1, 10, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    BOOST_CHECK_THROW(vec.at(3) = 20, std::out_of_range);

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    *vec.begin() = std::make_unique<test_user>("Ann", "Black");
    vec.at(1) = std::make_unique<test_user>("Sam", "Grey");
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

    vec.before_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++before_moved_count;
        BOOST_CHECK_EQUAL(after_moved_count, 0);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3, 4, 5};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.after_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);

        // vector is already modified
        std::vector<int> expected{1, 4, 5, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++before_moved_count;
        BOOST_CHECK_EQUAL(after_moved_count, 0);
        BOOST_CHECK_EQUAL(first_idx, 3);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 1);

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3, 4, 5};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.after_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 3);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 1);

        // vector is already modified
        std::vector<int> expected{1, 4, 5, 2, 3};
        BOOST_CHECK_EQUAL_COLLECTIONS(vec.cbegin(), vec.cend(), expected.begin(), expected.end());
    });

    vec.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    vec.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    vec.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    vec.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    vec.before_changed.connect([&](size_t) { ++before_changed_count; });
    vec.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    vec.before_moved.connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved.connect([&](size_t, size_t, size_t) { ++after_moved_count; });

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


BOOST_AUTO_TEST_SUITE_END()
