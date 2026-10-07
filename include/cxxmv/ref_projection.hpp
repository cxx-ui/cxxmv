// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_projection.hpp
/// Contains definition of the ref_projection class.

#pragma once

#include "projection.hpp"
#include "signal_ref.hpp"
#include <utility>


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

    /// Returns true if mutator is empty
    bool empty() const {
        return base_.emtpy();
    }

    /// Reads value from observable
    decltype(auto) get() const {
        return base_.get();
    }

    /// Reads value from observable
    decltype(auto) operator*() const {
        return get();
    }

    /// Returns true if value of base observable is null
    bool is_null() const requires nullable_observable<Base> {
        return base_.is_null();
    }

    /// Returns mutator for model value
    auto mut() {
        return base_.mut();
    }

    signal_ref<decltype(get_sig_type(std::declval<Base>()))> changed;

private:
    Base & base_;           ///< Reference to base observable
};


template <typename Base>
inline constexpr bool enable_borrowed_observable<ref_projection<Base>> = true;


}
