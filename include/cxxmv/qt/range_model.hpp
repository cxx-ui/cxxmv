// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file range_model.hpp
/// Contains definition of the range_model class.

#pragma once

#include "../ranges/all.hpp"
#include "../ranges/projection.hpp"
#include "../signals.hpp"
#include <QAbstractItemModel>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <utility>


namespace mv::qt {


/// Base Qt item model for observable range
template <ranges::projectable_observable Range>
class range_model: public QAbstractItemModel {
public:
    /// Constructs model with specified range and parent object
    range_model(Range rng, QObject * parent = nullptr):
    QAbstractItemModel{parent},
    rng_{std::move(rng)} {
        before_inserted_con_ = rng_.before_inserted.connect([this](size_t idx, size_t count) {
            assert(count > 0 && "inserted count should not be 0");
            int row = static_cast<int>(idx);
            beginInsertRows(QModelIndex{}, row, row + static_cast<int>(count) - 1);
        });

        after_inserted_con_ = rng_.after_inserted.connect([this](size_t, size_t count) {
            assert(count > 0 && "inserted count should not be 0");
            endInsertRows();
        });

        before_erased_con_ = rng_.before_erased.connect([this](size_t idx, size_t count) {
            assert(count > 0 && "erased count should not be 0");
            int row = static_cast<int>(idx);
            beginRemoveRows(QModelIndex{}, row, row + static_cast<int>(count) - 1);
        });

        after_erased_con_ = rng_.after_erased.connect([this](size_t, size_t count) {
            assert(count > 0 && "erased count should not be 0");
            endRemoveRows();
        });

        after_changed_con_ = rng_.after_changed.connect([this](size_t idx) {
            int row = static_cast<int>(idx);
            emit dataChanged(index(row, 0), index(row, columnCount() - 1));
        });

        if constexpr (ranges::observable_with_move<Range>) {
            before_moved_con_ = rng_.before_moved.connect(
            [this](size_t first_idx, size_t count, size_t dest_idx) {
                assert(count > 0 && "moved count should not be 0");
                int row = static_cast<int>(first_idx);
                [[maybe_unused]] bool res = beginMoveRows(QModelIndex{}, row,
                                                          row + static_cast<int>(count) - 1,
                                                          QModelIndex{},
                                                          static_cast<int>(dest_idx));
                assert(res && "invalid move");
            });

            after_moved_con_ = rng_.after_moved.connect([this](size_t, size_t count, size_t) {
                assert(count > 0 && "moved count should not be 0");
                endMoveRows();
            });
        }
    }

    /// Returns index of item with specified row and column
    QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override {
        if (parent.isValid() ||
            row < 0 || row >= rowCount() ||
            column < 0 || column >= columnCount()) {

            return {};
        }

        return createIndex(row, column);
    }

    /// Returns parent of item. Table items have no parent.
    QModelIndex parent(const QModelIndex &) const override {
        return {};
    }

    /// Returns number of rows
    int rowCount(const QModelIndex & parent = {}) const override {
        return parent.isValid() ? 0 : static_cast<int>(std::ranges::size(rng_));
    }

protected:
    /// Returns range
    const Range & range() const { return rng_; }

    /// Returns range
    Range & range() { return rng_; }

private:
    Range rng_;                                         ///< Range
    scoped_signal_connection before_inserted_con_;      ///< Connection to before_inserted signal
    scoped_signal_connection after_inserted_con_;       ///< Connection to after_inserted signal
    scoped_signal_connection before_erased_con_;        ///< Connection to before_erased signal
    scoped_signal_connection after_erased_con_;         ///< Connection to after_erased signal
    scoped_signal_connection after_changed_con_;        ///< Connection to after_changed signal
    scoped_signal_connection before_moved_con_;         ///< Connection to before_moved signal
    scoped_signal_connection after_moved_con_;          ///< Connection to after_moved signal
};


template <ranges::projectable_observable Range>
range_model(Range && rng) -> range_model<ranges::all_t<Range>>;


}
