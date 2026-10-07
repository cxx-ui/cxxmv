// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_transform_range_test.cpp
/// Contains unit tests for the range ref_transform projection.

#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/model.hpp>
#include <cxxmv/observable.hpp>
#include <cxxmv/ranges/element_model.hpp>
#include <cxxmv/ranges/ref_transform.hpp>
#include <cxxmv/vector.hpp>
#include <memory>
#include <ranges>
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


BOOST_AUTO_TEST_SUITE(ref_transform_range_test)


/// Tests construction of ref transform projection
BOOST_AUTO_TEST_CASE(ctor) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    using names_t = std::decay_t<decltype(names)>;
    static_assert(mv::ranges::model<names_t, std::string>);
    static_assert(mv::ranges::borrowed_observable<names_t>);
    static_assert(std::is_same_v<decltype(*names.cbegin()), const std::string &>);

    BOOST_CHECK_EQUAL(names.size(), 3);
    BOOST_CHECK_EQUAL(std::ranges::size(names), 3);
    BOOST_CHECK(names.cbegin() != names.cend());

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(names.cbegin()[0], "John");
    BOOST_CHECK_EQUAL(names.cbegin()[1], "Jane");
    BOOST_CHECK_EQUAL(names.cbegin()[2], "Bob");
}


/// Tests inserting single element into base model
BOOST_AUTO_TEST_CASE(insert_base_single) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_inserted().connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.after_inserted().connect([&](size_t idx, size_t count) {
        ++after_inserted_count;
        BOOST_CHECK_EQUAL(before_inserted_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.before_erased().connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased().connect([&](size_t, size_t) { ++after_erased_count; });
    names.before_changed().connect([&](size_t) { ++before_changed_count; });
    names.after_changed().connect([&](size_t) { ++after_changed_count; });

    vec.insert(vec.begin() + 1, user{"Alice", "White"});

    std::vector<std::string> expected{"John", "Alice", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests inserting range of elements into base model
BOOST_AUTO_TEST_CASE(insert_base_range) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_inserted().connect([&](size_t idx, size_t count) {
        ++before_inserted_count;
        BOOST_CHECK_EQUAL(after_inserted_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // transformed model is not modified yet
        std::vector<std::string> expected{"John", "Jane", "Bob"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.after_inserted().connect([&](size_t idx, size_t count) {
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
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.before_erased().connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased().connect([&](size_t, size_t) { ++after_erased_count; });
    names.before_changed().connect([&](size_t) { ++before_changed_count; });
    names.after_changed().connect([&](size_t) { ++after_changed_count; });

    std::vector<user> users{{"Alice", "White"}, {"Tom", "Green"}};
    vec.insert(vec.begin() + 1, users.begin(), users.end());

    std::vector<std::string> expected{"John", "Alice", "Tom", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 1);
    BOOST_CHECK_EQUAL(after_inserted_count, 1);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests erasing range of elements in base model
BOOST_AUTO_TEST_CASE(erase_base) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"},
                         {"Alice", "White"}, {"Tom", "Green"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_erased().connect([&](size_t idx, size_t count) {
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
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.after_erased().connect([&](size_t idx, size_t count) {
        ++after_erased_count;
        BOOST_CHECK_EQUAL(before_erased_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(count, 2);

        // idx is the index of the element following erased ones
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");

        // transformed model is already modified
        std::vector<std::string> expected{"John", "Alice", "Tom"};
        BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(),
                                      expected.begin(), expected.end());
    });

    names.before_inserted().connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted().connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_changed().connect([&](size_t) { ++before_changed_count; });
    names.after_changed().connect([&](size_t) { ++after_changed_count; });

    vec.erase(vec.begin() + 1, vec.begin() + 3);

    std::vector<std::string> expected{"John", "Alice", "Tom"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 1);
    BOOST_CHECK_EQUAL(after_erased_count, 1);
    BOOST_CHECK_EQUAL(before_changed_count, 0);
    BOOST_CHECK_EQUAL(after_changed_count, 0);
}


/// Tests changing element in base model
BOOST_AUTO_TEST_CASE(change_base) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_changed().connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Jane");
    });

    names.after_changed().connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
    });

    names.before_inserted().connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted().connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_erased().connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased().connect([&](size_t, size_t) { ++after_erased_count; });

    vec.mut(1) = user{"Alice", "White"};

    std::vector<std::string> expected{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_inserted_count, 0);
    BOOST_CHECK_EQUAL(after_inserted_count, 0);
    BOOST_CHECK_EQUAL(before_erased_count, 0);
    BOOST_CHECK_EQUAL(after_erased_count, 0);
    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests changing element in transformed model
BOOST_AUTO_TEST_CASE(change_transformed) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;

    names.before_changed().connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Jane");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Jane");
    });

    names.after_changed().connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Alice");
    });

    names.before_inserted().connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted().connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_erased().connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased().connect([&](size_t, size_t) { ++after_erased_count; });

    (names.begin() + 1).mut() = std::string{"Alice"};

    // only referenced field of base model element is changed
    BOOST_REQUIRE_EQUAL(vec.size(), 3);
    BOOST_CHECK_EQUAL(std::as_const(vec)[0].first_name, "John");
    BOOST_CHECK_EQUAL(std::as_const(vec)[0].last_name, "Smith");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name, "Doe");
    BOOST_CHECK_EQUAL(std::as_const(vec)[2].first_name, "Bob");
    BOOST_CHECK_EQUAL(std::as_const(vec)[2].last_name, "Brown");

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
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};

    int before_changed_count = 0;
    int after_changed_count = 0;

    (vec | mv::ranges::ref_transform(get_first_name)).before_changed().connect([&](size_t idx) {
        ++before_changed_count;
        BOOST_CHECK_EQUAL(after_changed_count, 0);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is not modified yet
        BOOST_CHECK_EQUAL((vec | mv::ranges::ref_transform(get_first_name)).cbegin()[idx],
                          "Jane");
    });

    (vec | mv::ranges::ref_transform(get_first_name)).after_changed().connect([&](size_t idx) {
        ++after_changed_count;
        BOOST_CHECK_EQUAL(before_changed_count, 1);
        BOOST_CHECK_EQUAL(idx, 1);

        // element is already modified
        BOOST_CHECK_EQUAL((vec | mv::ranges::ref_transform(get_first_name)).cbegin()[idx],
                          "Alice");
    });

    (vec | mv::ranges::ref_transform(get_first_name)).mut(1) = std::string{"Alice"};

    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name, "Doe");

    std::vector<std::string> expected{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(
        std::ranges::cbegin(vec | mv::ranges::ref_transform(get_first_name)),
        std::ranges::cend(vec | mv::ranges::ref_transform(get_first_name)),
        expected.begin(), expected.end());

    BOOST_CHECK_EQUAL(before_changed_count, 1);
    BOOST_CHECK_EQUAL(after_changed_count, 1);
}


/// Tests changing object pointed by base model element
BOOST_AUTO_TEST_CASE(change_unique_ptr) {
    mv::vector<std::unique_ptr<test_user>> vec;
    vec.emplace_back(std::make_unique<test_user>("John", "Smith"));
    vec.emplace_back(std::make_unique<test_user>("Jane", "Doe"));

    auto users = vec | mv::ranges::ref_transform([](auto && p) -> auto & { return *p; });

    int changed_count = 0;
    users.after_changed().connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(users.cbegin()[idx].first_name(), "Alice");
    });

    users.mut(1)->set_first_name("Alice");

    BOOST_CHECK_EQUAL(users.cbegin()[0].first_name(), "John");
    BOOST_CHECK_EQUAL(users.cbegin()[1].first_name(), "Alice");
    BOOST_CHECK_EQUAL(users.cbegin()[1].last_name(), "Doe");
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests nested ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform_nested) {
    struct team {
        user leader;
        std::string name;
    };

    mv::vector<team> vec{{{"John", "Smith"}, "red"}, {{"Jane", "Doe"}, "blue"}};

    auto get_leader = [](auto && t) -> auto & { return t.leader; };
    auto names = vec | mv::ranges::ref_transform(get_leader) |
                 mv::ranges::ref_transform(get_first_name);

    int changed_count = 0;
    names.after_changed().connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
    });

    names.mut(1) = std::string{"Alice"};

    BOOST_CHECK_EQUAL(std::as_const(vec)[1].leader.first_name, "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].leader.last_name, "Doe");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].name, "blue");
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests ref transform projection of temporary range model
BOOST_AUTO_TEST_CASE(ref_transform_temporary_base) {
    auto names = mv::vector<user>{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}} |
                 mv::ranges::ref_transform(get_first_name);

    using names_t = std::decay_t<decltype(names)>;
    static_assert(mv::ranges::model<names_t, std::string>);
    static_assert(!mv::ranges::borrowed_observable<names_t>);

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names.after_changed().connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
    });

    (names.begin() + 1).mut() = std::string{"Alice"};

    std::vector<std::string> changed{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving ref transform projection of temporary range model
BOOST_AUTO_TEST_CASE(ref_transform_temporary_base_move) {
    auto names = mv::vector<user>{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}} |
                 mv::ranges::ref_transform(get_first_name);
    auto names2 = std::move(names);

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names2.after_changed().connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names2.cbegin()[idx], "Alice");
    });

    (names2.begin() + 1).mut() = std::string{"Alice"};

    std::vector<std::string> changed{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests copying ref transform projection of range model reference
BOOST_AUTO_TEST_CASE(ref_transform_ref_base_copy) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);
    auto names2 = names;

    std::vector<std::string> expected{"John", "Jane", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), expected.begin(), expected.end());

    int changed_count = 0;
    names2.after_changed().connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names2.cbegin()[idx], "Alice");
    });

    (names2.begin() + 1).mut() = std::string{"Alice"};

    // copy changes the same base model
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Alice");
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].last_name, "Doe");

    std::vector<std::string> changed{"John", "Alice", "Bob"};
    BOOST_CHECK_EQUAL_COLLECTIONS(names.cbegin(), names.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.cbegin(), names2.cend(), changed.begin(), changed.end());
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving ref transform mutator
BOOST_AUTO_TEST_CASE(ref_transform_mutator_move) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int changed_count = 0;
    names.after_changed().connect([&](size_t idx) {
        ++changed_count;
        BOOST_CHECK_EQUAL(idx, 1);
        BOOST_CHECK_EQUAL(names.cbegin()[idx], "Alice");
    });

    {
        auto mut = names.mut(1);
        BOOST_CHECK_EQUAL(mut.ref(), "Jane");

        mut.ref() = "Alice";

        auto mut2 = std::move(mut);
        BOOST_CHECK_EQUAL(mut2.ref(), "Alice");

        // element is changed in place, signal is not emitted until mutator is destroyed
        BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Alice");
        BOOST_CHECK_EQUAL(changed_count, 0);
    }

    // signal is emitted only once by destroyed mutator
    BOOST_CHECK_EQUAL(std::as_const(vec)[1].first_name, "Alice");
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving elements in base model
BOOST_AUTO_TEST_CASE(move_base) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"},
                         {"Alice", "White"}, {"Tom", "Green"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    static_assert(mv::ranges::observable_with_move<decltype(names)>);

    int before_inserted_count = 0;
    int after_inserted_count = 0;
    int before_erased_count = 0;
    int after_erased_count = 0;
    int before_changed_count = 0;
    int after_changed_count = 0;
    int before_moved_count = 0;
    int after_moved_count = 0;

    names.before_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
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

    names.after_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
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

    names.before_inserted().connect([&](size_t, size_t) { ++before_inserted_count; });
    names.after_inserted().connect([&](size_t, size_t) { ++after_inserted_count; });
    names.before_erased().connect([&](size_t, size_t) { ++before_erased_count; });
    names.after_erased().connect([&](size_t, size_t) { ++after_erased_count; });
    names.before_changed().connect([&](size_t) { ++before_changed_count; });
    names.after_changed().connect([&](size_t) { ++after_changed_count; });

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


/// Tests moving elements in base model after moving ref transform projection
BOOST_AUTO_TEST_CASE(move_base_after_projection_move) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    int moved_count = 0;
    names.after_moved().connect([&](size_t, size_t, size_t) { ++moved_count; });

    auto names2 = std::move(names);

    int moved_count2 = 0;
    names2.after_moved().connect([&](size_t first_idx, size_t count, size_t dest_idx) {
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


/// Tests element model of ref transform projection
BOOST_AUTO_TEST_CASE(element) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    using element_t = mv::ranges::element_model<std::decay_t<decltype(names)>>;

    static_assert(mv::model_of<element_t, std::string>);
    static_assert(mv::nullable_observable_as<element_t, std::string>);
    static_assert(std::is_same_v<decltype(std::declval<element_t>().get()), const std::string &>);

    element_t name{names, vec.handle(1)};
    BOOST_CHECK(!name.is_null());
    BOOST_CHECK_EQUAL(*name, "Jane");

    int changed_count = 0;
    name.changed().connect([&changed_count] { ++changed_count; });

    vec.insert(vec.cbegin(), user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(changed_count, 0);

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(*name, "Jane");
    BOOST_CHECK_EQUAL(changed_count, 0);

    name.mut() = "Alice";
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(vec[0].first_name, "Alice");
    BOOST_CHECK_EQUAL(vec[0].last_name, "Doe");

    vec.erase(vec.cbegin(), vec.cbegin() + 1);
    BOOST_CHECK(name.is_null());
    BOOST_CHECK_EQUAL(changed_count, 2);
}


/// Tests setting handle of element model of ref transform projection
BOOST_AUTO_TEST_CASE(element_set) {
    mv::vector<user> vec{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    auto names = vec | mv::ranges::ref_transform(get_first_name);

    mv::ranges::element_model<std::decay_t<decltype(names)>> name{names};
    BOOST_CHECK(name.is_null());

    int changed_count = 0;
    name.changed().connect([&changed_count] { ++changed_count; });

    name.set(vec.handle(2));
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Bob");

    vec.insert(vec.cbegin(), user{"Tom", "Green"});
    BOOST_CHECK_EQUAL(*name, "Bob");
    BOOST_CHECK_EQUAL(changed_count, 1);

    name.set({});
    BOOST_CHECK_EQUAL(changed_count, 2);
    BOOST_CHECK(name.is_null());
}


/// Tests element model of ref transform projection of temporary vector
BOOST_AUTO_TEST_CASE(element_temporary_base) {
    auto names = mv::vector<user>{{"John", "Smith"}, {"Jane", "Doe"}}
               | mv::ranges::ref_transform(get_first_name);

    using element_t = mv::ranges::element_model<std::decay_t<decltype(names)>>;

    element_t name{names, names.handle(1)};
    BOOST_CHECK_EQUAL(*name, "Jane");

    int changed_count = 0;
    name.changed().connect([&changed_count] { ++changed_count; });

    names.mut(0) = "Tom";
    BOOST_CHECK_EQUAL(changed_count, 0);

    name.mut() = "Alice";
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*name, "Alice");
    BOOST_CHECK_EQUAL(names.cbegin()[1], "Alice");
}


BOOST_AUTO_TEST_SUITE_END()
