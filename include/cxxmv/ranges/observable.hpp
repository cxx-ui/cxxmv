// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file observable.hpp
/// Cotnains definition of the ranges::observable concept.

#pragma once

#include "element_handle.hpp"
#include "../signals.hpp"
#include <concepts>
#include <ranges>
#include <type_traits>


namespace mv::ranges {


/// Observable range concept
template <typename Range>
concept observable = std::ranges::random_access_range<Range> && requires(const Range & obj) {
    { obj.before_inserted() } -> Signal<size_t, size_t>;
    { obj.after_inserted() } -> Signal<size_t, size_t>;
    { obj.before_erased() } -> Signal<size_t, size_t>;
    { obj.after_erased() } -> Signal<size_t, size_t>;
    { obj.before_changed() } -> Signal<std::ranges::iterator_t<const std::remove_reference_t<Range>>>;
    { obj.after_changed() } -> Signal<std::ranges::iterator_t<const std::remove_reference_t<Range>>>;
};


/// Concept for observable range values of which can be obtained and used to construct another type
template <typename Range, typename Val>
concept observable_as = observable<Range> &&
                        std::constructible_from<Val, std::ranges::range_value_t<Range>>;


/// Borrowed observable range is an observable range that can be safely taken/stored by value
template <typename T>
concept borrowed_observable = observable<T> && std::ranges::borrowed_range<T>;


/// Observable range with support for move operations
template <typename Range>
concept observable_with_move = observable<Range> && requires(const Range & obj) {
    { obj.before_moved() } -> Signal<size_t, size_t, size_t>;
    { obj.after_moved() } -> Signal<size_t, size_t, size_t>;
};


/// Observable range with element handle support
template <typename Range>
concept observable_with_handle = requires(Range & obj) {
    /// Type of element handle for range
    typename element_handle<Range>;

    /// Returns handle of element at specified index
    { obj.handle_at(std::declval<size_t>()) } -> std::convertible_to<element_handle<Range>>;

    /// Returns value of element pointed by specified handle
    { obj.get(std::declval<const element_handle<Range> &>()) };
};


/// Observable range with element handle support for type
template <typename Range, typename Value>
concept observable_with_handle_as = observable_with_handle<Range> && requires (Range & obj) {
    /// Returns value of element pointed by specified handle
    { obj.get(std::declval<const element_handle<Range> &>()) } -> std::convertible_to<Value>;
};


}
