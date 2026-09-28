// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection class.

#pragma once

#include "../signal_ref.hpp"
#include "projection.hpp"
#include <concepts>
#include <ranges>
#include <utility>


namespace mv::ranges {


template <typename Base>
requires observable<Base> && std::movable<Base>
class owning_projection: public projection_base {
public:
    /// Constructs projection owning specified observable range
    owning_projection(Base && b):
        base_{std::move(b)},
        before_inserted{base_.before_inserted},
        after_inserted{base_.after_inserted},
        before_erased{base_.before_erased},
        after_erased{base_.after_erased},
        before_changed{base_.before_changed},
        after_changed{base_.after_changed} {}

    /// Move constructor
    owning_projection(owning_projection && other):
        base_{std::move(other.base_)},
        before_inserted{base_.before_inserted},
        after_inserted{base_.after_inserted},
        before_erased{base_.before_erased},
        after_erased{base_.after_erased},
        before_changed{base_.before_changed},
        after_changed{base_.after_changed} {}

    /// Returns const iterator pointing to the first element
    auto begin() const { return std::ranges::begin(std::as_const(base_)); }

    /// Returns const iterator pointing to one past the last element
    auto end() const { return std::ranges::end(std::as_const(base_)); }

    /// Returns iterator pointing to the first element
    auto begin() { return std::ranges::begin(base_); }

    /// Returns iterator pointing to one past the last element
    auto end() { return std::ranges::end(base_); }

    /// Returns number of elements
    auto size() const { return std::ranges::size(std::as_const(base_)); }

private:
    Base base_;             ///< Base observable range

public:
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
