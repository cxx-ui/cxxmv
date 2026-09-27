// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file adaptor.hpp
/// Contains definition of the adaptor class for building projections.

#pragma once

#include "projection.hpp"


namespace mv {


/// Observable view adaptor closure that can be combined with | operator
template <typename Callable>
class adaptor_closure {
public:
    constexpr adaptor_closure(const Callable & c):
    callable_{c} {};

    template <projectable_observable Observable>
    auto constexpr operator()(Observable && obj) const {
        return callable_(std::forward<Observable>(obj));
    }

private:
    Callable callable_;
};


/// Generic adaptor for observable view.
template <typename Callable>
requires std::is_default_constructible_v<Callable>
class adaptor {
public:
    constexpr adaptor(const Callable & c = {}) {};

    template <typename ... Args>
    constexpr auto operator()(Args && ... args) const {
        if constexpr (std::is_invocable_v<Callable, Args...>) {
            // adaptor(range, args...) form
            return Callable{}(std::forward<Args>(args)...);
        } else {
            // adaptor(args...)(range) form
            auto make_fn = [args...]<typename Observable>(Observable && obj) {
                return Callable{}(std::forward<Observable>(obj), args...);
            };

            return adaptor_closure{make_fn};
        }
    }
};


template <projectable_observable Observable, typename Callable>
constexpr auto operator|(Observable && obj, const adaptor_closure<Callable> & ac) {
    return ac(std::forward<Observable>(obj));
}


}
