// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file observable.hpp
/// Cotnains definition of the ranges::observable concept.

#pragma once

#include "../signals.hpp"
#include <concepts>
#include <cstddef>
#include <ranges>
#include <type_traits>


namespace mv::ranges {


template <typename Range>
using const_range_iterator = std::ranges::iterator_t<const std::remove_reference_t<Range>>;


/// Observable range concept
template <typename Range>
concept observable = std::ranges::random_access_range<Range> && requires(Range obj) {
    { obj.before_inserted() } -> Signal<const_range_iterator<Range>, size_t>;
    { obj.after_inserted() } -> Signal<const_range_iterator<Range>, const_range_iterator<Range>>;
    { obj.before_erased() } -> Signal<const_range_iterator<Range>, const_range_iterator<Range>>;
    { obj.after_erased() } -> Signal<const_range_iterator<Range>, size_t>;
    { obj.before_changed() } -> Signal<const_range_iterator<Range>>;
    { obj.after_changed() } -> Signal<const_range_iterator<Range>>;
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



}
