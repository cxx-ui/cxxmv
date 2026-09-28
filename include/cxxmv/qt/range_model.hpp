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
        before_inserted_con_ = rng_.before_inserted.connect([this](auto pos, std::size_t count) {
            assert(count > 0 && "inserted count should not be 0");
            int row = get_row(pos);
            beginInsertRows(QModelIndex{}, row, row + static_cast<int>(count) - 1);
        });

        after_inserted_con_ = rng_.after_inserted.connect([this](auto, std::size_t count) {
            assert(count > 0 && "inserted count should not be 0");
            endInsertRows();
        });

        before_erased_con_ = rng_.before_erased.connect([this](auto pos, std::size_t count) {
            assert(count > 0 && "erased count should not be 0");
            int row = get_row(pos);
            beginRemoveRows(QModelIndex{}, row, row + static_cast<int>(count) - 1);
        });

        after_erased_con_ = rng_.after_erased.connect([this](auto, std::size_t count) {
            assert(count > 0 && "erased count should not be 0");
            endRemoveRows();
        });

        after_changed_con_ = rng_.after_changed.connect([this](auto pos) {
            int row = get_row(pos);
            emit dataChanged(index(row, 0), index(row, columnCount() - 1));
        });
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
    /// Returns row number for specified range iterator
    template <typename It>
    int get_row(const It & pos) const {
        return static_cast<int>(pos - std::ranges::begin(rng_));
    }

    Range rng_;                                         ///< Range
    scoped_signal_connection before_inserted_con_;      ///< Connection to before_inserted signal
    scoped_signal_connection after_inserted_con_;       ///< Connection to after_inserted signal
    scoped_signal_connection before_erased_con_;        ///< Connection to before_erased signal
    scoped_signal_connection after_erased_con_;         ///< Connection to after_erased signal
    scoped_signal_connection after_changed_con_;        ///< Connection to after_changed signal
};


template <ranges::projectable_observable Range>
range_model(Range && rng) -> range_model<ranges::all_t<Range>>;


}
