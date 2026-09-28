// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file all.hpp
/// Contains definition of the ref_projection range projection and all adaptor.

#pragma once

#include "../signal_ref.hpp"
#include "adaptor.hpp"
#include "projection.hpp"
#include <ranges>
#include <type_traits>
#include <utility>


namespace mv::ranges {


template <typename Base>
requires observable<Base>
class ref_projection: public projection_base {
public:
    /// Constructs view with reference to another range
    ref_projection(Base & b):
        base_{b},
        before_inserted{b.before_inserted},
        after_inserted{b.after_inserted},
        before_erased{b.before_erased},
        after_erased{b.after_erased},
        before_changed{b.before_changed},
        after_changed{b.after_changed} {}

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

private:
    Base & base_;           ///< Reference to base observable range

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


inline constexpr auto all = adaptor_closure {
    []<projectable_observable Range>(Range && r) {
        if constexpr (observable_projection<std::decay_t<Range>>) {
            return std::forward<Range>(r);
        } else {
            return ref_projection{std::forward<Range>(r)};
        }
    }
};


template <projectable_observable Range>
using all_t = decltype(all(std::declval<Range>()));


}
