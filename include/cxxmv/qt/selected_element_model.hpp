// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selected_element_model.hpp
/// Contains definition of the selected_element_model class.

#pragma once

#include "../ranges/element_handle.hpp"
#include "../ranges/element_model.hpp"
#include "../signals.hpp"
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QObject>
#include <cassert>
#include <cstddef>
#include <cstdint>


namespace mv::qt {


/// Item selection model that represents element selected in range model
template <typename Range>
requires requires { sizeof(ranges::element_model<Range>); } && ranges::observable_with_handle<Range>
class selected_element_model: public QItemSelectionModel {
public:
    /// Constructs selection model for specified item model and range
    selected_element_model(Range & rng, QAbstractItemModel * mdl, QObject * parent = nullptr):
    QItemSelectionModel{mdl, parent},
    rng_{rng},
    elem_{rng} {
        QObject::connect(this, &QItemSelectionModel::selectionChanged, [this] {
            size_t idx = selected_row();
            if (elem_.index() != idx) {
                elem_.set(idx == SIZE_MAX ? ranges::element_handle<Range>{} : rng_.handle_at(idx));
            }
        });

        elem_changed_con_ = elem_.after_changed().connect([this] { update_selection(); });
    }

    /// Returns model of selected element
    ranges::element_model<Range> & element() {
        return elem_;
    }

private:
    /// Returns selected row or SIZE_MAX if there is no selected row
    /// or more than one row is selected
    size_t selected_row() const {
        QModelIndexList rows = selectedRows();
        return rows.size() == 1 ? static_cast<size_t>(rows.front().row()) : SIZE_MAX;
    }

    /// Updates selected row and current row in QItemSelectionModel
    void update_selection() {
        size_t idx = elem_.index();
        if (selected_row() == idx) {
            return;
        }

        if (idx == SIZE_MAX) {
            clearCurrentIndex();
            clearSelection();
        } else {
            QModelIndex mdl_idx = model()->index(static_cast<int>(idx), 0);
            assert(mdl_idx.isValid() && "element index is out of range of item model");
            setCurrentIndex(mdl_idx, QItemSelectionModel::ClearAndSelect |
                                     QItemSelectionModel::Rows);
        }
    }

    Range & rng_;                                   ///< Range of elements
    ranges::element_model<Range> elem_;             ///< Element model for selected element
    scoped_signal_connection elem_changed_con_;     ///< Connection to element changed signal
};


}
