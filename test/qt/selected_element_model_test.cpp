// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selected_element_model_test.cpp
/// Contains unit tests for the selected_element_model class.

#include <boost/test/unit_test.hpp>
#include <cxxmv/qt/range_model.hpp>
#include <cxxmv/qt/selected_element_model.hpp>
#include <cxxmv/vector.hpp>
#include <QItemSelectionModel>
#include <QVariant>
#include <cstdint>


namespace {


/// Test model displaying integer values in single column
template <mv::ranges::projectable_observable Range>
class test_int_model: public mv::qt::range_model<Range> {
public:
    /// Constructs model with specified range of integers
    test_int_model(Range rng):
    mv::qt::range_model<Range>{std::move(rng)} {}

    /// Returns number of columns
    int columnCount(const QModelIndex & parent = {}) const override {
        return parent.isValid() ? 0 : 1;
    }

    /// Returns data of item
    QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override {
        if (!idx.isValid() || role != Qt::DisplayRole) {
            return {};
        }

        return std::ranges::begin(this->range())[idx.row()];
    }
};


/// Fixture for selected element test suite
struct selected_element_model_test_fixture {
    mv::vector<int> vec{10, 20, 30, 40};
    test_int_model<mv::ranges::all_t<mv::vector<int> &>> model{vec};
    mv::qt::selected_element_model<mv::vector<int>> selection{vec, &model};

    /// Returns current row of selection model, -1 if there is no current row
    int current_row() const {
        return selection.currentIndex().isValid() ? selection.currentIndex().row() : -1;
    }
};


}


BOOST_FIXTURE_TEST_SUITE(selected_element_model_test, selected_element_model_test_fixture)


/// Tests selected element after construction
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK(selection.element().is_null());
    BOOST_CHECK_EQUAL(current_row(), -1);
}


/// Tests changing current row in selection model
BOOST_AUTO_TEST_CASE(change_current_row) {
    auto & elem = selection.element();

    int changed_count = 0;

    elem.changed.connect([&] {
        ++changed_count;
        BOOST_CHECK_EQUAL(elem.index(), 1);
        BOOST_CHECK_EQUAL(*elem, 20);
    });

    selection.setCurrentIndex(model.index(1, 0), QItemSelectionModel::ClearAndSelect);
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(elem.index(), 1);
    BOOST_CHECK_EQUAL(*elem, 20);
}


/// Tests clearing selection in selection model, current row is not changed
BOOST_AUTO_TEST_CASE(clear_selection) {
    auto & elem = selection.element();
    selection.setCurrentIndex(model.index(1, 0), QItemSelectionModel::ClearAndSelect);

    int changed_count = 0;

    elem.changed.connect([&] {
        ++changed_count;
        BOOST_CHECK(elem.is_null());
    });

    selection.clearSelection();
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK(elem.is_null());
    BOOST_CHECK_EQUAL(current_row(), 1);
}


/// Tests setting handle of selected element
BOOST_AUTO_TEST_CASE(set) {
    auto & elem = selection.element();

    int changed_count = 0;
    int current_row_changed_count = 0;

    elem.changed.connect([&] { ++changed_count; });

    QObject::connect(&selection, &QItemSelectionModel::currentRowChanged,
    [&](const QModelIndex & current) {
        ++current_row_changed_count;
        BOOST_CHECK_EQUAL(current.row(), 2);
    });

    elem.set(vec.handle(2));
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(current_row_changed_count, 1);
    BOOST_CHECK_EQUAL(current_row(), 2);
    BOOST_CHECK(selection.isRowSelected(2));
    BOOST_CHECK(!selection.isRowSelected(1));
    BOOST_CHECK_EQUAL(*elem, 30);
}


/// Tests setting null handle of selected element
BOOST_AUTO_TEST_CASE(set_null) {
    auto & elem = selection.element();
    elem.set(vec.handle(2));

    int changed_count = 0;
    elem.changed.connect([&] { ++changed_count; });

    elem.set({});
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK(elem.is_null());
    BOOST_CHECK_EQUAL(current_row(), -1);
    BOOST_CHECK(!selection.hasSelection());
}


/// Tests changing value of selected element
BOOST_AUTO_TEST_CASE(change_element) {
    auto & elem = selection.element();
    elem.set(vec.handle(1));

    int changed_count = 0;
    elem.changed.connect([&] { ++changed_count; });

    vec.mut(0) = 15;
    BOOST_CHECK_EQUAL(changed_count, 0);

    vec.mut(1) = 25;
    BOOST_CHECK_EQUAL(changed_count, 1);
    BOOST_CHECK_EQUAL(*elem, 25);
    BOOST_CHECK_EQUAL(current_row(), 1);
}


/// Tests inserting elements before selected element
BOOST_AUTO_TEST_CASE(insert_before) {
    auto & elem = selection.element();
    elem.set(vec.handle(1));

    vec.insert(vec.cbegin(), 5);
    BOOST_CHECK_EQUAL(elem.index(), 2);
    BOOST_CHECK_EQUAL(current_row(), 2);
    BOOST_CHECK_EQUAL(*elem, 20);
}


/// Tests moving selected element
BOOST_AUTO_TEST_CASE(move_selected) {
    auto & elem = selection.element();
    elem.set(vec.handle(2));

    vec.move(vec.cbegin() + 2, vec.cbegin() + 3, vec.cbegin());
    BOOST_CHECK_EQUAL(elem.index(), 0);
    BOOST_CHECK_EQUAL(current_row(), 0);
    BOOST_CHECK_EQUAL(*elem, 30);
}


/// Tests erasing selected element
BOOST_AUTO_TEST_CASE(erase_selected) {
    auto & elem = selection.element();
    elem.set(vec.handle(2));

    vec.erase(vec.cbegin() + 2, vec.cbegin() + 3);
    BOOST_CHECK(elem.is_null());
    BOOST_CHECK(!selection.hasSelection());
}


BOOST_AUTO_TEST_SUITE_END()
