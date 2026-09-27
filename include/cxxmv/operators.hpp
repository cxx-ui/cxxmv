// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file operators.hpp
/// Contains definition of operators for observable values.

#pragma once

#include "transform.hpp"
#include "zip_transform.hpp"
#include <utility>


namespace mv {


/// Generates observable projection from projectable observable and value with operator +
/// as transform function
template <projectable_observable Observable, typename Value>
auto operator+(Observable && obs, const Value & val) {
    auto transform_func = [val](auto && obj_val) {
        return obj_val + val;
    };
    return std::forward<Observable>(obs) | transform(transform_func);
}


/// Generates observable projection from two projectable observables
/// with operator + as get transform function
template <projectable_observable Observable1, projectable_observable Observable2>
auto operator+(Observable1 && obs1, Observable2 && obs2) {
    auto transform_fn = [](auto && first, auto && second) {
        return first + second;
    };

    return zip_transform(transform_fn,
                         std::forward<Observable1>(obs1),
                         std::forward<Observable2>(obs2));
}



}
