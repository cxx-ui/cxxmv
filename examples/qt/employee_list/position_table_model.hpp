// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file position_table_model.hpp
/// Contains definition of the position_table_model class.

#pragma once

#include "position.hpp"
#include <cxxmv/qt/range_model.hpp>
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/ranges/model.hpp>
#include <cxxmv/ranges/projection.hpp>
#include <QString>
#include <QVariant>
#include <ranges>
#include <utility>


template <mv::ranges::projectable_observable Range>
class position_table_model: public mv::qt::range_model<Range> {
public:
    /// Table columns
    enum column {
        name_column,
        employee_column,
        column_count
    };

    /// Is model read only?
    static constexpr bool is_read_only = !mv::ranges::model<Range, position>;

    /// Does range support moving of positions?
    static constexpr bool supports_move = mv::ranges::model_with_move<Range, position>;

    /// Constructs model with specified range of positions and parent object
    position_table_model(Range rng, QObject * parent = nullptr):
    mv::qt::range_model<Range>{std::move(rng), parent} {}

    /// Returns number of columns
    int columnCount(const QModelIndex & parent = {}) const override {
        return parent.isValid() ? 0 : column_count;
    }

    /// Returns data of item
    QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override {
        if (!idx.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
            return {};
        }

        const position & pos = std::ranges::begin(this->range())[idx.row()];
        switch (idx.column()) {
        case name_column:
            return QString::fromStdWString(pos.name());
        case employee_column:
            if (!pos.employee()) {
                return QString{};
            }

            return QString::fromStdWString(pos.employee()->full_name());
        default:
            return {};
        }
    }

    /// Assigns data to item, only position name is editable
    bool setData(const QModelIndex & idx, const QVariant & var, int role = Qt::EditRole) override {
        if constexpr (is_read_only) {
            return false;
        } else {
            if (!idx.isValid() || idx.column() != name_column ||
                (role != Qt::EditRole && role != Qt::DisplayRole)) {
                return false;
            }

            auto it = std::ranges::begin(this->range()) + idx.row();
            it.mut()->set_name(var.toString().toStdWString());
            return true;
        }
    }

    /// Returns item flags
    Qt::ItemFlags flags(const QModelIndex & idx) const override {
        if (!idx.isValid()) {
            // root item support drag and drop
            return supports_move ? Qt::ItemIsDropEnabled : Qt::NoItemFlags;
        }

        Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if constexpr (!is_read_only) {
            res |= Qt::ItemIsEditable;
        }

        if constexpr (supports_move) {
            res |= Qt::ItemIsDragEnabled;
        }

        return res;
    }

    /// Returns supported drop actions
    Qt::DropActions supportedDropActions() const override {
        return supports_move ? Qt::MoveAction : Qt::IgnoreAction;
    }
};


template <mv::ranges::projectable_observable Range>
position_table_model(Range && rng) -> position_table_model<mv::ranges::all_t<Range>>;
