// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file model.hpp
/// Contains definition of the ranges::model concept

#pragma once

#include "observable.hpp"
#include "../model.hpp"
#include <ranges>
#include <utility>


namespace mv::ranges {



/// Range model concept
template <typename Range, typename Val>
concept model = observable_as<Range, Val> && requires (Range & mdl, size_t idx) {
    /// Starts mutating element at specified index
    { mdl.mut(idx) } -> mutator<Val>;

    /// Starts mutating element pointed by specified iterator
    { mdl.mut(mdl.begin()) } -> mutator<Val>;
};


/// Range model with insert support
template <typename Range, typename Val>
concept model_with_insert = model<Range, Val> &&
                            requires(Range & mdl, std::ranges::iterator_t<Range> pos) {
    /// Inserts range of elements into model
    mdl.insert(pos, std::declval<const Val *>(), std::declval<const Val *>());
};


/// Range model with erase support
template <typename Range, typename Val>
concept model_with_erase = model<Range, Val> &&
                           requires(Range & mdl, std::ranges::iterator_t<Range> pos) {
    /// Removes range of elements from model
    mdl.erase(pos, pos);
};


/// Range model with move support
template <typename Range, typename Val>
concept model_with_move = model<Range, Val> &&
                          requires(Range & mdl, std::ranges::iterator_t<Range> pos) {
    /// Moves elements in model
    mdl.move(pos, pos, pos);
};


/// Range model with element handle support
template <typename Range>
concept model_with_handle = observable_with_handle<Range> && requires(Range & obj) {
    /// Starts mutating of element pointed by specified handle
    { obj.mut(std::declval<const element_handle<Range> &>()) };
};


/// Range model with element handle support for type
template <typename Range, typename Value>
concept model_with_handle_of = model_with_handle<Range> && requires (Range & obj) {
    /// Starts mutating of element pointed by specified handle
    { obj.get(std::declval<const element_handle<Range> &>()) } -> mutator<Value>;
};


}
