// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee.hpp
/// Contains definition of the employee class.

#pragma once

#include <cxxmv/vector.hpp>
#include <cxxmv/ranges/element.hpp>
#include <string>
#include <utility>


/// Represents employee with first and last name
struct employee {
public:
    /// Constructs employee
    employee(std::wstring fname, std::wstring lname):
        first_name_{fname}, last_name_{lname} {}

    auto & first_name() const { return first_name_; }
    void set_first_name(std::wstring val) { first_name_ = std::move(val); }

    auto & last_name() const { return last_name_; }
    void set_last_name(std::wstring val) { last_name_ = std::move(val); }

    std::wstring full_name() const {
        auto res = first_name();
        if (!last_name().empty()) {
            res += L" " + last_name();
        }

        return res;
    }

    void set_full_name(const std::wstring & val) {
        auto pos = val.find_first_of(L" ");
        if (pos == std::wstring::npos) {
            set_first_name(val);
            set_last_name({});
        } else {
            set_first_name(val.substr(0, pos));
            set_last_name(val.substr(pos + 1));
        }
    }

    bool operator==(const employee & other) const {
        return first_name() == other.first_name() && last_name() == other.last_name();
    }

private:
    std::wstring first_name_;
    std::wstring last_name_;
};


/// List of employees
using employee_list = mv::vector<employee>;


/// Employee handle
using employee_handle = employee_list::handle;


/// Employee reference
using employee_ref = decltype(std::declval<employee_list &>()
    | mv::ranges::element(std::declval<employee_list &>().handle_at(0)));

