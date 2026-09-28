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


namespace mv::ranges {


/// Range model concept
template <typename Range, typename Val>
concept model = observable_as<Range, Val> && std::ranges::output_range<Range, Val>;


}
