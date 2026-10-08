// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file element.hpp
/// Contains declaration of the element_projection class and element adaptor.

#pragma once

#include "../projection.hpp"
#include "../signals.hpp"
#include "all.hpp"
#include "model.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>


namespace mv::ranges {


/// Projection of single element of range
template <typename Range>
requires observable<Range>
class element_projection: public mv::projection_base {
public:
    /// Type of iterator pointing to element
    using iterator = std::ranges::iterator_t<Range>;

    /// Type of const iterator over range elements
    using const_iterator = std::ranges::iterator_t<const Range>;

    /// Signal emitted before or after element is changed
    class changed_signal {
    public:
        /// Constructs signal for specified element projection
        changed_signal(const element_projection * proj, bool before):
            proj_{proj}, before_{before} {}

        /// Connects function to signal
        signal_connection connect(const std::function<void ()> & fn) const {
            if (before_) {
                return connect_to(proj_->base_.before_changed(), fn);
            } else {
                return connect_to(proj_->base_.after_changed(), fn);
            }
        }

    private:
        /// Connects function to specified changed signal of range
        template <typename RangeSignal>
        signal_connection connect_to(RangeSignal && sig, const std::function<void ()> & fn) const {
            return sig.connect([elem = const_iterator{proj_->it_}, fn](const auto & it) {
                if (elem == it) {
                    fn();
                }
            });
        }

        const element_projection * proj_;       ///< Pointer to element projection
        bool before_;                           ///< Is it before changed signal
    };

    /// Constructs projection of element in specified range pointed by specified iterator
    element_projection(Range base, const iterator & it = {}):
        base_{std::move(base)}, it_{it} {}

    /// Copy constructor
    element_projection(const element_projection & other) = default;

    /// Move constructor
    element_projection(element_projection && other) = default;

    /// Copy assignment operator
    element_projection & operator=(const element_projection & other) = default;

    /// Move assignment operator
    element_projection & operator=(element_projection && other) = default;

    /// Returns true if projection does not point to element
    bool is_null() const {
        return it_ == iterator{};
    }

    /// Reads value of element
    decltype(auto) get() const {
        assert(!is_null() && "reading null range element");
        return base_.get(it_);
    }

    /// Reads value of element
    decltype(auto) operator*() const {
        return get();
    }

    /// Starts mutating of element
    auto mut() requires model<Range, std::ranges::range_value_t<Range>> {
        assert(!is_null() && "mutating null range element");
        return base_.mut(it_);
    }

    /// Returns signal emitted before element is changed
    changed_signal before_changed() const {
        return {this, true};
    }

    /// Returns signal emitted after element is changed
    changed_signal after_changed() const {
        return {this, false};
    }

private:
    Range base_;            ///< Projection of range
    iterator it_;           ///< Iterator pointing to element in range
};


template <projectable_observable Range, typename It>
element_projection(Range && r, const It &) -> element_projection<all_t<Range>>;


/// Closure of element adaptor that stores iterator pointing to element
template <typename It>
class element_adaptor_closure {
public:
    /// Constructs closure with specified iterator
    element_adaptor_closure(const It & it):
    it_{it} {}

    /// Returns projection of element of specified range
    template <projectable_observable Range>
    auto operator()(Range && r) const {
        return element_projection{std::forward<Range>(r), it_};
    }

private:
    It it_;                 ///< Iterator pointing to element
};


template <projectable_observable Range, typename It>
auto operator|(Range && r, const element_adaptor_closure<It> & c) {
    return c(std::forward<Range>(r));
}


/// Adaptor for creating projection of single range element
class element_adaptor {
public:
    constexpr element_adaptor() = default;

    /// Returns projection of element of specified range pointed by iterator
    template <projectable_observable Range, typename It>
    auto operator()(Range && r, const It & it) const {
        return element_projection{std::forward<Range>(r), it};
    }

    /// Returns closure for creating projection of element pointed by iterator
    template <typename It>
    auto operator()(const It & it) const {
        return element_adaptor_closure<It>{it};
    }
};


inline constexpr auto element = element_adaptor{};


}
