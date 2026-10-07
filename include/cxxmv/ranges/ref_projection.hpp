// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_projection.hpp
/// Contains definition of the ref_projection range class.

#pragma once

#include "element_handle.hpp"
#include "element_model.hpp"
#include "model.hpp"
#include "projection.hpp"
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <utility>


namespace mv::ranges {


template <typename Base>
requires observable<Base>
class ref_projection: public projection_base {
public:
    /// Type of iterator
    using iterator = std::ranges::iterator_t<Base>;

    /// Constructs view with reference to another range
    ref_projection(Base & b):
        base_{b} {}

    /// Copy constructor
    ref_projection(const ref_projection & other) = default;

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

    /// Inserts elements at specified position into base range
    template <typename It>
    void insert(const std::ranges::iterator_t<Base> & pos, It first, It last)
    requires model_with_insert<Base, std::ranges::range_value_t<Base>> {
        base_.insert(pos, first, last);
    }

    /// Erases elements from base range
    void erase(const std::ranges::iterator_t<Base> & first,
               const std::ranges::iterator_t<Base> & last)
    requires model_with_erase<Base, std::ranges::range_value_t<Base>> {
        base_.erase(first, last);
    }

    /// Moves elements in base range
    void move(const std::ranges::iterator_t<Base> & first,
              const std::ranges::iterator_t<Base> & last,
              const std::ranges::iterator_t<Base> & dest)
    requires model_with_move<Base, std::ranges::range_value_t<Base>> {
        base_.move(first, last, dest);
    }

    /// Starts mutating of element at specified index
    auto mut(size_t idx) {
        return base_.mut(idx);
    }

    /// Starts mutating of element pointed by specified iterator
    auto mut(const iterator & it) {
        return base_.mut(it);
    }

    /// Returns handle of element at specified index
    auto handle(size_t idx) requires has_element_handle<Base> {
        return base_.handle(idx);
    }

    /// Returns signal of base range emitted before items added
    decltype(auto) before_inserted() const { return base_.before_inserted(); }

    /// Returns signal of base range emitted after items added
    decltype(auto) after_inserted() const { return base_.after_inserted(); }

    /// Returns signal of base range emitted before items removed
    decltype(auto) before_erased() const { return base_.before_erased(); }

    /// Returns signal of base range emitted after items removed
    decltype(auto) after_erased() const { return base_.after_erased(); }

    /// Returns signal of base range emitted before item is changed
    decltype(auto) before_changed() const { return base_.before_changed(); }

    /// Returns signal of base range emitted after item is changed
    decltype(auto) after_changed() const { return base_.after_changed(); }

    /// Returns signal of base range emitted before items moved
    decltype(auto) before_moved() const requires observable_with_move<Base> {
        return base_.before_moved();
    }

    /// Returns signal of base range emitted after items moved
    decltype(auto) after_moved() const requires observable_with_move<Base> {
        return base_.after_moved();
    }

private:
    friend class element_model<ref_projection>;

    Base & base_;           ///< Reference to base observable range
};


/// Model of element in ref projection, defined if element model is defined for base range
template <typename Base>
requires requires { sizeof(element_model<Base>); }
class element_model<ref_projection<Base>>: public element_model<Base> {
public:
    using element_model<Base>::element_model;

    /// Constructs model of element in base range of projection referenced by specified handle
    element_model(ref_projection<Base> & proj, const element_handle<Base> & handle = {}):
        element_model<Base>{proj.base_, handle} {}
};


/// Element handle type for ref projection
template <typename Base>
requires requires { typename element_handle<Base>; }
struct element_handle_impl<ref_projection<Base>> {
    using type = element_handle<Base>;
};


}


template <typename Base>
inline constexpr bool std::ranges::enable_borrowed_range<mv::ranges::ref_projection<Base>> = true;
