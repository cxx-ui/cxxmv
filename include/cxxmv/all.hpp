// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all.hpp
/// Contains definition of the ref_projection projection and all adaptor.

#pragma once

#include "adaptor.hpp"
#include "assign_wrapper.hpp"
#include "projection.hpp"
#include "signal_ref.hpp"


namespace mv {


template <typename Base>
requires observable<Base>
class ref_projection: public projection_base {
    static auto & get_sig_type(Base && b) { return b.changed; }

public:
    /// Constructs view with reference to another range
    ref_projection(Base & b):
        base_{b}, changed{b.changed} {}

    /// Copy constructor
    ref_projection(const ref_projection & other) = default;

    /// Reads value from observable
    decltype(auto) get() const {
        return base_.get();
    }

    /// Reads value from observable
    decltype(auto) operator*() const {
        return get();
    }

    /// Assigns value to model
    template <typename Arg>
    void assign(Arg && val) {
        base_.assign(std::forward<Arg>(val));
    }

    /// Returns wrapper for assigning value to projection
    auto operator*() {
        return assign_wrapper{*this};
    }

    signal_ref<decltype(get_sig_type(std::declval<Base>()))> changed;

private:
    Base & base_;           ///< Reference to base observable
};


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
