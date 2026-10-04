// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file range_model_test.cpp
/// Contains unit tests for the range_model class.

#include "../test_user.hpp"
#include <cxxmv/vector.hpp>
#include <cxxmv/qt/range_model.hpp>
#include <boost/test/unit_test.hpp>
#include <QString>
#include <QVariant>
#include <ranges>
#include <vector>


namespace {


/// Test model displaying first and last names of users in two columns
template <mv::ranges::projectable_observable_as<test_user> Range>
class test_user_model: public mv::qt::range_model<Range> {
public:
    /// Table columns
    enum column {
        first_name_column,
        last_name_column,
        column_count
    };

    /// Constructs model with specified range of users and parent object
    test_user_model(Range rng, QObject * parent = nullptr):
    mv::qt::range_model<Range>{std::move(rng), parent} {}

    /// Returns number of columns
    int columnCount(const QModelIndex & parent = {}) const override {
        return parent.isValid() ? 0 : column_count;
    }

    /// Returns data of item
    QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override {
        if (!idx.isValid() || role != Qt::DisplayRole) {
            return {};
        }

        const test_user & user = std::ranges::begin(this->range())[idx.row()];
        switch (idx.column()) {
        case first_name_column:
            return QString::fromStdString(user.first_name());
        case last_name_column:
            return QString::fromStdString(user.last_name());
        default:
            return {};
        }
    }
};


template <mv::ranges::projectable_observable Range>
test_user_model(Range && rng) -> test_user_model<mv::ranges::all_t<Range>>;


/// Fixture for range model test suite
struct range_model_test_fixture {
    mv::vector<test_user> users{{"John", "Smith"}, {"Jane", "Doe"}, {"Bob", "Brown"}};
    test_user_model<mv::ranges::all_t<mv::vector<test_user> &>> model{users};
};


}


BOOST_FIXTURE_TEST_SUITE(range_model_test, range_model_test_fixture)


/// Tests model values after construction
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK_EQUAL(model.rowCount(), 3);
    BOOST_CHECK_EQUAL(model.columnCount(), 2);

    BOOST_CHECK(model.data(model.index(0, 0)) == "John");
    BOOST_CHECK(model.data(model.index(0, 1)) == "Smith");
    BOOST_CHECK(model.data(model.index(1, 0)) == "Jane");
    BOOST_CHECK(model.data(model.index(1, 1)) == "Doe");
    BOOST_CHECK(model.data(model.index(2, 0)) == "Bob");
    BOOST_CHECK(model.data(model.index(2, 1)) == "Brown");

    // indexes out of range are invalid
    BOOST_CHECK(!model.index(3, 0).isValid());
    BOOST_CHECK(!model.index(0, 2).isValid());
    BOOST_CHECK(!model.index(-1, 0).isValid());

    // table items have no children and no parent
    BOOST_CHECK_EQUAL(model.rowCount(model.index(0, 0)), 0);
    BOOST_CHECK_EQUAL(model.columnCount(model.index(0, 0)), 0);
    BOOST_CHECK(!model.parent(model.index(0, 0)).isValid());
}


/// Tests inserting elements into base model
BOOST_AUTO_TEST_CASE(insert_base) {
    int rows_about_to_be_inserted_count = 0;
    int rows_inserted_count = 0;
    int rows_about_to_be_removed_count = 0;
    int rows_removed_count = 0;
    int data_changed_count = 0;

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted,
                     [&](const QModelIndex & parent, int first, int last) {
        ++rows_about_to_be_inserted_count;
        BOOST_CHECK_EQUAL(rows_inserted_count, 0);
        BOOST_CHECK(!parent.isValid());
        BOOST_CHECK_EQUAL(first, 1);
        BOOST_CHECK_EQUAL(last, 2);

        // model is not modified yet
        BOOST_CHECK_EQUAL(model.rowCount(), 3);
        BOOST_CHECK(model.data(model.index(1, 0)) == "Jane");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsInserted,
                     [&](const QModelIndex & parent, int first, int last) {
        ++rows_inserted_count;
        BOOST_CHECK_EQUAL(rows_about_to_be_inserted_count, 1);
        BOOST_CHECK(!parent.isValid());
        BOOST_CHECK_EQUAL(first, 1);
        BOOST_CHECK_EQUAL(last, 2);

        // model is already modified
        BOOST_CHECK_EQUAL(model.rowCount(), 5);
        BOOST_CHECK(model.data(model.index(1, 0)) == "Alice");
        BOOST_CHECK(model.data(model.index(2, 0)) == "Tom");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeRemoved, [&] {
        ++rows_about_to_be_removed_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, [&] { ++rows_removed_count; });
    QObject::connect(&model, &QAbstractItemModel::dataChanged, [&] { ++data_changed_count; });

    std::vector<test_user> new_users{{"Alice", "White"}, {"Tom", "Green"}};
    users.insert(users.begin() + 1, new_users.begin(), new_users.end());

    BOOST_CHECK_EQUAL(model.rowCount(), 5);
    BOOST_CHECK(model.data(model.index(0, 0)) == "John");
    BOOST_CHECK(model.data(model.index(1, 0)) == "Alice");
    BOOST_CHECK(model.data(model.index(1, 1)) == "White");
    BOOST_CHECK(model.data(model.index(2, 0)) == "Tom");
    BOOST_CHECK(model.data(model.index(2, 1)) == "Green");
    BOOST_CHECK(model.data(model.index(3, 0)) == "Jane");
    BOOST_CHECK(model.data(model.index(4, 0)) == "Bob");

    BOOST_CHECK_EQUAL(rows_about_to_be_inserted_count, 1);
    BOOST_CHECK_EQUAL(rows_inserted_count, 1);
    BOOST_CHECK_EQUAL(rows_about_to_be_removed_count, 0);
    BOOST_CHECK_EQUAL(rows_removed_count, 0);
    BOOST_CHECK_EQUAL(data_changed_count, 0);
}


/// Tests erasing elements in base model
BOOST_AUTO_TEST_CASE(erase_base) {
    int rows_about_to_be_inserted_count = 0;
    int rows_inserted_count = 0;
    int rows_about_to_be_removed_count = 0;
    int rows_removed_count = 0;
    int data_changed_count = 0;

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeRemoved,
                     [&](const QModelIndex & parent, int first, int last) {
        ++rows_about_to_be_removed_count;
        BOOST_CHECK_EQUAL(rows_removed_count, 0);
        BOOST_CHECK(!parent.isValid());
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);

        // model is not modified yet
        BOOST_CHECK_EQUAL(model.rowCount(), 3);
        BOOST_CHECK(model.data(model.index(0, 0)) == "John");
        BOOST_CHECK(model.data(model.index(1, 0)) == "Jane");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsRemoved,
                     [&](const QModelIndex & parent, int first, int last) {
        ++rows_removed_count;
        BOOST_CHECK_EQUAL(rows_about_to_be_removed_count, 1);
        BOOST_CHECK(!parent.isValid());
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 1);

        // model is already modified
        BOOST_CHECK_EQUAL(model.rowCount(), 1);
        BOOST_CHECK(model.data(model.index(0, 0)) == "Bob");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, [&] {
        ++rows_about_to_be_inserted_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, [&] { ++rows_inserted_count; });
    QObject::connect(&model, &QAbstractItemModel::dataChanged, [&] { ++data_changed_count; });

    users.erase(users.begin(), users.begin() + 2);

    BOOST_CHECK_EQUAL(model.rowCount(), 1);
    BOOST_CHECK(model.data(model.index(0, 0)) == "Bob");
    BOOST_CHECK(model.data(model.index(0, 1)) == "Brown");

    BOOST_CHECK_EQUAL(rows_about_to_be_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_about_to_be_removed_count, 1);
    BOOST_CHECK_EQUAL(rows_removed_count, 1);
    BOOST_CHECK_EQUAL(data_changed_count, 0);
}


/// Tests changing element in base model
BOOST_AUTO_TEST_CASE(change_base) {
    int rows_about_to_be_inserted_count = 0;
    int rows_inserted_count = 0;
    int rows_about_to_be_removed_count = 0;
    int rows_removed_count = 0;
    int data_changed_count = 0;

    QObject::connect(&model, &QAbstractItemModel::dataChanged,
                     [&](const QModelIndex & top_left, const QModelIndex & bottom_right) {
        ++data_changed_count;

        // whole row is changed
        BOOST_CHECK_EQUAL(top_left.row(), 1);
        BOOST_CHECK_EQUAL(top_left.column(), 0);
        BOOST_CHECK_EQUAL(bottom_right.row(), 1);
        BOOST_CHECK_EQUAL(bottom_right.column(), 1);

        // model is already modified
        BOOST_CHECK(model.data(model.index(1, 0)) == "Alice");
        BOOST_CHECK(model.data(model.index(1, 1)) == "White");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, [&] {
        ++rows_about_to_be_inserted_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, [&] { ++rows_inserted_count; });
    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeRemoved, [&] {
        ++rows_about_to_be_removed_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, [&] { ++rows_removed_count; });

    users.mut(1) = test_user{"Alice", "White"};

    BOOST_CHECK_EQUAL(model.rowCount(), 3);
    BOOST_CHECK(model.data(model.index(0, 0)) == "John");
    BOOST_CHECK(model.data(model.index(1, 0)) == "Alice");
    BOOST_CHECK(model.data(model.index(1, 1)) == "White");
    BOOST_CHECK(model.data(model.index(2, 0)) == "Bob");

    BOOST_CHECK_EQUAL(rows_about_to_be_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_about_to_be_removed_count, 0);
    BOOST_CHECK_EQUAL(rows_removed_count, 0);
    BOOST_CHECK_EQUAL(data_changed_count, 1);
}


/// Tests moving elements in base model to position after them
BOOST_AUTO_TEST_CASE(move_base_forward) {
    int rows_about_to_be_inserted_count = 0;
    int rows_inserted_count = 0;
    int rows_about_to_be_removed_count = 0;
    int rows_removed_count = 0;
    int data_changed_count = 0;
    int rows_about_to_be_moved_count = 0;
    int rows_moved_count = 0;

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeMoved,
                     [&](const QModelIndex & source_parent, int source_start, int source_end,
                         const QModelIndex & dest_parent, int dest_row) {
        ++rows_about_to_be_moved_count;
        BOOST_CHECK_EQUAL(rows_moved_count, 0);
        BOOST_CHECK(!source_parent.isValid());
        BOOST_CHECK_EQUAL(source_start, 0);
        BOOST_CHECK_EQUAL(source_end, 0);
        BOOST_CHECK(!dest_parent.isValid());
        BOOST_CHECK_EQUAL(dest_row, 3);

        // model is not modified yet
        BOOST_CHECK(model.data(model.index(0, 0)) == "John");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsMoved,
                     [&](const QModelIndex & source_parent, int source_start, int source_end,
                         const QModelIndex & dest_parent, int dest_row) {
        ++rows_moved_count;
        BOOST_CHECK_EQUAL(rows_about_to_be_moved_count, 1);
        BOOST_CHECK(!source_parent.isValid());
        BOOST_CHECK_EQUAL(source_start, 0);
        BOOST_CHECK_EQUAL(source_end, 0);
        BOOST_CHECK(!dest_parent.isValid());
        BOOST_CHECK_EQUAL(dest_row, 3);

        // model is already modified
        BOOST_CHECK(model.data(model.index(2, 0)) == "John");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, [&] {
        ++rows_about_to_be_inserted_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, [&] { ++rows_inserted_count; });
    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeRemoved, [&] {
        ++rows_about_to_be_removed_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, [&] { ++rows_removed_count; });
    QObject::connect(&model, &QAbstractItemModel::dataChanged, [&] { ++data_changed_count; });

    users.move(users.cbegin(), users.cbegin() + 1, users.cend());

    BOOST_CHECK_EQUAL(model.rowCount(), 3);
    BOOST_CHECK(model.data(model.index(0, 0)) == "Jane");
    BOOST_CHECK(model.data(model.index(1, 0)) == "Bob");
    BOOST_CHECK(model.data(model.index(2, 0)) == "John");
    BOOST_CHECK(model.data(model.index(2, 1)) == "Smith");

    BOOST_CHECK_EQUAL(rows_about_to_be_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_about_to_be_removed_count, 0);
    BOOST_CHECK_EQUAL(rows_removed_count, 0);
    BOOST_CHECK_EQUAL(data_changed_count, 0);
    BOOST_CHECK_EQUAL(rows_about_to_be_moved_count, 1);
    BOOST_CHECK_EQUAL(rows_moved_count, 1);
}


/// Tests moving elements in base model to position before them
BOOST_AUTO_TEST_CASE(move_base_backward) {
    int rows_about_to_be_inserted_count = 0;
    int rows_inserted_count = 0;
    int rows_about_to_be_removed_count = 0;
    int rows_removed_count = 0;
    int data_changed_count = 0;
    int rows_about_to_be_moved_count = 0;
    int rows_moved_count = 0;

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeMoved,
                     [&](const QModelIndex & source_parent, int source_start, int source_end,
                         const QModelIndex & dest_parent, int dest_row) {
        ++rows_about_to_be_moved_count;
        BOOST_CHECK_EQUAL(rows_moved_count, 0);
        BOOST_CHECK(!source_parent.isValid());
        BOOST_CHECK_EQUAL(source_start, 1);
        BOOST_CHECK_EQUAL(source_end, 2);
        BOOST_CHECK(!dest_parent.isValid());
        BOOST_CHECK_EQUAL(dest_row, 0);

        // model is not modified yet
        BOOST_CHECK(model.data(model.index(0, 0)) == "John");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsMoved,
                     [&](const QModelIndex & source_parent, int source_start, int source_end,
                         const QModelIndex & dest_parent, int dest_row) {
        ++rows_moved_count;
        BOOST_CHECK_EQUAL(rows_about_to_be_moved_count, 1);
        BOOST_CHECK(!source_parent.isValid());
        BOOST_CHECK_EQUAL(source_start, 1);
        BOOST_CHECK_EQUAL(source_end, 2);
        BOOST_CHECK(!dest_parent.isValid());
        BOOST_CHECK_EQUAL(dest_row, 0);

        // model is already modified
        BOOST_CHECK(model.data(model.index(0, 0)) == "Jane");
    });

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, [&] {
        ++rows_about_to_be_inserted_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, [&] { ++rows_inserted_count; });
    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeRemoved, [&] {
        ++rows_about_to_be_removed_count;
    });
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, [&] { ++rows_removed_count; });
    QObject::connect(&model, &QAbstractItemModel::dataChanged, [&] { ++data_changed_count; });

    users.move(users.cbegin() + 1, users.cend(), users.cbegin());

    BOOST_CHECK_EQUAL(model.rowCount(), 3);
    BOOST_CHECK(model.data(model.index(0, 0)) == "Jane");
    BOOST_CHECK(model.data(model.index(1, 0)) == "Bob");
    BOOST_CHECK(model.data(model.index(2, 0)) == "John");

    BOOST_CHECK_EQUAL(rows_about_to_be_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_inserted_count, 0);
    BOOST_CHECK_EQUAL(rows_about_to_be_removed_count, 0);
    BOOST_CHECK_EQUAL(rows_removed_count, 0);
    BOOST_CHECK_EQUAL(data_changed_count, 0);
    BOOST_CHECK_EQUAL(rows_about_to_be_moved_count, 1);
    BOOST_CHECK_EQUAL(rows_moved_count, 1);
}


BOOST_AUTO_TEST_SUITE_END()
