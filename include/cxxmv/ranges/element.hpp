// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file element.hpp
/// Contains definition of the element class.

#pragma once

#include "../observable.hpp"
#include "../proxy.hpp"
#include "../signals.hpp"
#include "all.hpp"
#include "projection.hpp"
#include <ranges>
#include <utility>


namespace mv::ranges {


/// Model of element in range. Becomes null when element is removed from range.
template <observable_projection Range>
requires nullable_observable<std::ranges::iterator_t<Range>>
class element: public mv::proxy<std::ranges::iterator_t<Range>> {
public:
    /// Type of iterator pointing to element
    using iterator_type = std::ranges::iterator_t<Range>;

    /// Type of const iterator pointing to element
    using const_iterator_type = std::ranges::iterator_t<const Range>;

    /// Constructs model of element in specified range pointed by specified iterator
    element(Range rng, const iterator_type & it = {}):
    mv::proxy<iterator_type>{it},
    range_{std::move(rng)} {
        before_erased_con_ = range_.before_erased().connect([this](auto && first, auto && last) {
            if (this->is_null()) {
                return;
            }

            const_iterator_type it = this->base();
            if (first <= it && it < last) {
                this->set({});
            }
        });
    }

    /// Model is not copyable
    element(const element &) = delete;

    /// Model is not copy-assignable
    element & operator=(const element &) = delete;

    /// Returns iterator pointing to element
    const iterator_type & iterator() const {
        return this->base();
    }

private:
    Range range_;                                   ///< Range containing element
    scoped_signal_connection before_erased_con_;    ///< Connection to range before_erased signal
};


template <projectable_observable Range>
element(Range &&) -> element<all_t<Range>>;

template <projectable_observable Range, typename Iterator>
element(Range &&, Iterator) -> element<all_t<Range>>;


}
