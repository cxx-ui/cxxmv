// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection class.

#pragma once

#include "element_handle.hpp"
#include "element_model.hpp"
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

    /// Returns handle of element at specified index
    auto handle_at(size_t idx) requires observable_with_handle<Base> {
        return this->base_.handle_at(idx);
    }

    /// Reads element at specified index
    decltype(auto) get(size_t idx) const {
        return *(begin() + idx);
    }

    /// Reads element referenced by specified handle
    template <typename Handle>
    requires observable_with_handle<Base> && std::same_as<Handle, element_handle<Base>>
    decltype(auto) get(const Handle & h) const {
        return this->base_.get(h);
    }

    /// Starts mutating of element referenced by specified handle
    template <typename Handle>
    requires model_with_handle<Base> && std::same_as<Handle, element_handle<Base>>
    auto mut(const Handle & h) {
        return this->base_.mut(h);
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

private:
    friend class element_model<owning_projection>;
};


/// Model of element in owning projection, defined if element model is defined for base range
template <typename Base>
requires requires { sizeof(element_model<Base>); }
class element_model<owning_projection<Base>>: public element_model<Base> {
public:
    using element_model<Base>::element_model;

    /// Constructs model of element in base range of projection referenced by specified handle
    element_model(owning_projection<Base> & proj, const element_handle<Base> & handle = {}):
        element_model<Base>{proj.base_, handle} {}
};


/// Element handle type for owning projection
template <typename Base>
requires requires { typename element_handle<Base>; }
struct element_handle_impl<owning_projection<Base>> {
    using type = element_handle<Base>;
};


}
