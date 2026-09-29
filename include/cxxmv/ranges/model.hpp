// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file model.hpp
/// Contains definition of the ranges::model concept

#pragma once

#include "observable.hpp"
#include <ranges>
#include <utility>


namespace mv::ranges {


/// Range model concept
template <typename Range, typename Val>
concept model = observable_as<Range, Val> &&
                std::ranges::output_range<Range, Val>;


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


}
