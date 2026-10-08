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
#include <cxxmv/ranges/element.hpp>
#include <cxxmv/signals.hpp>
#include <cstddef>
#include <string>
#include <utility>


/// Represents position with name and employee occupying it
class position {
public:
    /// Position mutator
    class mutator {
    public:
        /// Constructs mutator with specified reference to position
        mutator(position & pos):
            pos_{pos} {}

        /// Returns true if mutator is empty
        bool empty() const { return false; }

        /// Returns reference to position
        position & ref() const { return pos_; }

        /// Returns pointer to position
        position * ptr() const { return &pos_; }

        /// Returns pointer to position
        position * operator->() const { return ptr(); }

    private:
        position & pos_;        ///< Reference to position
    };

    /// Constructs position with specified name and employee in list of employees
    position(std::wstring name,
             employee_list & employees,
             const employee_handle & empl = {}):
    name_{std::move(name)}, employees_{employees}, empl_{employees | mv::ranges::element(empl)} {
        connect_employee();

        employees_before_erased_con_ = employees_.before_erased().connect(
        [this](size_t idx, size_t count) {
            if (empl_.is_null()) {
                return;
            }

            for (size_t i = idx; i < idx + count; ++i) {
                if (&empl_.get() == &employees_[i]) {
                    set_employee(employees_ | mv::ranges::element(employee_handle{}));
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

    const auto & employee() const { return empl_; }

    void set_employee(employee_ref empl) {
        before_changed_();
        empl_ = std::move(empl);
        connect_employee();
        after_changed_();
    }

    /// Returns const reference to position
    const position & get() const { return *this; }

    /// Returns const reference to position
    const position & operator*() const { return get(); }

    /// Starts mutating position
    mutator mut() { return {*this}; }

    /// Returns signal emitted before position is changed
    auto & before_changed() const { return before_changed_; }

    /// Returns signal emitted after position is changed
    auto & after_changed() const { return after_changed_; }

private:
    /// Connects to changed signals of employee projection
    void connect_employee() {
        empl_before_changed_con_ = empl_.before_changed().connect([this] { before_changed_(); });
        empl_after_changed_con_ = empl_.after_changed().connect([this] { after_changed_(); });
    }

    std::wstring name_;                 ///< Name of position
    employee_list & employees_;         ///< Reference to list of employees
    employee_ref empl_;                 ///< Employee occupying position

    mutable mv::signal<void ()> before_changed_;    ///< Before changed signal
    mutable mv::signal<void ()> after_changed_;     ///< After changed signal

    mv::scoped_signal_connection empl_before_changed_con_;      ///< Connection to employee before_changed
    mv::scoped_signal_connection empl_after_changed_con_;       ///< Connection to employee after_changed
    mv::scoped_signal_connection employees_before_erased_con_;  ///< Connection to employees before_erased
};


static_assert(mv::model<position>);


/// List of positions
using position_list = mv::model_vector<position>;

