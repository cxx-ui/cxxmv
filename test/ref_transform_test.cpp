// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_transform_test.cpp
/// Contains unit tests for the ref_transform projection.

#include "cxxmv/ranges/observable.hpp"
#include "test_user.hpp"
#include <boost/test/unit_test.hpp>
#include <cxxmv/basic_model.hpp>
#include <cxxmv/ref_ransform.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>


namespace {

/// Point with two coordinates
struct point {
    int x = 0;
    int y = 0;
};

/// Segment with two points
struct segment {
    point first;
    point second;
};

/// Returns reference to x coordinate of point
auto get_x = [](auto && p) -> auto & { return p.x; };

/// Returns reference to first point of segment
auto get_first = [](auto && s) -> auto & { return s.first; };

}


BOOST_AUTO_TEST_SUITE(ref_transform_test)


/// Tests changing member of model value via ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform_member) {
    mv::basic_model<point> mdl{1, 2};
    auto mdl2 = mdl | mv::ref_transform(get_x);

    using transform_t = std::decay_t<decltype(mdl2)>;

    static_assert(mv::mutator<std::decay_t<decltype(mdl2.mut())>, int>);
    static_assert(mv::model_of<transform_t, int>);
    static_assert(std::move_constructible<transform_t>);
    static_assert(std::copy_constructible<transform_t>);

    BOOST_CHECK_EQUAL(mdl2.get(), 1);

    int changed_count = 0;
    mdl2.changed.connect([&changed_count, &mdl, &mdl2] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl->x, 5);
        BOOST_CHECK_EQUAL(mdl->y, 2);
        BOOST_CHECK_EQUAL(mdl2.get(), 5);
    });

    mdl2.mut().ref() = 5;

    BOOST_CHECK_EQUAL(mdl->x, 5);
    BOOST_CHECK_EQUAL(mdl->y, 2);
    BOOST_CHECK_EQUAL(mdl2.get(), 5);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests changing object pointed by model value via ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform_unique_ptr) {
    mv::basic_model<std::unique_ptr<test_user>> mdl{std::make_unique<test_user>("first", "last")};
    auto user = mdl | mv::ref_transform([](auto && p) -> auto & { return *p; });

    BOOST_CHECK_EQUAL(user.get().first_name(), "first");

    int changed_count = 0;
    user.changed.connect([&changed_count, &mdl, &user] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl.get()->first_name(), "new first");
        BOOST_CHECK_EQUAL(user.get().first_name(), "new first");
    });

    user.mut()->set_first_name("new first");

    BOOST_CHECK_EQUAL(mdl.get()->first_name(), "new first");
    BOOST_CHECK_EQUAL(mdl.get()->last_name(), "last");
    BOOST_CHECK_EQUAL(user.get().first_name(), "new first");
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests ref transform projection of ref transform projection
BOOST_AUTO_TEST_CASE(ref_transform_nested) {
    mv::basic_model<segment> mdl{point{1, 2}, point{3, 4}};
    auto mdl2 = mdl | mv::ref_transform(get_first) | mv::ref_transform(get_x);

    BOOST_CHECK_EQUAL(mdl2.get(), 1);

    int changed_count = 0;
    mdl2.changed.connect([&changed_count, &mdl, &mdl2] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl->first.x, 5);
        BOOST_CHECK_EQUAL(mdl2.get(), 5);
    });

    mdl2.mut().ref() = 5;

    BOOST_CHECK_EQUAL(mdl->first.x, 5);
    BOOST_CHECK_EQUAL(mdl->first.y, 2);
    BOOST_CHECK_EQUAL(mdl->second.x, 3);
    BOOST_CHECK_EQUAL(mdl->second.y, 4);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests conversion of ref transform projection to model value
BOOST_AUTO_TEST_CASE(ref_transform_deref_convert) {
    mv::basic_model<point> mdl{1, 2};
    auto mdl2 = mdl | mv::ref_transform(get_x);

    int val = *mdl2;
    BOOST_CHECK_EQUAL(val, 1);
}


/// Tests conversion of ref transform projection to model value in function call
BOOST_AUTO_TEST_CASE(ref_transform_deref_convert_call) {
    mv::basic_model<point> mdl{1, 2};
    auto mdl2 = mdl | mv::ref_transform(get_x);

    auto call_func = [](const int & val) {
        BOOST_CHECK_EQUAL(val, 1);
    };

    call_func(*mdl2);
}


/// Tests ref transform projection of temporary model
BOOST_AUTO_TEST_CASE(ref_transform_temporary_model) {
    auto mdl = mv::basic_model<point>{1, 2} | mv::ref_transform(get_x);

    BOOST_CHECK_EQUAL(mdl.get(), 1);

    int changed_count = 0;
    mdl.changed.connect([&changed_count, &mdl] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl.get(), 5);
    });

    mdl.mut().ref() = 5;

    BOOST_CHECK_EQUAL(mdl.get(), 5);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving ref transform projection of temporary model
BOOST_AUTO_TEST_CASE(ref_transform_temporary_model_move) {
    auto mdl = mv::basic_model<point>{1, 2} | mv::ref_transform(get_x);
    auto mdl2 = std::move(mdl);

    BOOST_CHECK_EQUAL(mdl2.get(), 1);

    int changed_count = 0;
    mdl2.changed.connect([&changed_count, &mdl2] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl2.get(), 5);
    });

    mdl2.mut().ref() = 5;

    BOOST_CHECK_EQUAL(mdl2.get(), 5);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests copying ref transform projection of model reference
BOOST_AUTO_TEST_CASE(ref_transform_ref_model_copy) {
    mv::basic_model<point> mdl{1, 2};
    auto mdl2 = mdl | mv::ref_transform(get_x);
    auto mdl3 = mdl2;

    BOOST_CHECK_EQUAL(mdl3.get(), 1);

    int changed_count = 0;
    mdl3.changed.connect([&changed_count, &mdl, &mdl2, &mdl3] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl->x, 5);
        BOOST_CHECK_EQUAL(mdl2.get(), 5);
        BOOST_CHECK_EQUAL(mdl3.get(), 5);
    });

    mdl3.mut().ref() = 5;

    BOOST_CHECK_EQUAL(mdl->x, 5);
    BOOST_CHECK_EQUAL(mdl2.get(), 5);
    BOOST_CHECK_EQUAL(mdl3.get(), 5);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests moving ref transform mutator
BOOST_AUTO_TEST_CASE(ref_transform_mutator_move) {
    mv::basic_model<point> mdl{1, 2};
    auto mdl2 = mdl | mv::ref_transform(get_x);

    int changed_count = 0;
    mdl2.changed.connect([&changed_count, &mdl, &mdl2] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl->x, 5);
        BOOST_CHECK_EQUAL(mdl2.get(), 5);
    });

    {
        auto mut = mdl2.mut();
        BOOST_CHECK_EQUAL(mut.ref(), 1);

        mut.ref() = 5;

        auto mut2 = std::move(mut);
        BOOST_CHECK_EQUAL(mut2.ref(), 5);

        // model value is changed in place, signal is not emitted until mutator is destroyed
        BOOST_CHECK_EQUAL(mdl->x, 5);
        BOOST_CHECK_EQUAL(changed_count, 0);
    }

    // signal is emitted only once by destroyed mutator
    BOOST_CHECK_EQUAL(mdl->x, 5);
    BOOST_CHECK_EQUAL(mdl2.get(), 5);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


/// Tests assignment to ref transform projection via mutator
BOOST_AUTO_TEST_CASE(ref_transform_mut_assign) {
    mv::basic_model<point> mdl{1, 2};
    auto mdl2 = mdl | mv::ref_transform(get_x);

    int changed_count = 0;
    mdl2.changed.connect([&changed_count, &mdl, &mdl2] {
        ++changed_count;
        BOOST_CHECK_EQUAL(mdl->x, 5);
        BOOST_CHECK_EQUAL(mdl2.get(), 5);
    });

    mdl2.mut() = 5;

    BOOST_CHECK_EQUAL(mdl->x, 5);
    BOOST_CHECK_EQUAL(mdl->y, 2);
    BOOST_CHECK_EQUAL(mdl2.get(), 5);
    BOOST_CHECK_EQUAL(changed_count, 1);
}


BOOST_AUTO_TEST_SUITE_END()
