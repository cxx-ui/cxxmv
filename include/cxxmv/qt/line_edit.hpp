// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file line_edit.hpp
/// Contains definition of the line_edit class.

#pragma once

#include "../projection.hpp"
#include "../all.hpp"
#include <QLineEdit>


namespace mv::qt {


/// Line view control for text model containing std::string or std::wstring
template <projectable_observable Model>
class line_edit: public QLineEdit {
public:
    static constexpr bool is_read_only = !projectable_model<Model, std::string> &&
                                         !projectable_model<Model, std::wstring>;

    /// Constructs line view with specified reference to model and parent widget
    line_edit(Model mdl, QWidget * parent = nullptr):
    QLineEdit{parent},
    mdl_{mdl} {
        this->setReadOnly(is_read_only);

        if constexpr (!is_read_only) {
            this->connect(this, &QLineEdit::textChanged, [this] {
                if (is_updating_ || mv::is_null(mdl_)) {
                    return;
                }

                if constexpr (model_of<Model, std::string>) {
                    mdl_.mut().ref() = this->text().toStdString();
                } else if constexpr (model_of<Model, std::wstring>) {
                    mdl_.mut().ref() = this->text().toStdWString();
                }

                // resulting value in model may be different from value in line edit.
                // We need update line edit value in such case.
                this->update_value();
            });
        }

        con_ = mdl_.changed().connect([this] { update_value(); });

        update_value();
    }

private:
    /// Returns QString containing in model
    QString get_model_val() const {
        if constexpr (observable_as<Model, std::string>) {
            return QString::fromStdString(mdl_.get());
        } else if constexpr (observable_as<Model, std::wstring>) {
            return QString::fromStdWString(mdl_.get());
        }
    }

    /// Updates line edit value with value from model, disables line edit if model value is null
    void update_value() {
        bool null = mv::is_null(mdl_);
        if constexpr (nullable_observable<Model>) {
            setEnabled(!null);
        }

        QString s = null ? QString{} : get_model_val();
        if (s != text()) {
            is_updating_ = true;
            setText(s);
            is_updating_ = false;
        }
    }

    Model mdl_;                         ///< Model
    scoped_signal_connection con_;      ///< Connection to model changed signal
    bool is_updating_ = false;          ///< Is view being updated now?
};


template <projectable_observable Model>
line_edit(Model && mdl) -> line_edit<all_t<Model>>;


}
