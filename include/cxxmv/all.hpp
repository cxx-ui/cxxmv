// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all.hpp
/// Contains definition of the all adaptor.

#pragma once

#include "adaptor.hpp"
#include "projection.hpp"
#include "ref_projection.hpp"
#include <type_traits>
#include <utility>


namespace mv {


inline constexpr auto all = adaptor_closure {
    []<projectable_observable Observable>(Observable && obj) {
        if constexpr (observable_projection<std::decay_t<Observable>>) {
            return std::forward<Observable>(obj);
        } else {
            return ref_projection{std::forward<Observable>(obj)};
        }
    }
};


template <projectable_observable Observable>
using all_t = decltype(all(std::declval<Observable>()));


}
