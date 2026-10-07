// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection projection.

#pragma once

#include "projection.hpp"
#include "signal_ref.hpp"
#include <concepts>
#include <utility>


namespace mv {


template <typename Base>
requires observable<Base> && std::movable<Base>
class owning_projection: public projection_base {
public:
    /// Constructs projection owning specified observable
    owning_projection(Base && b):
        base_{std::move(b)}, changed{base_.changed} {}

    /// Move constructor
    owning_projection(owning_projection && other):
        base_{std::move(other.base_)}, changed{base_.changed} {}

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

    /// Returns model value mutator
    auto mut() {
        return base_.mut();
    }

private:
    Base base_;             ///< Base observable

public:
    signal_ref<decltype(Base::changed)> changed;
};


}
