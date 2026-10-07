// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file position.hpp
/// Contains definition of the position class.

#pragma once

#include "employee.hpp"
#include <cxxmv/ranges/element.hpp>
#include <cxxmv/signals.hpp>
#include <cstddef>
#include <string>
#include <utility>


/// Represents position with name and employee occupying it
class position {
public:
    /// Constructs position with specified name and model of employee in list of employees
    position(std::wstring name, employee_ref empl):
        name_{std::move(name)}, empl_{std::move(empl)} {}

    auto & name() const { return name_; }
    void set_name(std::wstring val) { name_ = std::move(val); }

    const auto & employee() const { return empl_; }
    void set_employee(employee_ref empl) { empl_ = std::move(empl); }

private:
    std::wstring name_;         ///< Name of position
    employee_ref empl_;         ///< Employee occupying position
};


class position_list: public mv::vector<position> {
public:
    /// Constructs list of positions for specified list of employees
    position_list(employee_list & elist):
    employees_{elist} {
        employees_before_changed_con_ = employees_.before_changed().connect([this](size_t idx) {
            for_each_position(idx, idx + 1, [this](size_t pos) { before_changed()(pos); });
        });

        employees_after_changed_con_ = employees_.after_changed().connect([this](size_t idx) {
            for_each_position(idx, idx + 1, [this](size_t pos) { after_changed()(pos); });
        });

        employees_before_erased_con_ = employees_.before_erased().connect(
        [this](size_t idx, size_t count) {
            for_each_position(idx, idx + count, [this](size_t pos) {
                mut(pos)->set_employee(employees_ | mv::ranges::element(employee_handle{}));
            });
        });
    }

    /// List is not copyable
    position_list(const position_list &) = delete;

    /// List is not movable
    position_list(position_list &&) = delete;

private:
    /// Calls function with index of each position occupied by employee
    /// with index in range [first, last)
    template <typename Fn>
    void for_each_position(size_t first, size_t last, Fn fn) const {
        for (size_t pos = 0; pos < size(); ++pos) {
            const auto & empl = (*this)[pos].employee();
            if (empl.is_null()) {
                continue;
            }

            for (size_t idx = first; idx < last; ++idx) {
                if (&empl.get() == &employees_[idx]) {
                    fn(pos);
                    break;
                }
            }
        }
    }

    employee_list & employees_;                                 ///< Reference to list of employes
    mv::scoped_signal_connection employees_before_changed_con_; ///< Connection to before_changed
    mv::scoped_signal_connection employees_after_changed_con_;  ///< Connection to after_changed
    mv::scoped_signal_connection employees_before_erased_con_;  ///< Connection to before_erased
};
