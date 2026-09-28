// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all.hpp
/// Contains definition of the all range adaptor.

#pragma once

#include "adaptor.hpp"
#include "owning_projection.hpp"
#include "projection.hpp"
#include "ref_projection.hpp"
#include <type_traits>
#include <utility>


namespace mv::ranges {


inline constexpr auto all = adaptor_closure {
    []<projectable_observable Range>(Range && r) {
        if constexpr (observable_projection<std::decay_t<Range>>) {
            return std::forward<Range>(r);
        } else if constexpr (requires { owning_projection{std::forward<Range>(r)}; }) {
            return owning_projection{std::forward<Range>(r)};
        } else {
            return ref_projection{std::forward<Range>(r)};
        }
    }
};


template <projectable_observable Range>
using all_t = decltype(all(std::declval<Range>()));


}
