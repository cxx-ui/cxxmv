// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee_list_widget.hpp
/// Contains definition of the employee_list_widget class.

#pragma once

#include "employee.hpp"
#include "employee_table_model.hpp"
#include "employee_widget.hpp"
#include <cxxmv/qt/selected_element_model.hpp>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <cstddef>
#include <cstdint>
#include <type_traits>


/// Widget for viewing and editing list of employees
class employee_list_widget: public QWidget {
public:
    /// Constructs widget with specified reference to list of employees and parent widget
    employee_list_widget(employee_list & employees, QWidget * parent = nullptr):
    QWidget{parent},
    employees_{employees},
    model_{employees},
    sel_{employees, &model_} {
        auto layout = new QHBoxLayout{this};
        auto list_layout = new QVBoxLayout;
        layout->addLayout(list_layout);

        view_ = new QTableView;
        view_->setSelectionBehavior(QAbstractItemView::SelectRows);
        view_->setSelectionMode(QAbstractItemView::SingleSelection);
        view_->setDragDropMode(QAbstractItemView::InternalMove);

        // overwrite mode is enabled in table view by default, it drops rows on items
        // instead of dropping them between items
        view_->setDragDropOverwriteMode(false);
        view_->setDropIndicatorShown(true);
        list_layout->addWidget(view_);

        auto old_mdl = view_->model();
        view_->setModel(&model_);
        delete old_mdl;

        auto old_sel = view_->selectionModel();
        view_->setSelectionModel(&sel_);
        delete old_sel;

        auto buttons_layout = new QHBoxLayout;
        list_layout->addLayout(buttons_layout);

        add_button_ = new QPushButton{"Add"};
        remove_button_ = new QPushButton{"Remove"};
        up_button_ = new QPushButton{"Up"};
        down_button_ = new QPushButton{"Down"};

        buttons_layout->addWidget(add_button_);
        buttons_layout->addWidget(remove_button_);
        buttons_layout->addWidget(up_button_);
        buttons_layout->addWidget(down_button_);
        buttons_layout->addStretch();

        connect(add_button_, &QPushButton::clicked, [this] { add(); });
        connect(remove_button_, &QPushButton::clicked, [this] { remove(); });
        connect(up_button_, &QPushButton::clicked, [this] { move_up(); });
        connect(down_button_, &QPushButton::clicked, [this] { move_down(); });

        connect(&sel_, &QItemSelectionModel::selectionChanged, [this] { update_buttons(); });
        connect(&model_, &QAbstractItemModel::rowsInserted, [this] { update_buttons(); });
        connect(&model_, &QAbstractItemModel::rowsRemoved, [this] { update_buttons(); });
        connect(&model_, &QAbstractItemModel::rowsMoved, [this] { update_buttons(); });

        edit_ = new employee_widget{sel_.element()};
        layout->addWidget(edit_);

        update_buttons();
    }

    /// Destroys widget, destroys child widgets before models
    ~employee_list_widget() {
        delete view_;
        delete edit_;
    }

private:
    /// Adds new employee after selected one or at the end of list and selects it
    void add() {
        auto & elem = sel_.element();
        auto pos = elem.is_null() ? employees_.cend() : elem.iterator() + 1;
        elem.set(employees_.emplace(pos, L"New", L"Employee"));
    }

    /// Removes selected employee
    void remove() {
        auto it = sel_.element().iterator();
        employees_.erase(it, it + 1);
    }

    /// Moves selected employee one position up
    void move_up() {
        auto it = sel_.element().iterator();
        employees_.move(it, it + 1, it - 1);
    }

    /// Moves selected employee one position down
    void move_down() {
        auto it = sel_.element().iterator();
        employees_.move(it, it + 1, it + 2);
    }

    /// Enables or disables buttons depending on selected employee
    void update_buttons() {
        auto & elem = sel_.element();
        bool has_sel = !elem.is_null();
        remove_button_->setEnabled(has_sel);
        up_button_->setEnabled(has_sel && elem.iterator() != employees_.begin());
        down_button_->setEnabled(has_sel && elem.iterator() + 1 != employees_.end());
    }

    using model_t = std::decay_t<decltype(employee_table_model{std::declval<employee_list &>()})>;

    employee_list & employees_;                             ///< Reference to list of employees
    model_t model_;                                         ///< Table model of employees
    mv::qt::selected_element_model<employee_list> sel_;     ///< Selected employee model
    QTableView * view_;                                     ///< View of employees
    QWidget * edit_;                                        ///< Widget for editing selected employee
    QPushButton * add_button_;                              ///< Button for adding employee
    QPushButton * remove_button_;                           ///< Button for removing employee
    QPushButton * up_button_;                               ///< Button for moving employee up
    QPushButton * down_button_;                             ///< Button for moving employee down
};
