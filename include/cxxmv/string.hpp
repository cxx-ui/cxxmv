// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file string.hpp
/// Contains definitions of string model types and related concepts.

#pragma once

#include "basic_model.hpp"
#include <string>


namespace mv {


template <typename Observable>
concept observable_as_string = observable_as<Observable, std::string>;


/// String model
template <typename Char>
using basic_string = basic_model<std::basic_string<Char>>;


using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u8string = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;


static_assert(model_of<string, std::string>);
static_assert(model_of<wstring, std::wstring>);
static_assert(model_of<u8string, std::u8string>);
static_assert(model_of<u16string, std::u16string>);
static_assert(model_of<u32string, std::u32string>);


}
