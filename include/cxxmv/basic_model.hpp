// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file basic_model.hpp
/// Contains definition of the basic_model class.

#pragma once

#include "assign_wrapper.hpp"
#include "model.hpp"
#include <initializer_list>
#include <type_traits>


namespace mv {


/// Basic implementation of model for specified value type.
template <typename Value>
class basic_model {
public:
    /// The changed signal is emitted after value is changed in the model
    mutable signal<void()> changed;

    /// Constructs model with stored value constructed from specified arguments
    template <typename ... Args>
    requires (std::constructible_from<Value, Args...>)
    basic_model(Args && ... args):
        value_{std::forward<Args>(args)...} {}

    /// Constructs model with stored value constructed from initializer list
    template <typename T>
    requires (std::constructible_from<Value, std::initializer_list<T>>)
    basic_model(std::initializer_list<T> init):
        value_(init) {}

    /// Returns const reference to stored value
    const Value & get() const {
        return value_;
    }

    /// Returns const reference to stored value
    const Value & operator*() const {
        return get();
    }

    /// Returns const pointer to stored value
    const Value * operator->() const {
        return &get();
    }

    /// Assigns specified argument to stored value. Emits the changed signal 
    /// after assignment.
    template <typename Arg>
    requires (std::is_assignable_v<Value &, Arg>)
    void assign(Arg && arg) {
        value_ = std::forward<Arg>(arg);
        changed();
    }

    /// Returns assignable reference wrapper for stored value
    auto operator*() {
        return assign_wrapper{*this};
    }

private:
    Value value_;           ///< Stored value
};


}
