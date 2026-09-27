// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file test_user.hpp
/// Contains definition of the test_user class.

#pragma once

#include <string>


/// Helper class for testing cxxmv library
class test_user {
public:
    test_user(const std::string f, const std::string l):
        first_name_{std::move(f)}, last_name_{std::move(l)} {}

    const std::string & first_name() const { return first_name_; }
    const std::string & last_name() const { return last_name_; }

    void set_first_name(std::string f) { first_name_ = std::move(f); }
    void set_last_name(std::string l) { last_name_ = std::move(l); }

private:
    std::string first_name_;
    std::string last_name_;
};

