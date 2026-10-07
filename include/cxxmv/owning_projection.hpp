// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection projection.

#pragma once

#include "projection.hpp"
#include <concepts>
#include <utility>


namespace mv {


template <typename Base>
requires observable<Base> && std::movable<Base>
class owning_projection: public projection_base {
public:
    /// Constructs projection owning specified observable
    owning_projection(Base && b):
        base_{std::move(b)} {}

    /// Copy constructor
    owning_projection(const owning_projection & other) = default;

    /// Move constructor
    owning_projection(owning_projection && other) = default;

    /// Copy assignment operator
    owning_projection & operator=(const owning_projection & other) = default;

    /// Move assignment operator
    owning_projection & operator=(owning_projection && other) = default;

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

    /// Returns before changed signal of base observable
    decltype(auto) before_changed() const {
        return base_.before_changed();
    }

    /// Returns after changed signal of base observable
    decltype(auto) after_changed() const {
        return base_.after_changed();
    }

private:
    Base base_;             ///< Base observable
};


}
