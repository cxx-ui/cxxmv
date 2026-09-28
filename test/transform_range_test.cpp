// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file transform_range_test.cpp
/// Contains unit tests for the range transform projection.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/ranges/transform.hpp>
#include <cxxmv/vector.hpp>
#include <ranges>
#include <string>
#include <utility>
#include <vector>


BOOST_AUTO_TEST_SUITE(transform_range_test)


/// Tests construction of transform projection
BOOST_AUTO_TEST_CASE(ctor) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    BOOST_CHECK_EQUAL(names.size(), 3);
    BOOST_CHECK_EQUAL(std::ranges::size(names), 3);
    BOOST_CHECK(names.begin() != names.end());

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(names.begin()[0], "John");
    BOOST_CHECK_EQUAL(names.begin()[1], "Jane");
    BOOST_CHECK_EQUAL(names.begin()[2], "Bob");
}


/// Tests inserting single element into base model
BOOST_AUTO_TEST_CASE(insert_base_single) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_inserted.connect([&](auto pos, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);
        BOOST_CHECK_EQUAL(count, 1);

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.after_inserted.connect([&](auto pos, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(*pos, "Alice");

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.before_erased.connect([&](auto, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](auto, size_t) { ++after_erased_count; });
    names.before_changed.connect([&](auto) { ++before_changed_count; });
    names.after_changed.connect([&](auto) { ++after_changed_count; });

    vec.insert(vec.begin() + 1, test_user{"Alice", "White"});

    std::vector<std::string> expected{"John", "Alice", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests inserting range of elements into base model
BOOST_AUTO_TEST_CASE(insert_base_range) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_inserted.connect([&](auto pos, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);
        BOOST_CHECK_EQUAL(count, 2);

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.after_inserted.connect([&](auto pos, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);
        BOOST_CHECK_EQUAL(count, 2);

        // pos points to the first of inserted elements
        std::vector<std::string> inserted{"Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(pos, pos + count, inserted.begin(), inserted.end());

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.before_erased.connect([&](auto, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](auto, size_t) { ++after_erased_count; });
    names.before_changed.connect([&](auto) { ++before_changed_count; });
    names.after_changed.connect([&](auto) { ++after_changed_count; });

    std::vector<test_user> users{{"Alice", "White"}, {"Tom", "Green"}};
    vec.insert(vec.begin() + 1, users.begin(), users.end());

    std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests erasing range of elements in base model
BOOST_AUTO_TEST_CASE(erase_base) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"},
                              {"Alice", "White"}, {"Tom", "Green"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_erased.connect([&](auto pos, size_t count) {
        ++before_erased_count;
        BOOST_CHECK_EQUAL(after_erased_count, 0);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);
        BOOST_CHECK_EQUAL(count, 2);

        // pos points to the first of elements being erased
        std::vector<std::string> erased{"Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(pos, pos + count, erased.begin(), erased.end());

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.after_erased.connect([&](auto pos, size_t count) {
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);
        BOOST_CHECK_EQUAL(count, 2);

        // pos points to the element following erased ones
        BOOST_CHECK_EQUAL(*pos, "Alice");

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.before_inserted.connect([&](auto, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](auto, size_t) { ++after_inserted_count; });
    names.before_changed.connect([&](auto) { ++before_changed_count; });
    names.after_changed.connect([&](auto) { ++after_changed_count; });

    vec.erase(vec.begin() + 1, vec.begin() + 3);

    std::vector<std::string> expected{"John", "Alice", "Tom"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 1);
    BOOST_CHECK_EQUAL(after_erased_count, 1);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests changing element in base model
BOOST_AUTO_TEST_CASE(change_base) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_changed.connect([&](auto pos) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(*pos, "Jane");
    });

    names.after_changed.connect([&](auto pos) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(std::distance(names.begin(), pos), 1);

        // element is already modified
        BOOST_CHECK_EQUAL(*pos, "Alice");
    });

    names.before_inserted.connect([&](auto, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](auto, size_t) { ++after_inserted_count; });
    names.before_erased.connect([&](auto, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](auto, size_t) { ++after_erased_count; });

    vec.at(1) = test_user{"Alice", "White"};

    std::vector<std::string> expected{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests changing element in transformed model
BOOST_AUTO_TEST_CASE(change_transformed) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto names = vec | mv::ranges::transform(get_fn, set_fn);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_changed.connect([&](auto pos) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), pos), 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(*pos, "Jane");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Jane");
    });

    names.after_changed.connect([&](auto pos) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), pos), 1);

        // element is already modified
        BOOST_CHECK_EQUAL(*pos, "Alice");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    });

    names.before_inserted.connect([&](auto, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](auto, size_t) { ++after_inserted_count; });
    names.before_erased.connect([&](auto, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](auto, size_t) { ++after_erased_count; });

    *(names.begin() + 1) = std::string{"Alice"};

    // base model is changed with set function, other fields are kept
    BOOST_REQUIRE_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(std::as_const(vec)[0].first_name(), "John");
    BOOST_CHECK_EQUAL(std::as_const(vec)[0].last_name(), "Smith");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name(), "Doe");
    BOOST_CHECK_EQUAL(std::as_const(vec)[2].first_name(), "Bob");
    BOOST_CHECK_EQUAL(std::as_const(vec)[2].last_name(), "Brown");

    std::vector<std::string> expected{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests transform projection of temporary range model
BOOST_AUTO_TEST_CASE(transform_temporary_base) {
    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto names = mv::vector<test_user>{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}} |
                 mv::ranges::transform(get_fn, set_fn);

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names.after_changed.connect([&](auto pos) {
        ++changed_count;
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), pos), 1);
        BOOST_CHECK_EQUAL(*pos, "Alice");
    });

    *(names.begin() + 1) = std::string{"Alice"};

    std::vector<std::string> changed{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving transform projection of temporary range model
BOOST_AUTO_TEST_CASE(transform_temporary_base_move) {
    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto names = mv::vector<test_user>{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}} |
                 mv::ranges::transform(get_fn, set_fn);
    auto names2 = std::move(names);

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names2.after_changed.connect([&](auto pos) {
        ++changed_count;
        BOOST_CHECK_EQUAL(std::distance(names2.cbegin(), pos), 1);
        BOOST_CHECK_EQUAL(*pos, "Alice");
    });

    *(names2.begin() + 1) = std::string{"Alice"};

    std::vector<std::string> changed{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests copying transform projection of range model reference
BOOST_AUTO_TEST_CASE(transform_ref_base_copy) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto names = vec | mv::ranges::transform(get_fn, set_fn);
    auto names2 = names;

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names2.after_changed.connect([&](auto pos) {
        ++changed_count;
        BOOST_CHECK_EQUAL(std::distance(names2.cbegin(), pos), 1);
        BOOST_CHECK_EQUAL(*pos, "Alice");
    });

    *(names2.begin() + 1) = std::string{"Alice"};

    // copy changes the same base model
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name(), "Doe");

    std::vector<std::string> changed{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving elements in base model
BOOST_AUTO_TEST_CASE(move_base) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"},
                              {"Alice", "White"}, {"Tom", "Green"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    static_assert(mv::ranges::observable_with_move<decltype(names)>);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    names.before_moved.connect([&](auto first, size_t count, auto dest) {
        ++before_moved_count;
        BOOST_CHECK_EQUAL(after_moved_count, 0);
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), first), 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), dest), 5);
        BOOST_CHECK_EQUAL(*first, "Jane");

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.after_moved.connect([&](auto first, size_t count, auto dest) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), first), 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(std::distance(names.cbegin(), dest), 5);

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.before_inserted.connect([&](auto, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](auto, size_t) { ++after_inserted_count; });
    names.before_erased.connect([&](auto, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](auto, size_t) { ++after_erased_count; });
    names.before_changed.connect([&](auto) { ++before_changed_count; });
    names.after_changed.connect([&](auto) { ++after_changed_count; });

    vec.move(vec.cbegin() + 1, vec.cbegin() + 3, vec.cend());

    std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
    BOOST_CHECK_EQUAL(before_moved_count, 1);
    BOOST_CHECK_EQUAL(after_moved_count, 1);
}


/// Tests moving elements in base model after moving transform projection
BOOST_AUTO_TEST_CASE(move_base_after_projection_move) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    int moved_count = 0;
    names.after_moved.connect([&](auto, size_t, auto) { ++moved_count; });

    auto names2 = std::move(names);

    int moved_count2 = 0;
    names2.after_moved.connect([&](auto first, size_t count, auto dest) {
        ++moved_count2;
        BOOST_CHECK_EQUAL(std::distance(names2.cbegin(), first), 2);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(std::distance(names2.cbegin(), dest), 0);
    });

    vec.move(vec.cbegin() + 2, vec.cend(), vec.cbegin());

    std::vector<std::string> expected{"Bob", "John", "Jane"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(),
                                  expected.begin(), expected.end());

    // slot connected before projection move is moved with signal
    BOOST_CHECK_EQUAL(moved_count, 1);
    BOOST_CHECK_EQUAL(moved_count2, 1);
}


BOOST_AUTO_TEST_SUITE_END()
