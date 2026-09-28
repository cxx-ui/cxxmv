// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection class.

#pragma once

#include "../signal_ref.hpp"
#include "move_signal_refs.hpp"
#include "projection.hpp"
#include <concepts>
#include <ranges>
#include <utility>


namespace mv::ranges {


/// Base class of owning_projection containing base observable range
template <typename Base>
struct owning_projection_base {
    Base base_;             ///< Base observable range
};


/// Projection that owns range passed to it
template <typename Base>
requires observable<Base> && std::movable<Base>
class owning_projection: public projection_base,
                         private owning_projection_base<Base>,
                         public move_signal_refs<Base> {
public:
    /// Constructs projection owning specified observable range
    owning_projection(Base && b):
        owning_projection_base<Base>{std::move(b)},
        move_signal_refs<Base>{this->base_},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Move constructor
    owning_projection(owning_projection && other):
        owning_projection_base<Base>{std::move(other.base_)},
        move_signal_refs<Base>{this->base_},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Returns const iterator pointing to the first element
    auto begin() const { return std::ranges::begin(std::as_const(this->base_)); }

    /// Returns const iterator pointing to one past the last element
    auto end() const { return std::ranges::end(std::as_const(this->base_)); }

    /// Returns iterator pointing to the first element
    auto begin() { return std::ranges::begin(this->base_); }

    /// Returns iterator pointing to one past the last element
    auto end() { return std::ranges::end(this->base_); }

    /// Returns number of elements
    auto size() const { return std::ranges::size(std::as_const(this->base_)); }

    /// The signal is emitted before items added
    signal_ref<decltype(Base::before_inserted)> before_inserted;

    /// The signal is emitted after items added
    signal_ref<decltype(Base::after_inserted)> after_inserted;

    /// The signal is emitted before items removed
    signal_ref<decltype(Base::before_erased)> before_erased;

    /// The signal is emitted after items removed
    signal_ref<decltype(Base::after_erased)> after_erased;

    /// The signal is emitted before item is changed
    signal_ref<decltype(Base::before_changed)> before_changed;

    /// The signal is emitted after item is changed
    signal_ref<decltype(Base::after_changed)> after_changed;
};


}
