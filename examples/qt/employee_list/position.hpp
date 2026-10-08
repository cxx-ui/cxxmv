// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file position.hpp
/// Contains definition of the position class.

#pragma once

#include "employee.hpp"
#include <cxxmv/model.hpp>
#include <cxxmv/model_vector.hpp>
#include <cxxmv/signals.hpp>
#include <cstddef>
#include <string>
#include <utility>


/// Represents position with name and employee occupying it
class position {
public:
    /// Constructs position with specified name and pointer to employee in list of employees
    position(std::wstring name,
             employee_list & employees,
             const ::employee * empl = nullptr):
    name_{std::move(name)}, employees_{employees}, empl_{empl} {
        employees_before_changed_con_ = employees_.before_changed().connect([this](const auto & it) {
            if (&*it == empl_) {
                before_changed_();
            }
        });

        employees_after_changed_con_ = employees_.after_changed().connect([this](const auto & it) {
            if (&*it == empl_) {
                after_changed_();
            }
        });

        employees_before_erased_con_ = employees_.before_erased().connect(
        [this](const auto & first, const auto & last) {
            if (!empl_) {
                return;
            }

            for (auto it = first; it != last; ++it) {
                if (&*it == empl_) {
                    set_employee(nullptr);
                    break;
                }
            }
        });
    }

    /// Position is not copyable
    position(const position &) = delete;

    /// Position is not movable
    position(position &&) = delete;

    auto & name() const { return name_; }

    void set_name(std::wstring val) {
        before_changed_();
        name_ = std::move(val);
        after_changed_();
    }

    /// Returns pointer to employee occupying position or nullptr if position is vacant
    const ::employee * employee() const { return empl_; }

    void set_employee(const ::employee * empl) {
        before_changed_();
        empl_ = empl;
        after_changed_();
    }

    /// Returns signal emitted before position is changed
    auto & before_changed() const { return before_changed_; }

    /// Returns signal emitted after position is changed
    auto & after_changed() const { return after_changed_; }

private:
    std::wstring name_;                 ///< Name of position
    employee_list & employees_;         ///< Reference to list of employees
    const ::employee * empl_;           ///< Pointer to employee occupying position

    mutable mv::signal<void ()> before_changed_;    ///< Before changed signal
    mutable mv::signal<void ()> after_changed_;     ///< After changed signal

    mv::scoped_signal_connection employees_before_changed_con_; ///< Connection to employees before_changed
    mv::scoped_signal_connection employees_after_changed_con_;  ///< Connection to employees after_changed
    mv::scoped_signal_connection employees_before_erased_con_;  ///< Connection to employees before_erased
};


static_assert(mv::model<position>);


/// List of positions
using position_list = mv::model_vector<position>;
