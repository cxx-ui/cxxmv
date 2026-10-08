// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file element_model.hpp
/// Contains definition of the element_model class.

#pragma once

#include "../model.hpp"
#include "../observable.hpp"
#include "../signals.hpp"
#include "all.hpp"
#include "projection.hpp"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <utility>


namespace mv::ranges {


/// Model of element in range. Becomes null when element is removed from range.
template <observable_projection Range>
requires nullable_observable<std::ranges::iterator_t<Range>>
class element_model {
public:
    /// Type of iterator pointing to element
    using iterator = std::ranges::iterator_t<Range>;

    /// Type of const iterator pointing to element
    using const_iterator = std::ranges::iterator_t<const Range>;

    /// Constructs model of element in specified range pointed by specified iterator
    template <projectable_observable R>
    requires std::same_as<all_t<R>, Range>
    element_model(R && rng, const iterator & it = {}):
    base_{all(std::forward<R>(rng))},
    it_{it} {
        connect_signals();
    }

    /// Model is not copyable
    element_model(const element_model &) = delete;

    /// Move constructor
    element_model(element_model && other):
    element_model{std::move(other), other.index()} {}

    /// Model is not copy-assignable
    element_model & operator=(const element_model &) = delete;

    /// Model is not move-assignable
    element_model & operator=(element_model &&) = delete;

    /// Returns true if element was removed from range
    bool is_null() const {
        return it_.is_null();
    }

    /// Returns index of element in range or SIZE_MAX if element is null
    size_t index() const {
        if (is_null()) {
            return SIZE_MAX;
        }

        const_iterator it = it_;
        return it - std::ranges::begin(base_);
    }

    /// Reads value of element
    decltype(auto) get() const {
        assert(!is_null() && "reading null range element");
        return it_.get();
    }

    /// Reads value of element
    decltype(auto) operator*() const {
        return get();
    }

    /// Starts mutating of element
    auto mut() requires mv::model<iterator> {
        assert(!is_null() && "mutating null range element");
        return it_.mut();
    }

    /// Sets iterator pointing to element in range. Emits before and after changed signals.
    void set(const iterator & it) {
        before_changed_();
        disconnect_signals();
        it_ = it;
        connect_signals();
        after_changed_();
    }

    /// Returns signal emitted before element is changed
    signal<void ()> & before_changed() const {
        return before_changed_;
    }

    /// Returns signal emitted after element is changed
    signal<void ()> & after_changed() const {
        return after_changed_;
    }

private:
    /// Moves range from other model and sets iterator to element at specified index
    element_model(element_model && other, size_t idx):
    base_{std::move(other.base_)} {
        assert(other.before_changed_.empty() && other.after_changed_.empty() &&
               "moving model with signal connections");

        other.disconnect_signals();
        if (idx != SIZE_MAX) {
            it_ = std::ranges::begin(base_) + idx;
        }

        connect_signals();
    }

    /// Connects to range and element signals if element is not null
    void connect_signals() {
        if (is_null()) {
            return;
        }

        before_erased_con_ = base_.before_erased().connect([this](auto && first, auto && last) {
            const_iterator it = it_;
            if (first <= it && it < last) {
                before_changed_();
                disconnect_signals();
                it_ = {};
                after_changed_();
            }
        });

        before_changed_con_ = it_.before_changed().connect([this] { before_changed_(); });
        after_changed_con_ = it_.after_changed().connect([this] { after_changed_(); });
    }

    /// Disconnects from range and element signals
    void disconnect_signals() {
        before_erased_con_.disconnect();
        before_changed_con_.disconnect();
        after_changed_con_.disconnect();
    }

    Range base_;                                    ///< Range containing element
    iterator it_;                                   ///< Iterator pointing to element
    mutable signal<void ()> before_changed_;        ///< Before changed signal
    mutable signal<void ()> after_changed_;         ///< After changed signal
    scoped_signal_connection before_erased_con_;    ///< Connection to range before_erased signal
    scoped_signal_connection before_changed_con_;   ///< Connection to element before_changed signal
    scoped_signal_connection after_changed_con_;    ///< Connection to element after_changed signal
};


template <projectable_observable Range>
element_model(Range &&) -> element_model<all_t<Range>>;

template <projectable_observable Range, typename Iterator>
element_model(Range &&, Iterator) -> element_model<all_t<Range>>;


}
