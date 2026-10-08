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
#include "element_handle.hpp"
#include "model.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>


namespace mv::ranges {


/// Projection of single element of range
template <typename Range>
requires observable<Range> && observable_with_handle<Range>
class element_projection: public mv::projection_base {
public:
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
            return sig.connect([handle = proj_->handle_, fn](const auto & it) {
                if (handle == it) {
                    fn();
                }
            });
        }

        const element_projection * proj_;       ///< Pointer to element projection
        bool before_;                           ///< Is it before changed signal
    };

    /// Constructs projection of element in specified range referenced by specified handle
    element_projection(Range base, const element_handle<Range> & handle = {}):
        base_{std::move(base)}, handle_{handle} {}

    /// Copy constructor
    element_projection(const element_projection & other) = default;

    /// Move constructor
    element_projection(element_projection && other) = default;

    /// Copy assignment operator
    element_projection & operator=(const element_projection & other) = default;

    /// Move assignment operator
    element_projection & operator=(element_projection && other) = default;

    /// Returns true if element was removed from range
    bool is_null() const {
        return !static_cast<bool>(handle_);
    }

    /// Reads value of element
    decltype(auto) get() const {
        assert(!is_null() && "reading null range element");
        return base_.get(handle_);
    }

    /// Reads value of element
    decltype(auto) operator*() const {
        return get();
    }

    /// Starts mutating of element
    auto mut() requires model_with_handle<Range> {
        assert(!is_null() && "mutating null range element");
        return base_.mut(handle_);
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
    Range base_;                        ///< Projection of range
    element_handle<Range> handle_;      ///< Handle of element in range
};


template <projectable_observable Range, typename Handle>
element_projection(Range && r, const Handle &) -> element_projection<all_t<Range>>;


/// Closure of element adaptor that stores element handle
template <typename Handle>
class element_adaptor_closure {
public:
    /// Constructs closure with specified element handle
    element_adaptor_closure(const Handle & handle):
    handle_{handle} {}

    /// Returns projection of element of specified range
    template <projectable_observable Range>
    auto operator()(Range && r) const {
        return element_projection{std::forward<Range>(r), handle_};
    }

private:
    Handle handle_;         ///< Element handle
};


template <projectable_observable Range, typename Handle>
auto operator|(Range && r, const element_adaptor_closure<Handle> & c) {
    return c(std::forward<Range>(r));
}


/// Adaptor for creating projection of single range element
class element_adaptor {
public:
    constexpr element_adaptor() = default;

    /// Returns projection of element of specified range referenced by handle
    template <projectable_observable Range, typename Handle>
    auto operator()(Range && r, const Handle & handle) const {
        return element_projection{std::forward<Range>(r), handle};
    }

    /// Returns closure for creating projection of element referenced by handle
    template <typename Handle>
    auto operator()(const Handle & handle) const {
        return element_adaptor_closure<Handle>{handle};
    }
};


inline constexpr auto element = element_adaptor{};


}
