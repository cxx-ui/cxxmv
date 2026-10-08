// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file model_vector_test.cpp
/// Contains unit tests for the model_vector model.

#include <boost/test/unit_test.hpp>
#include <cxxmv/basic_model.hpp>
#include <cxxmv/model_vector.hpp>
#include <cxxmv/ranges/element.hpp>
#include <cxxmv/ranges/model.hpp>
#include <cxxmv/ranges/observable.hpp>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <vector>


namespace {


using int_model = mv::basic_model<int>;
using int_vector = mv::model_vector<int_model>;


/// Appends objects with specified values to vector
void fill(int_vector & vec, std::initializer_list<int> vals) {
    for (auto val : vals) {
        vec.emplace_back(val);
    }
}


/// Returns values of objects in vector
std::vector<int> values(const int_vector & vec) {
    std::vector<int> res;
    for (auto & obj : vec) {
        res.push_back(obj.get());
    }
    return res;
}


}


BOOST_AUTO_TEST_SUITE(model_vector_test)


/// Tests default constructor
BOOST_AUTO_TEST_CASE(ctor_default) {
    static_assert(mv::ranges::observable<int_vector>);
    static_assert(mv::ranges::observable_with_move<int_vector>);

    int_vector vec;

    BOOST_CHECK(vec.empty());
    BOOST_CHECK_EQUAL(vec.size(), 0);
    BOOST_CHECK(vec.begin() == vec.end());

    BOOST_CHECK(std::ranges::empty(vec));
    BOOST_CHECK_EQUAL(std::ranges::size(vec), 0);
    BOOST_CHECK(std::ranges::begin(vec) == std::ranges::end(vec));
}


/// Tests inserting object owned by unique pointer
BOOST_AUTO_TEST_CASE(insert_unique_ptr) {
    int_vector vec;
    fill(vec, {1, 2, 3});

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
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx].get(), 10);

        // vector is already modified
        std::vector<int> expected{1, 10, 2, 3};
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    auto obj = std::make_unique<int_model>(10);
    auto ptr = obj.get();
    vec.insert(vec.cbegin() + 1, std::move(obj));

    std::vector<int> expected{1, 10, 2, 3};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    // object is not copied
    BOOST_CHECK_EQUAL(&vec[1], ptr);

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests appending object owned by unique pointer to the end of vector
BOOST_AUTO_TEST_CASE(push_back_unique_ptr) {
    int_vector vec;
    fill(vec, {1, 2, 3});

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
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.after_inserted().connect([&](auto && first, auto && last) {
        size_t idx = first - vec.cbegin();
        size_t count = last - first;
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 3);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(vec[idx].get(), 10);

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 4);
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.push_back(std::make_unique<int_model>(10));

    std::vector<int> expected{1, 2, 3, 10};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests constructing object in place at specified position
BOOST_AUTO_TEST_CASE(emplace) {
    int_vector vec;
    fill(vec, {1, 2});

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
        BOOST_CHECK_EQUAL(vec[idx].get(), 10);

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    auto it = vec.emplace(vec.cbegin() + 1, 10);

    BOOST_CHECK_EQUAL(std::distance(vec.cbegin(), it), 1);

    std::vector<int> expected{1, 10, 2};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests constructing object in place at the end of vector
BOOST_AUTO_TEST_CASE(emplace_back) {
    int_vector vec;
    fill(vec, {1, 2});

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
        BOOST_CHECK_EQUAL(vec[idx].get(), 10);

        // vector is already modified
        BOOST_CHECK_EQUAL(vec.size(), 3);
    });

    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    auto it = vec.emplace_back(10);

    BOOST_CHECK_EQUAL(std::distance(vec.cbegin(), it), 2);

    std::vector<int> expected{1, 2, 10};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests erasing range of objects
BOOST_AUTO_TEST_CASE(erase) {
    int_vector vec;
    fill(vec, {1, 2, 3, 4, 5});

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

        // vector is not modified yet
        std::vector<int> expected{1, 2, 3, 4, 5};
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.after_erased().connect([&](auto && pos, size_t count) {
        size_t idx = pos - vec.cbegin();
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the object following erased ones
        BOOST_CHECK_EQUAL(vec[idx].get(), 4);

        // vector is already modified
        std::vector<int> expected{1, 4, 5};
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.erase(vec.cbegin() + 1, vec.cbegin() + 3);

    std::vector<int> expected{1, 4, 5};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 1);
    BOOST_CHECK_EQUAL(after_erased_count, 1);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests erasing all objects
BOOST_AUTO_TEST_CASE(clear) {
    int_vector vec;
    fill(vec, {1, 2, 3});

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
        BOOST_CHECK_EQUAL(vec.size(), 3);
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


/// Tests changing object via iterator
BOOST_AUTO_TEST_CASE(iterator_mut) {
    int_vector vec;
    fill(vec, {1, 2, 3});

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

        // object is not modified yet
        BOOST_CHECK_EQUAL(it->get(), 2);
    });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // object is already modified
        BOOST_CHECK_EQUAL(it->get(), 10);
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });

    auto it = vec.begin() + 1;
    it.mut()->mut() = 10;

    BOOST_CHECK_EQUAL(it->get(), 10);

    std::vector<int> expected{1, 10, 3};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests changing object via mut method
BOOST_AUTO_TEST_CASE(mut) {
    int_vector vec;
    fill(vec, {1, 2, 3});

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

        // object is not modified yet
        BOOST_CHECK_EQUAL(vec[idx].get(), 2);
    });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // object is already modified
        BOOST_CHECK_EQUAL(vec[idx].get(), 10);
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });

    vec.mut(1)->mut() = 10;

    BOOST_CHECK_EQUAL(vec.at(1).get(), 10);

    std::vector<int> expected{1, 10, 3};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests changing object directly, without vector mutator
BOOST_AUTO_TEST_CASE(object_mut) {
    int_vector vec;
    fill(vec, {1, 2});

    auto obj = std::make_unique<int_model>(3);
    auto ptr = obj.get();
    vec.insert(vec.cbegin() + 1, std::move(obj));

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

        // object is not modified yet
        BOOST_CHECK_EQUAL(it->get(), 3);
    });

    vec.after_changed().connect([&](const auto & it) {
        size_t idx = it - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // object is already modified
        BOOST_CHECK_EQUAL(it->get(), 10);
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });

    ptr->mut() = 10;

    std::vector<int> expected{1, 10, 2};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests that changed signals report current index of object after structural changes
BOOST_AUTO_TEST_CASE(mut_after_structural_changes) {
    int_vector vec;
    fill(vec, {1, 2, 3, 4, 5});

    auto obj = &vec[2];
    std::vector<size_t> changed_indexes;

    vec.after_changed().connect([&](const auto & it) {
        changed_indexes.push_back(it - vec.cbegin());
        BOOST_CHECK_EQUAL(&*it, obj);
    });

    vec.emplace(vec.cbegin(), 0);
    (vec.begin() + 3).mut()->mut() = 30;

    vec.erase(vec.cbegin(), vec.cbegin() + 2);
    vec.mut(1)->mut() = 31;

    vec.move(vec.cbegin() + 1, vec.cbegin() + 2, vec.cend());
    vec.mut(3)->mut() = 32;

    std::vector<size_t> expected{3, 1, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(changed_indexes.begin(), changed_indexes.end(),
                                  expected.begin(), expected.end());
    BOOST_CHECK_EQUAL(obj->get(), 32);
}


/// Tests that erasing empty range doesn't emit signals
BOOST_AUTO_TEST_CASE(erase_empty_range) {
    int_vector vec;
    fill(vec, {1, 2, 3});

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

    vec.erase(vec.cbegin() + 1, vec.cbegin() + 1);

    std::vector<int> expected{1, 2, 3};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

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
    int_vector vec;

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


/// Tests moving objects to position after them
BOOST_AUTO_TEST_CASE(move_forward) {
    int_vector vec;
    fill(vec, {1, 2, 3, 4, 5});

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
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.after_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);

        // vector is already modified
        std::vector<int> expected{1, 4, 5, 2, 3};
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cend());

    std::vector<int> expected{1, 4, 5, 2, 3};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 1);
    BOOST_CHECK_EQUAL(after_moved_count, 1);
}


/// Tests moving objects to position before them
BOOST_AUTO_TEST_CASE(move_backward) {
    int_vector vec;
    fill(vec, {1, 2, 3, 4, 5});

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
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.after_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 3);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 1);

        // vector is already modified
        std::vector<int> expected{1, 4, 5, 2, 3};
        auto vals = values(vec);
        BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());
    });

    vec.before_inserted().connect([&](auto && ...) { ++before_inserted_count; });
    vec.after_inserted().connect([&](auto && ...) { ++after_inserted_count; });
    vec.before_erased().connect([&](auto && ...) { ++before_erased_count; });
    vec.after_erased().connect([&](auto && ...) { ++after_erased_count; });
    vec.before_changed().connect([&](const auto &) { ++before_changed_count; });
    vec.after_changed().connect([&](const auto &) { ++after_changed_count; });

    vec.move(vec.cbegin() + 3, vec.cend(), vec.cbegin() + 1);

    std::vector<int> expected{1, 4, 5, 2, 3};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

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
    int_vector vec;
    fill(vec, {1, 2, 3, 4, 5});

    int before_moved_count = 0;
    int after_moved_count = 0;

    vec.before_moved().connect([&](size_t, size_t, size_t) { ++before_moved_count; });
    vec.after_moved().connect([&](size_t, size_t, size_t) { ++after_moved_count; });

    // empty range
    vec.move(vec.cbegin() + 1, vec.cbegin() + 1, vec.cend());

    // destination is the first moved object
    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cbegin() + 1);

    // destination is the object after moved ones
    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cbegin() + 3);

    std::vector<int> expected{1, 2, 3, 4, 5};
    auto vals = values(vec);
    BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_moved_count, 0);
    BOOST_CHECK_EQUAL(after_moved_count, 0);
}


/// Tests reading and mutating objects by iterator
BOOST_AUTO_TEST_CASE(iterator_get_mut) {
    int_vector vec;
    fill(vec, {1, 2, 3});

    auto it = vec.begin() + 1;
    BOOST_CHECK_EQUAL(vec.get(it).get(), 2);

    int after_changed_count = 0;
    vec.after_changed().connect([&](const auto & changed) {
        size_t idx = changed - vec.cbegin();
        ++after_changed_count;
        BOOST_CHECK_EQUAL(idx, 2);
    });

    vec.emplace(vec.cbegin(), 0);
    BOOST_CHECK(it == vec.begin() + 2);
    BOOST_CHECK_EQUAL(vec.get(it).get(), 2);

    vec.mut(it)->mut() = 20;
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(vec[2].get(), 20);
    BOOST_CHECK_EQUAL(vec.get(it).get(), 20);
}


/// Tests that iterators keep pointing to the same objects after structural changes
BOOST_AUTO_TEST_CASE(iterator_stability) {
    int_vector vec;
    fill(vec, {0, 1, 2, 3, 4, 5});

    auto it = vec.begin() + 2;
    auto cit = vec.cbegin() + 2;

    vec.emplace(vec.cbegin(), 10);
    BOOST_CHECK(it == vec.begin() + 3);
    BOOST_CHECK(cit == vec.cbegin() + 3);

    vec.erase(vec.cbegin(), vec.cbegin() + 2);
    BOOST_CHECK(it == vec.begin() + 1);

    vec.move(vec.cbegin() + 1, vec.cbegin() + 2, vec.cend());
    BOOST_CHECK(it == vec.end() - 1);

    vec.move(vec.cend() - 1, vec.cend(), vec.cbegin());
    BOOST_CHECK(it == vec.begin());
    BOOST_CHECK(cit == vec.cbegin());

    BOOST_CHECK_EQUAL(it->get(), 2);
    BOOST_CHECK_EQUAL(cit->get(), 2);
}


/// Tests comparison of iterators and const iterators
BOOST_AUTO_TEST_CASE(iterator_compare) {
    int_vector vec;
    fill(vec, {1, 2, 3});

    auto it0 = vec.begin();
    auto it1 = vec.begin() + 1;

    BOOST_CHECK(it0 != it1);
    BOOST_CHECK(it0 < it1);
    BOOST_CHECK(it1 > it0);

    BOOST_CHECK(it1 == vec.cbegin() + 1);
    BOOST_CHECK(it1 != vec.cbegin());
    BOOST_CHECK(it1 < vec.cbegin() + 2);
    BOOST_CHECK(it1 > vec.cbegin());

    vec.move(vec.cbegin(), vec.cbegin() + 1, vec.cend());
    BOOST_CHECK(it0 > it1);
    BOOST_CHECK_EQUAL(it0 - it1, 2);
}


/// Tests projection of single object of vector
BOOST_AUTO_TEST_CASE(element) {
    int_vector vec;
    fill(vec, {1, 2, 3});

    auto elem = vec | mv::ranges::element(vec.begin() + 1);
    BOOST_CHECK(!elem.is_null());
    BOOST_CHECK_EQUAL(elem.get().get(), 2);

    int before_changed_count = 0;
    int after_changed_count = 0;

    elem.before_changed().connect([&] {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count + 1, before_changed_count);
    });

    elem.after_changed().connect([&] {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, before_changed_count);
    });

    vec.mut(0)->mut() = 10;
    vec.mut(2)->mut() = 30;
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);

    vec.emplace(vec.cbegin(), 0);
    vec.move(vec.cbegin(), vec.cbegin() + 1, vec.cend());
    BOOST_CHECK_EQUAL(elem.get().get(), 2);

    vec.mut(1)->mut() = 20;
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
    BOOST_CHECK_EQUAL(elem.get().get(), 20);

    elem.mut()->mut() = 21;
    BOOST_CHECK_EQUAL(before_changed_count, 2);
    BOOST_CHECK_EQUAL(after_changed_count, 2);
    BOOST_CHECK_EQUAL(vec[1].get(), 21);
}


BOOST_AUTO_TEST_SUITE_END()
