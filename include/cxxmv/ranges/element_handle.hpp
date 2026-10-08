// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file element_handle.hpp
/// Contains declaration of the element_handle type.

#pragma once

#include <concepts>
#include <cstddef>


namespace mv::ranges {


/// Element handle concept
template <typename Handle>
concept range_element_handle = std::copyable<Handle> && requires(const Handle & ch) {
    /// Converts to true if handle is valid
    static_cast<bool>(ch);
};


/// Implementation of element handle type for range model
template <typename Range>
struct element_handle_impl;


/// Type of element handle in range model
template <typename Range>
using element_handle = element_handle_impl<Range>::type;


}
