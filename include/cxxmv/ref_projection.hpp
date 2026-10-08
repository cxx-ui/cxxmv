// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_projection.hpp
/// Contains definition of the ref_projection class.

#pragma once

#include "projection.hpp"
#include <utility>


namespace mv {


template <typename Base>
requires observable<Base>
class ref_projection: public projection_base {
public:
    /// Constructs view with reference to another range
    ref_projection(Base & b):
        base_{&b} {}

    /// Copy constructor
    ref_projection(const ref_projection & other) = default;

    /// Copy assignment operator
    ref_projection & operator=(const ref_projection & other) = default;

    /// Returns true if mutator is empty
    bool empty() const {
        return base_->emtpy();
    }

    /// Reads value from observable
    decltype(auto) get() const {
        return mv::get(*base_);
    }

    /// Reads value from observable
    decltype(auto) operator*() const {
        return get();
    }

    /// Returns true if value of base observable is null
    bool is_null() const requires nullable_observable<Base> {
        return base_->is_null();
    }

    /// Returns mutator for model value
    auto mut() {
        return mv::mut(*base_);
    }

    /// Returns before changed signal of base observable
    decltype(auto) before_changed() const {
        return base_->before_changed();
    }

    /// Returns after changed signal of base observable
    decltype(auto) after_changed() const {
        return base_->after_changed();
    }

private:
    Base * base_;           ///< Pointer to base observable
};


template <typename Base>
inline constexpr bool enable_borrowed_observable<ref_projection<Base>> = true;


}
