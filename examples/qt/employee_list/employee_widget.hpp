// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee_widget.hpp
/// Contains definition of the employee_widget class.

#pragma once

#include "employee.hpp"
#include <cxxmv/all.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/qt/combo_box.hpp>
#include <cxxmv/qt/line_edit.hpp>
#include <QFormLayout>
#include <QWidget>
#include <string>
#include <utility>


/// Widget for viewing and editing employee
template <mv::model_of<employee> Model>
class employee_widget: public QWidget {
public:
    /// Constructs widget with specified employee model and parent widget
    employee_widget(Model mdl, QWidget * parent = nullptr):
    QWidget{parent},
    mdl_{std::move(mdl)} {
        auto layout = new QFormLayout{this};

        auto first_name = mdl_ | mv::transform(
            [](const employee & c) { return c.first_name(); },
            [](employee & c, std::wstring val) { c.set_first_name(std::move(val)); });

        auto last_name = mdl_ | mv::transform(
            [](const employee & c) { return c.last_name(); },
            [](employee & c, std::wstring val) { c.set_last_name(std::move(val)); });

        auto full_name = mdl_ | mv::transform(
            [](const employee & c) { return c.full_name(); },
            [](employee & c, const std::wstring & val) { c.set_full_name(val); });

        auto type = mdl_ | mv::transform(
            [](const employee & c) { return static_cast<int>(c.type()); },
            [](employee & c, int idx) { c.set_type(idx == 0 ? employee_type::permanent :
                                                              employee_type::contractor); });

        layout->addRow("First name:", new mv::qt::line_edit{std::move(first_name)});
        layout->addRow("Last name:", new mv::qt::line_edit{std::move(last_name)});
        layout->addRow("Full name:", new mv::qt::line_edit{std::move(full_name)});

        auto type_select = new mv::qt::combo_box{std::move(type)};
        layout->addRow("Type:", type_select);
        type_select->addItem("Permanent");
        type_select->addItem("Contractor");
    }

private:
    Model mdl_;                 ///< Employee model
};


template <mv::projectable_observable_as<employee> Model>
employee_widget(Model && mdl) -> employee_widget<mv::all_t<Model>>;
