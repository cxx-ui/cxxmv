// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file element_handle.hpp
/// Contains declaration of the element_handle type.

#pragma once


namespace mv::ranges {


/// Implementation of element handle type for range model
template <typename Range>
struct element_handle_impl;


/// Type of element handle in range model
template <typename Range>
using element_handle = element_handle_impl<Range>::type;


/// Range model with defined element handle type
template <typename Range>
concept has_element_handle = requires { typename element_handle<Range>; };


}
