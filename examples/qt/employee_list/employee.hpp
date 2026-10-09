// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee.hpp
/// Contains definition of the employee class.

#pragma once

#include <cxxmv/vector.hpp>
#include <string>
#include <utility>


/// Represents employee type
enum class employee_type {
    permanent,
    contractor
};


/// Represents employee with first and last name
struct employee {
public:
    /// Constructs employee
    employee(std::wstring fname, std::wstring lname, employee_type type):
        first_name_{fname}, last_name_{lname}, type_{type} {}

    auto & first_name() const { return first_name_; }
    void set_first_name(std::wstring val) { first_name_ = std::move(val); }

    auto & last_name() const { return last_name_; }
    void set_last_name(std::wstring val) { last_name_ = std::move(val); }

    employee_type type() const { return type_; }
    void set_type(employee_type t) { type_ = t; }

    /// Returns employee full name
    std::wstring full_name() const;

    /// Sets employee first and last name from full name
    void set_full_name(const std::wstring & val);

    /// Returns true if two employes are equal
    bool operator==(const employee & other) const {
        return first_name() == other.first_name() && last_name() == other.last_name();
    }

private:
    std::wstring first_name_;
    std::wstring last_name_;
    employee_type type_;
};


/// List of employees
using employee_list = mv::vector<employee>;


/// Converts employee type to string
std::string employee_type_to_string(employee_type type);
