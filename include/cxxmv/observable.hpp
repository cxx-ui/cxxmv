// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file observable.hpp
/// Contains definition of the observable concept.

#pragma once

#include "signals.hpp"
#include <concepts>


namespace mv {


/// Observable concept
template <typename Observable>
concept observable = requires(const Observable & obj) {
    /// Returns signal emitted before observable value is changed
    { obj.before_changed() } -> Signal<>;

    /// Returns signal emitted after observable value is changed
    { obj.after_changed() } -> Signal<>;
};


/// Nullable observable with method for checking if value is available
template <typename Observable>
concept nullable_observable = observable<Observable> && requires (const Observable & obj) {
    /// Returns true if observable value is null and can't be accessed
    { obj.is_null() } -> std::convertible_to<bool>;
};


/// Observable concept with dereference operator to access value of specified type
template <typename Observable, typename Value>
concept observable_as = observable<Observable> && requires(const Observable & obj) {
    /// Returns value of observable
    { obj.get() } -> std::convertible_to<Value>;

    /// Dereference operator for retreiving value of observable
    { *obj } -> std::convertible_to<Value>;
};


/// Nullable observable of specified value type
template <typename Observable, typename Value>
concept nullable_observable_as = observable_as<Observable, Value> &&
                                 nullable_observable<Observable>;


template<typename T>
inline constexpr bool enable_borrowed_observable = false;

/// Borrowed observable is an observable that can be safely taken/stored by value
template <typename Observable>
concept borrowed_observable = observable<Observable> &&
                              (std::is_lvalue_reference_v<Observable> ||
                               enable_borrowed_observable<std::remove_cvref_t<Observable>>);


/// Returns true if observable is nullable and it's value is null
template <observable Observable>
bool is_null(const Observable & obj) {
    if constexpr (nullable_observable<Observable>) {
        return obj.is_null();
    } else {
        return false;
    }
}


}
