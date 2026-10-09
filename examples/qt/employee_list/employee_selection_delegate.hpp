// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee_selection_delegate.hpp
/// Contains definition of the employee_selection_delegate class.

#pragma once

#include "employee.hpp"
#include "employee_table_model.hpp"
#include "position.hpp"
#include "position_table_model.hpp"
#include <cxxmv/ranges/all.hpp>
#include <cxxmv/qt/combo_box.hpp>
#include <QAbstractItemModel>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <algorithm>
#include <ranges>


/// Delegate that edits employee of position with combo box that displays employees
class employee_selection_delegate: public QStyledItemDelegate {
public:
    /// Type of table model of positions
    using positions_model_t = position_table_model<mv::ranges::all_t<position_list &>>;

    /// Constructs delegate with specified references to table model of positions, list
    /// of employees and parent object
    employee_selection_delegate(positions_model_t & positions_model,
                                employee_list & employees,
                                QObject * parent = nullptr):
    QStyledItemDelegate{parent},
    positions_model_{positions_model},
    model_{employees} {}

    /// Creates combo box editor
    QWidget * createEditor(QWidget * parent,
                           const QStyleOptionViewItem &,
                           const QModelIndex &) const override {
        auto self = const_cast<employee_selection_delegate *>(this);

        auto combo = new QComboBox{parent};
        combo->setModel(&self->model_);
        combo->setModelColumn(model_t::full_name_column);

        // committing data and closing editor when item is selected in combo box
        connect(combo, &QComboBox::activated, this, [self, combo] {
            emit self->commitData(combo);
            emit self->closeEditor(combo);
        });

        return combo;
    }

    /// Sets current item of combo box to employee of position
    void setEditorData(QWidget * editor, const QModelIndex & idx) const override {
        auto combo = static_cast<QComboBox *>(editor);
        const employee * empl = positions_model_.at(idx)->employee();

        auto & employees = model_.range();
        auto it = std::ranges::find_if(employees, [empl](auto & e) { return &e == empl; });
        combo->setCurrentIndex(it == std::ranges::end(employees) ?
                               -1 :
                               static_cast<int>(it - std::ranges::begin(employees)));
    }

    /// Sets employee selected in combo box as employee of position
    void setModelData(QWidget * editor,
                      QAbstractItemModel *,
                      const QModelIndex & idx) const override {
        auto combo = static_cast<QComboBox *>(editor);
        int row = combo->currentIndex();
        const employee * empl = row < 0 ? nullptr : &std::ranges::begin(model_.range())[row];
        positions_model_.at(idx).mut()->set_employee(empl);
    }

private:
    using model_t = employee_table_model<mv::ranges::all_t<employee_list &>>;

    positions_model_t & positions_model_;   ///< Reference to table model of positions
    model_t model_;                         ///< Table model for employees that is used in combo box
};
