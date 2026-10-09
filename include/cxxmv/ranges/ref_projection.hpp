// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_projection.hpp
/// Contains definition of the ref_projection range class.

#pragma once

#include "model.hpp"
#include "projection.hpp"
#include <concepts>
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
        base_{&b} {}

    /// Copy constructor
    ref_projection(const ref_projection & other) = default;

    /// Copy assignment operator
    ref_projection & operator=(const ref_projection & other) = default;

    /// Returns const iterator pointing to the first element
    auto begin() const { return std::ranges::begin(std::as_const(*base_)); }

    /// Returns const iterator pointing to one past the last element
    auto end() const { return std::ranges::end(std::as_const(*base_)); }

    /// Returns iterator pointing to the first element
    auto begin() { return std::ranges::begin(*base_); }

    /// Returns iterator pointing to one past the last element
    auto end() { return std::ranges::end(*base_); }

    /// Returns number of elements
    auto size() const { return std::ranges::size(std::as_const(*base_)); }

    /// Inserts elements at specified position into base range
    template <typename It>
    void insert(const std::ranges::iterator_t<Base> & pos, It first, It last)
    requires model_with_insert<Base, std::ranges::range_value_t<Base>> {
        base_->insert(pos, first, last);
    }

    /// Erases elements from base range
    void erase(const std::ranges::iterator_t<Base> & first,
               const std::ranges::iterator_t<Base> & last)
    requires model_with_erase<Base, std::ranges::range_value_t<Base>> {
        base_->erase(first, last);
    }

    /// Moves elements in base range
    void move(const std::ranges::iterator_t<Base> & first,
              const std::ranges::iterator_t<Base> & last,
              const std::ranges::iterator_t<Base> & dest)
    requires model_with_move<Base, std::ranges::range_value_t<Base>> {
        base_->move(first, last, dest);
    }

    /// Starts mutating of element at specified index
    auto mut(size_t idx) requires model<Base, std::ranges::range_value_t<Base>> {
        return base_->mut(idx);
    }

    /// Starts mutating of element pointed by specified iterator
    auto mut(const iterator & it) requires model<Base, std::ranges::range_value_t<Base>> {
        return base_->mut(it);
    }

    /// Reads element at specified index
    decltype(auto) get(size_t idx) const {
        return *(begin() + idx);
    }

    /// Reads element pointed by specified iterator
    decltype(auto) get(const iterator & it) const {
        return std::as_const(*base_).get(it);
    }

    /// Returns signal of base range emitted before items added
    decltype(auto) before_inserted() const { return base_->before_inserted(); }

    /// Returns signal of base range emitted after items added
    decltype(auto) after_inserted() const { return base_->after_inserted(); }

    /// Returns signal of base range emitted before items removed
    decltype(auto) before_erased() const { return base_->before_erased(); }

    /// Returns signal of base range emitted after items removed
    decltype(auto) after_erased() const { return base_->after_erased(); }

    /// Returns signal of base range emitted before item is changed
    decltype(auto) before_changed() const { return base_->before_changed(); }

    /// Returns signal of base range emitted after item is changed
    decltype(auto) after_changed() const { return base_->after_changed(); }

    /// Returns signal of base range emitted before items moved
    decltype(auto) before_moved() const requires observable_with_move<Base> {
        return base_->before_moved();
    }

    /// Returns signal of base range emitted after items moved
    decltype(auto) after_moved() const requires observable_with_move<Base> {
        return base_->after_moved();
    }

private:
    Base * base_;           ///< Pointer to base observable range
};


}


template <typename Base>
inline constexpr bool std::ranges::enable_borrowed_range<mv::ranges::ref_projection<Base>> = true;
