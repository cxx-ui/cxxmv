// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file owning_projection.hpp
/// Contains definition of the owning_projection class.

#pragma once

#include "../signal_ref.hpp"
#include "element_handle.hpp"
#include "element_model.hpp"
#include "move_signal_refs.hpp"
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
                         private owning_projection_base<Base>,
                         public move_signal_refs<Base> {
public:
    /// Type of iterator
    using iterator = typename Base::iterator;

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

    /// Starts mutating of element at specified index
    auto mut(size_t idx) {
        return this->base_.mut(idx);
    }

    /// Starts mutating of element pointed by iterator
    auto mut(const iterator & it) {
        return this->base_.mut(it);
    }

    /// Returns handle of element at specified index
    auto handle(size_t idx) requires has_element_handle<Base> {
        return this->base_.handle(idx);
    }

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
