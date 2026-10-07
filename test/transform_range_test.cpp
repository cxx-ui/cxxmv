// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file transform_range_test.cpp
/// Contains unit tests for the range transform projection.

#include "cxxmv/ranges/observable.hpp"
#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/ranges/element_model.hpp>
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

    using names_t = std::decay_t<decltype(names)>;
    static_assert(mv::ranges::observable_as<names_t, std::string>);
    static_assert(mv::ranges::borrowed_observable<names_t>);

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

    names.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.after_inserted.connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    names.before_changed.connect([&](size_t) { ++before_changed_count; });
    names.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    names.before_inserted.connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.after_inserted.connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the first inserted element
        std::vector<std::string> inserted{"Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin() + idx, names.cbegin() + idx + count,
                                      inserted.begin(), inserted.end());

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    names.before_changed.connect([&](size_t) { ++before_changed_count; });
    names.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    names.before_erased.connect([&](size_t idx, size_t count) {
        ++before_erased_count;
        BOOST_CHECK_EQUAL(after_erased_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the first element being erased
        std::vector<std::string> erased{"Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin() + idx, names.cbegin() + idx + count,
                                      erased.begin(), erased.end());

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.after_erased.connect([&](size_t idx, size_t count) {
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the element following erased ones
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.begin(), names.end(), expected.begin(), expected.end());
    });

    names.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_changed.connect([&](size_t) { ++before_changed_count; });
    names.after_changed.connect([&](size_t) { ++after_changed_count; });

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

    names.before_changed.connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Jane");
    });

    names.after_changed.connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
    });

    names.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });

    vec.mut(1) = test_user{"Alice", "White"};

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

    using names_t = std::decay_t<decltype(names)>;
    static_assert(mv::ranges::model<names_t, std::string>);
    static_assert(mv::ranges::borrowed_observable<names_t>);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_changed.connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Jane");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Jane");
    });

    names.after_changed.connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    });

    names.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });

    (names.begin() + 1).mut() = std::string{"Alice"};

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


/// Tests changing element in temporary transformed model
BOOST_AUTO_TEST_CASE(change_transformed_borrowed) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    (vec | mv::ranges::transform(get_fn, set_fn)).before_changed.connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL((vec | mv::ranges::transform(get_fn, set_fn)).cbegin()[idx], "Jane");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Jane");
    });

    (vec | mv::ranges::transform(get_fn, set_fn)).after_changed.connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL((vec | mv::ranges::transform(get_fn, set_fn)).cbegin()[idx], "Alice");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    });

    (vec | mv::ranges::transform(get_fn, set_fn)).before_inserted.connect([&](size_t, size_t) {
        ++before_inserted_count;
    });
    (vec | mv::ranges::transform(get_fn, set_fn)).after_inserted.connect([&](size_t, size_t) {
        ++after_inserted_count;
    });
    (vec | mv::ranges::transform(get_fn, set_fn)).before_erased.connect([&](size_t, size_t) {
        ++before_erased_count;
    });
    (vec | mv::ranges::transform(get_fn, set_fn)).after_erased.connect([&](size_t, size_t) {
        ++after_erased_count;
    });

    (vec | mv::ranges::transform(get_fn, set_fn)).mut(1) = std::string{"Alice"};

    // base model is changed with set function, other fields are kept
    BOOST_REQUIRE_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(std::as_const(vec)[0].first_name(), "John");
    BOOST_CHECK_EQUAL(std::as_const(vec)[0].last_name(), "Smith");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name(), "Doe");
    BOOST_CHECK_EQUAL(std::as_const(vec)[2].first_name(), "Bob");
    BOOST_CHECK_EQUAL(std::as_const(vec)[2].last_name(), "Brown");

    std::vector<std::string> expected{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(std::ranges::cbegin(vec | mv::ranges::transform(get_fn, set_fn)),
                                  std::ranges::cend(vec | mv::ranges::transform(get_fn, set_fn)),
                                  expected.begin(), expected.end());

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

    using names_t = std::decay_t<decltype(names)>;
    static_assert(mv::ranges::model<names_t, std::string>);
    static_assert(!mv::ranges::borrowed_observable<names_t>);

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names.after_changed.connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
    });

    (names.begin() + 1).mut() = std::string{"Alice"};

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
    names2.after_changed.connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names2.cbegin()[idx], "Alice");
    });

    (names2.begin() + 1).mut() = std::string{"Alice"};

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
    names2.after_changed.connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names2.cbegin()[idx], "Alice");
    });

    (names2.begin() + 1).mut() = std::string{"Alice"};

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

    names.before_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++before_moved_count;
        BOOST_CHECK_EQUAL(after_moved_count, 0);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);
        BOOST_CHECK_EQUAL(names.cbegin()[first_idx], "Jane");

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.after_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++after_moved_count;
        BOOST_CHECK_EQUAL(before_moved_count, 1);
        BOOST_CHECK_EQUAL(first_idx, 1);
        BOOST_CHECK_EQUAL(count, 2);
        BOOST_CHECK_EQUAL(dest_idx, 5);

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.before_inserted.connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted.connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_erased.connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased.connect([&](size_t, size_t) { ++after_erased_count; });
    names.before_changed.connect([&](size_t) { ++before_changed_count; });
    names.after_changed.connect([&](size_t) { ++after_changed_count; });

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
    names.after_moved.connect([&](size_t, size_t, size_t) { ++moved_count; });

    auto names2 = std::move(names);

    int moved_count2 = 0;
    names2.after_moved.connect([&](size_t first_idx, size_t count, size_t dest_idx) {
        ++moved_count2;
        BOOST_CHECK_EQUAL(first_idx, 2);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(dest_idx, 0);
    });

    vec.move(vec.cbegin() + 2, vec.cend(), vec.cbegin());

    std::vector<std::string> expected{"Bob", "John", "Jane"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(),
                                  expected.begin(), expected.end());

    // slot connected before projection move is moved with signal
    BOOST_CHECK_EQUAL(moved_count, 1);
    BOOST_CHECK_EQUAL(moved_count2, 1);
}


/// Tests transform of transformed model
BOOST_AUTO_TEST_CASE(double_transform) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_name = [](const test_user & u) { return u.first_name(); };
    auto set_name = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto get_title = [](const std::string & name) { return "Mr. " + name; };
    auto set_title = [](std::string & name, const std::string & title) {
        name = title.substr(4);
    };

    auto titles = vec | mv::ranges::transform(get_name, set_name)
                      | mv::ranges::transform(get_title, set_title);

    using titles_t = std::decay_t<decltype(titles)>;
    static_assert(mv::ranges::model<titles_t, std::string>);
    static_assert(mv::ranges::borrowed_observable<titles_t>);

    std::vector<std::string> expected{"Mr. John", "Mr. Jane", "Mr. Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(titles.cbegin(), titles.cend(),
                                  expected.begin(), expected.end());

    int before_changed_count = 0;
    int after_changed_count = 0;

    titles.before_changed.connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(titles.cbegin()[idx], "Mr. Jane");
    });

    titles.after_changed.connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(titles.cbegin()[idx], "Mr. Alice");
    });

    (titles.begin() + 1).mut() = std::string{"Mr. Alice"};

    BOOST_REQUIRE_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name(), "Doe");

    expected = {"Mr. John", "Mr. Alice", "Mr. Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(titles.cbegin(), titles.cend(),
                                  expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests element model of transform projection
BOOST_AUTO_TEST_CASE(element) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto set_fn = [](test_user & u, const std::string & name) { u.set_first_name(name); };
    auto names = vec | mv::ranges::transform(get_fn, set_fn);

    using element_t = mv::ranges::element_model<std::decay_t<decltype(names)>>;

    static_assert(mv::model_of<element_t, std::string>);
    static_assert(mv::nullable_observable_as<element_t, std::string>);

    element_t name{names, 1};
    BOOST_CHECK(!name.is_null());
    BOOST_CHECK_EQUAL(*name, "Jane");

    int changed_count = 0;
    name.changed.connect([&changed_count] { ++changed_count; });

    vec.insert(vec.cbegin(), test_user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(changed_count, 0);

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(changed_count, 0);

    name.mut() = std::string{"Alice"};
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(vec[0].first_name(), "Alice");
    BOOST_CHECK_EQUAL(vec[0].last_name(), "Doe");

    vec.erase(vec.cbegin(), vec.cbegin() + 1);
    BOOST_CHECK(name.is_null());
    BOOST_CHECK_EQUAL(changed_count, 2);
}


/// Tests setting index of element model of transform projection
BOOST_AUTO_TEST_CASE(element_set_index) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    mv::ranges::element_model<std::decay_t<decltype(names)>> name{names};
    BOOST_CHECK(name.is_null());

    int changed_count = 0;
    name.changed.connect([&changed_count] { ++changed_count; });

    name.set_index(2);
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Bob");

    vec.insert(vec.cbegin(), test_user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Bob");
    BOOST_CHECK_EQUAL(changed_count, 1);

    name.set_index(SIZE_MAX);
    BOOST_CHECK_EQUAL(changed_count, 2);
    BOOST_CHECK(name.is_null());
}


/// Tests element model of transform projection without set function
BOOST_AUTO_TEST_CASE(element_read_only) {
    mv::vector<test_user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    auto get_fn = [](const test_user & u) { return u.first_name(); };
    auto names = vec | mv::ranges::transform(get_fn);

    using element_t = mv::ranges::element_model<std::decay_t<decltype(names)>>;

    static_assert(mv::nullable_observable_as<element_t, std::string>);
    static_assert(!mv::model<element_t>);

    element_t name{names, 1};
    BOOST_CHECK_EQUAL(*name, "Jane");

    int changed_count = 0;
    name.changed.connect([&changed_count] { ++changed_count; });

    vec.mut(0) = test_user{"Tom", "Green"};
    BOOST_CHECK_EQUAL(changed_count, 0);

    vec.mut(1) = test_user{"Alice", "White"};
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
}


BOOST_AUTO_TEST_SUITE_END()
