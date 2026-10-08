// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file position_list_widget.hpp
/// Contains definition of the position_list_widget class.

#pragma once

#include "employee.hpp"
#include "position.hpp"
#include "position_table_model.hpp"
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <cstddef>
#include <cstdint>
#include <type_traits>


/// Widget for viewing and editing list of positions
class position_list_widget: public QWidget {
public:
    /// Constructs widget with specified references to list of positions, list of employees
    /// and parent widget
    position_list_widget(position_list & positions,
                         employee_list & employees,
                         QWidget * parent = nullptr):
    QWidget{parent},
    positions_{positions},
    employees_{employees},
    model_{positions} {
        auto layout = new QVBoxLayout{this};

        view_ = new QTableView;
        view_->setSelectionBehavior(QAbstractItemView::SelectRows);
        view_->setSelectionMode(QAbstractItemView::SingleSelection);
        view_->setDragDropMode(QAbstractItemView::InternalMove);

        // overwrite mode is enabled in table view by default, it drops rows on items
        // instead of dropping them between items
        view_->setDragDropOverwriteMode(false);
        view_->setDropIndicatorShown(true);
        layout->addWidget(view_);

        auto old_mdl = view_->model();
        view_->setModel(&model_);
        delete old_mdl;

        auto buttons_layout = new QHBoxLayout;
        layout->addLayout(buttons_layout);

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

        connect(view_->selectionModel(), &QItemSelectionModel::selectionChanged,
                [this] { update_buttons(); });
        connect(&model_, &QAbstractItemModel::rowsInserted, [this] { update_buttons(); });
        connect(&model_, &QAbstractItemModel::rowsRemoved, [this] { update_buttons(); });
        connect(&model_, &QAbstractItemModel::rowsMoved, [this] { update_buttons(); });

        update_buttons();
    }

    /// Destroys widget, destroys child widgets before models
    ~position_list_widget() {
        delete view_;
    }

private:
    /// Returns index of selected position or SIZE_MAX if there is no selected position
    /// or more than one position is selected
    size_t selected() const {
        QModelIndexList rows = view_->selectionModel()->selectedRows();
        return rows.size() == 1 ? static_cast<size_t>(rows.front().row()) : SIZE_MAX;
    }

    /// Selects position with specified index
    void select(size_t idx) {
        view_->selectionModel()->setCurrentIndex(model_.index(static_cast<int>(idx), 0),
                                                 QItemSelectionModel::ClearAndSelect |
                                                 QItemSelectionModel::Rows);
    }

    /// Adds new position after selected one or at the end of list and selects it
    void add() {
        size_t idx = selected() == SIZE_MAX ? positions_.size() : selected() + 1;
        positions_.emplace(positions_.cbegin() + idx, L"New position", employees_);
        select(idx);
    }

    /// Removes selected position
    void remove() {
        size_t idx = selected();
        positions_.erase(positions_.cbegin() + idx, positions_.cbegin() + idx + 1);
    }

    /// Moves selected position one position up
    void move_up() {
        size_t idx = selected();
        positions_.move(positions_.cbegin() + idx,
                        positions_.cbegin() + idx + 1,
                        positions_.cbegin() + idx - 1);
    }

    /// Moves selected position one position down
    void move_down() {
        size_t idx = selected();
        positions_.move(positions_.cbegin() + idx,
                        positions_.cbegin() + idx + 1,
                        positions_.cbegin() + idx + 2);
    }

    /// Enables or disables buttons depending on selected position
    void update_buttons() {
        size_t idx = selected();
        bool has_sel = idx != SIZE_MAX;
        remove_button_->setEnabled(has_sel);
        up_button_->setEnabled(has_sel && idx > 0);
        down_button_->setEnabled(has_sel && idx + 1 < positions_.size());
    }

    using model_t = std::decay_t<decltype(position_table_model{std::declval<position_list &>()})>;

    position_list & positions_;         ///< Reference to list of positions
    employee_list & employees_;         ///< Reference to list of employees
    model_t model_;                     ///< Table model of positions
    QTableView * view_;                 ///< View of positions
    QPushButton * add_button_;          ///< Button for adding position
    QPushButton * remove_button_;       ///< Button for removing position
    QPushButton * up_button_;           ///< Button for moving position up
    QPushButton * down_button_;         ///< Button for moving position down
};
