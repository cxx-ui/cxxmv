// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file employee.cpp
/// Contains implementation of the employee class.

#include "employee.hpp"


std::wstring employee::full_name() const {
    auto res = first_name();
    if (!last_name().empty()) {
        res += L" " + last_name();
    }

    return res;
}


void employee::set_full_name(const std::wstring & val) {
    auto pos = val.find_first_of(L" ");
    if (pos == std::wstring::npos) {
        set_first_name(val);
        set_last_name({});
    } else {
        set_first_name(val.substr(0, pos));
        set_last_name(val.substr(pos + 1));
    }
}


std::string employee_type_to_string(employee_type type) {
    switch (type) {
    case employee_type::permanent:
        return "Permanent";
    case employee_type::contractor:
        return "Contractor";
    default:
        assert(false && "unknown employee type");
        return {};
    }
}
