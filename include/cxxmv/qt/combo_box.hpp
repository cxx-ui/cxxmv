
/// \file combo_box.hpp
/// Contains definition of the combo_box class.

#pragma once

#include "../projection.hpp"
#include "../all.hpp"
#include <QComboBox>


namespace mv::qt {


/// Combo box view control for model containing selected index
template <model_of<int> Model>
class combo_box: public QComboBox {
public:
    /// Constructs new combo box with specified model
    combo_box(Model mdl):
    mdl_{std::move(mdl)} {
        mdl_.after_changed().connect([this] {
            update_value();
        });

        // activated is emitted only when user selects an item
        connect(this, &QComboBox::activated, [this](auto idx) {
            mut(mdl_).ref() = idx;

            // resulting value in model may be different from value in combo box
            update_value();
        });

        // adding first item changes current index to 0, restoring value from model
        connect(model(), &QAbstractItemModel::rowsInserted, this, [this] {
            update_value();
        });

        update_value();
    }

private:
    /// Update combo box with value from model
    void update_value() {
        if (is_null(mdl_)) {
            setEnabled(false);
            setCurrentIndex(-1);
            return;
        }

        setEnabled(true);
        setCurrentIndex(get(mdl_));
    }

    Model mdl_;                     ///< Selected value model
};


template <projectable_observable Model>
combo_box(Model && mdl) -> combo_box<all_t<Model>>;


}
