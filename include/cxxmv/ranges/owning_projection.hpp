// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection class.

#pragma once

#include "projection.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
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
requires observable<Base> && std::move_constructible<Base>
class owning_projection: public projection_base,
                         private owning_projection_base<Base> {
public:
    /// Type of iterator
    using iterator = typename Base::iterator;

    /// Constructs projection owning specified observable range
    owning_projection(Base && b):
        owning_projection_base<Base>{std::move(b)} {}

    /// Copy constructor
    owning_projection(const owning_projection & other) = default;

    /// Move constructor
    owning_projection(owning_projection && other) = default;

    /// Copy assignment operator
    owning_projection & operator=(const owning_projection & other) = default;

    /// Move assignment operator
    owning_projection & operator=(owning_projection && other) = default;

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

    /// Starts mutating of element at specified index
    auto mut(size_t idx) {
        return this->base_.mut(idx);
    }

    /// Starts mutating of element pointed by iterator
    auto mut(const iterator & it) {
        return this->base_.mut(it);
    }

    /// Reads element at specified index
    decltype(auto) get(size_t idx) const {
        return *(begin() + idx);
    }

    /// Reads element pointed by specified iterator
    decltype(auto) get(const iterator & it) const {
        return this->base_.get(it);
    }

    /// Returns signal of base range emitted before items added
    decltype(auto) before_inserted() const { return this->base_.before_inserted(); }

    /// Returns signal of base range emitted after items added
    decltype(auto) after_inserted() const { return this->base_.after_inserted(); }

    /// Returns signal of base range emitted before items removed
    decltype(auto) before_erased() const { return this->base_.before_erased(); }

    /// Returns signal of base range emitted after items removed
    decltype(auto) after_erased() const { return this->base_.after_erased(); }

    /// Returns signal of base range emitted before item is changed
    decltype(auto) before_changed() const { return this->base_.before_changed(); }

    /// Returns signal of base range emitted after item is changed
    decltype(auto) after_changed() const { return this->base_.after_changed(); }

    /// Returns signal of base range emitted before items moved
    decltype(auto) before_moved() const requires observable_with_move<Base> {
        return this->base_.before_moved();
    }

    /// Returns signal of base range emitted after items moved
    decltype(auto) after_moved() const requires observable_with_move<Base> {
        return this->base_.after_moved();
    }
};


}
