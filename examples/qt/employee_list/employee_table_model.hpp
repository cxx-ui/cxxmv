// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee_table_model.hpp
/// Contains definition of the employee_table_model class.

#pragma once

#include "employee.hpp"
#include <cxxmv/qt/range_model.hpp>
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/ranges/model.hpp>
#include <cxxmv/ranges/projection.hpp>
#include <QString>
#include <QVariant>
#include <ranges>
#include <utility>
#include <vector>


template <mv::ranges::projectable_observable Range>
class employee_table_model: public mv::qt::range_model<Range> {
public:
    /// Table columns
    enum column {
        first_name_column,
        last_name_column,
        full_name_column,
        type_column,
        column_count
    };

    /// Is model read only?
    static constexpr bool is_read_only = !mv::ranges::model<Range, employee>;

    /// Does range support moving of employees?
    static constexpr bool supports_move = mv::ranges::model_with_move<Range, employee>;

    /// Does range support inserting of employees?
    static constexpr bool supports_insert = mv::ranges::model_with_insert<Range, employee>;

    /// Constructs model with specified range of employees and parent object
    employee_table_model(Range rng, QObject * parent = nullptr):
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

        const employee & cont = std::ranges::begin(this->range())[idx.row()];
        switch (idx.column()) {
        case first_name_column:
            return QString::fromStdWString(cont.first_name());
        case last_name_column:
            return QString::fromStdWString(cont.last_name());
        case full_name_column:
            return QString::fromStdWString(cont.full_name());
        case type_column:
            return QString::fromStdString(employee_type_to_string(cont.type()));
        default:
            return {};
        }
    }

    /// Assigns data to item
    bool setData(const QModelIndex & idx, const QVariant & var, int role = Qt::EditRole) override {
        if constexpr (is_read_only) {
            return false;
        } else {
            // display role is assigned by setItemData when dropped rows are inserted
            if (!idx.isValid() || (role != Qt::EditRole && role != Qt::DisplayRole)) {
                return false;
            }

            auto it = std::ranges::begin(this->range()) + idx.row();
            employee cont = *it;
            auto val = var.toString().toStdWString();

            switch (idx.column()) {
            case first_name_column:
                cont.set_first_name(std::move(val));
                break;
            case last_name_column:
                cont.set_last_name(std::move(val));
                break;
            case full_name_column:
                cont.set_full_name(val);
                break;
            case type_column:
                cont.set_type(employee_type::contractor);
            default:
                return false;
            }

            it.mut() = cont;
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
            if (idx.column() != type_column) {
                res |= Qt::ItemIsEditable;
            }
        }

        if constexpr (supports_move) {
            res |= Qt::ItemIsDragEnabled;
        }

        return res;
    }

    /// Inserts empty employees at specified row
    bool insertRows(int row, int count, const QModelIndex & parent = {}) override {
        if constexpr (!supports_insert) {
            return false;
        } else {
            if (parent.isValid() || row < 0 || row > this->rowCount() || count <= 0) {
                return false;
            }

            std::vector<employee> vals(count, employee{{}, {}, employee_type::permanent});
            auto pos = std::ranges::begin(this->range()) + row;
            this->range().insert(pos, vals.begin(), vals.end());
            return true;
        }
    }

    /// Returns supported drop actions
    Qt::DropActions supportedDropActions() const override {
        return supports_move ? Qt::MoveAction : Qt::IgnoreAction;
    }
};


template <mv::ranges::projectable_observable Range>
employee_table_model(Range && rng) -> employee_table_model<mv::ranges::all_t<Range>>;
