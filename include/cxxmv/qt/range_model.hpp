// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file range_model.hpp
/// Contains definition of the range_model class.

#pragma once

#include "../ranges/all.hpp"
#include "../ranges/model.hpp"
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
    /// Does range support erasing of elements?
    static constexpr bool supports_erase =
        ranges::model_with_erase<Range, std::ranges::range_value_t<Range>>;

    /// Does range support moving of elements?
    static constexpr bool supports_move =
        ranges::model_with_move<Range, std::ranges::range_value_t<Range>>;

public:
    /// Constructs model with specified range and parent object
    range_model(Range rng, QObject * parent = nullptr):
    QAbstractItemModel{parent},
    rng_{std::move(rng)} {
        before_inserted_con_ = rng_.before_inserted().connect(
        [this](const auto & pos, size_t count) {
            assert(count > 0 && "inserted count should not be 0");
            int row = static_cast<int>(pos - std::ranges::cbegin(rng_));
            beginInsertRows(QModelIndex{}, row, row + static_cast<int>(count) - 1);
        });

        after_inserted_con_ = rng_.after_inserted().connect(
        [this](const auto & first, const auto & last) {
            assert(first != last && "inserted count should not be 0");
            endInsertRows();
        });

        before_erased_con_ = rng_.before_erased().connect(
        [this](const auto & first, const auto & last) {
            assert(first != last && "erased count should not be 0");
            int row = static_cast<int>(first - std::ranges::cbegin(rng_));
            int count = static_cast<int>(last - first);
            beginRemoveRows(QModelIndex{}, row, row + count - 1);
        });

        after_erased_con_ = rng_.after_erased().connect([this](const auto &, size_t count) {
            assert(count > 0 && "erased count should not be 0");
            endRemoveRows();
        });

        after_changed_con_ = rng_.after_changed().connect([this](const auto & it) {
            int row = static_cast<int>(it - std::ranges::cbegin(rng_));
            emit dataChanged(index(row, 0), index(row, columnCount() - 1));
        });

        if constexpr (ranges::observable_with_move<Range>) {
            before_moved_con_ = rng_.before_moved().connect(
            [this](size_t first_idx, size_t count, size_t dest_idx) {
                assert(count > 0 && "moved count should not be 0");
                int row = static_cast<int>(first_idx);
                [[maybe_unused]] bool res = beginMoveRows(QModelIndex{}, row,
                                                          row + static_cast<int>(count) - 1,
                                                          QModelIndex{},
                                                          static_cast<int>(dest_idx));
                assert(res && "invalid move");
            });

            after_moved_con_ = rng_.after_moved().connect([this](size_t, size_t count, size_t) {
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

    /// Returns iterator pointing to element of item with specified index
    auto at(const QModelIndex & idx) {
        assert(idx.isValid() && idx.model() == this && "invalid item index");
        return std::ranges::begin(rng_) + idx.row();
    }

    /// Returns const iterator pointing to element of item with specified index
    auto at(const QModelIndex & idx) const {
        assert(idx.isValid() && idx.model() == this && "invalid item index");
        return std::ranges::begin(rng_) + idx.row();
    }

    /// Removes rows from model
    bool removeRows(int row, int count, const QModelIndex & parent = {}) override {
        if constexpr (supports_erase) {
            if (parent.isValid() || row < 0 || count <= 0 || row + count > rowCount()) {
                return false;
            }

            auto first = std::ranges::begin(rng_) + row;
            rng_.erase(first, first + count);
            return true;
        } else {
            return false;
        }
    }

    /// Moves rows in model to position before destination row
    bool moveRows(const QModelIndex & source_parent,
                  int source_row,
                  int count,
                  const QModelIndex & dest_parent,
                  int dest_row) override {
        if constexpr (supports_move) {
            if (source_parent.isValid() || dest_parent.isValid() ||
                source_row < 0 || count <= 0 || source_row + count > rowCount() ||
                dest_row < 0 || dest_row > rowCount()) {

                return false;
            }

            // don't move rows with destination inside move range
            if (dest_row >= source_row && dest_row <= source_row + count) {
                return false;
            }

            auto first = std::ranges::begin(rng_) + source_row;
            auto dest = std::ranges::begin(rng_) + dest_row;
            rng_.move(first, first + count, dest);
            return true;
        } else {
            return false;
        }
    }

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
