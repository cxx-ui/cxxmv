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
    /// Changed signal emitted after observable value is changed
    { obj.changed } -> Signal<>;
};


/// Observable concept with dereference operator to access value
template <typename Observable, typename Value>
concept observable_as = observable<Observable> && requires(const Observable & obj) {
    /// Returns value of observable
    { obj.get() } -> std::convertible_to<Value>;

    /// Dereference operator for retreiving value of observable
    { *obj } -> std::convertible_to<Value>;
};


template<typename T>
inline constexpr bool enable_borrowed_observable = false;

/// Borrowed observable is an observable that can be safely taken/stored by value
template <typename Observable>
concept borrowed_observable = observable<Observable> &&
                              (std::is_lvalue_reference_v<Observable> ||
                               enable_borrowed_observable<std::remove_cvref_t<Observable>>);


}
