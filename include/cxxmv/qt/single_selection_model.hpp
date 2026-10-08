// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file single_selection_model.hpp
/// Contains definition of the single_selection_model class.

#pragma once

#include "../ranges/all.hpp"
#include "../ranges/element.hpp"
#include "../ranges/projection.hpp"
#include "../signals.hpp"
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QObject>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <utility>


namespace mv::qt {


/// Item selection model that represents element selected in range model
template <ranges::observable_projection Range>
class single_selection_model: public QItemSelectionModel {
public:
    /// Type of model of selected element
    using element_type = ranges::element<Range>;

    /// Constructs selection model for specified range, element model and item model
    single_selection_model(Range rng,
                           element_type & elem,
                           QAbstractItemModel * mdl,
                           QObject * parent = nullptr):
    QItemSelectionModel{mdl, parent},
    rng_{std::move(rng)},
    elem_{elem} {
        QObject::connect(this, &QItemSelectionModel::selectionChanged, [this] {
            auto rows = selectedRows();
            auto it = rows.size() == 1 ?
                 std::ranges::begin(rng_) + static_cast<size_t>(rows.front().row()) :
                 std::ranges::end(rng_);
            elem_.set(it);
        });

        elem_changed_con_ = elem_.after_changed().connect([this] { update_selection(); });
        update_selection();
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

    Range rng_;                                     ///< Range of elements
    element_type & elem_;                           ///< Element model for selected element
    scoped_signal_connection elem_changed_con_;     ///< Connection to element changed signal
};


template <ranges::projectable_observable Range, typename ... Args>
single_selection_model(Range &&, Args && ...) -> single_selection_model<ranges::all_t<Range>>;


}
