// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selected_element_model.hpp
/// Contains definition of the selected_element_model class.

#pragma once

#include "../ranges/element.hpp"
#include "../signals.hpp"
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QObject>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ranges>


namespace mv::qt {


/// Item selection model that represents element selected in range model
template <typename Range>
requires requires { sizeof(ranges::element<ranges::all_t<Range &>>); }
class selected_element_model: public QItemSelectionModel {
public:
    /// Type of model of selected element
    using element_type = ranges::element<ranges::all_t<Range &>>;

    /// Constructs selection model for specified item model and range
    selected_element_model(Range & rng, QAbstractItemModel * mdl, QObject * parent = nullptr):
    QItemSelectionModel{mdl, parent},
    rng_{rng},
    elem_{rng} {
        QObject::connect(this, &QItemSelectionModel::selectionChanged, [this] {
            auto rows = selectedRows();
            auto it = rows.size() == 1 ?
                 std::ranges::begin(rng_) + static_cast<size_t>(rows.front().row()) :
                 std::ranges::end(rng_);
            elem_.set(it);
        });

        elem_changed_con_ = elem_.after_changed().connect([this] { update_selection(); });
    }

    /// Returns model of selected element
    element_type & element() {
        return elem_;
    }

private:
    /// Updates selected row and current row in QItemSelectionModel
    void update_selection() {
        if (elem_.is_null()) {
            clearCurrentIndex();
            clearSelection();
        } else {
            auto row = elem_.iterator() - std::ranges::begin(rng_);
            QModelIndex mdl_idx = model()->index(static_cast<int>(row), 0);
            assert(mdl_idx.isValid() && "element index is out of range of item model");
            setCurrentIndex(mdl_idx, QItemSelectionModel::ClearAndSelect |
                                     QItemSelectionModel::Rows);
        }
    }

    Range & rng_;                                   ///< Range of elements
    element_type elem_;                             ///< Element model for selected element
    scoped_signal_connection elem_changed_con_;     ///< Connection to element changed signal
};


}
