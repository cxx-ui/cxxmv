// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file proxy.hpp
/// Contains definition of the proxy class.

#pragma once

#include "all.hpp"
#include "model.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include "signals.hpp"
#include <cassert>
#include <concepts>
#include <utility>


namespace mv {


/// Observable or model that forwards access to projection which can be replaced
template <observable_projection Projection>
class proxy {
public:
    /// Constructs proxy for specified projection
    proxy(Projection proj = {}):
    proj_{std::move(proj)} {
        connect_signals();
    }

    /// Proxy is not copyable
    proxy(const proxy &) = delete;

    /// Move constructor
    proxy(proxy && other):
    proj_{std::move(other.proj_)} {
        assert(other.before_changed_.empty() && other.after_changed_.empty() &&
               "moving proxy with signal connections");

        other.disconnect_signals();
        connect_signals();
    }

    /// Proxy is not copy-assignable
    proxy & operator=(const proxy &) = delete;

    /// Proxy is not move-assignable
    proxy & operator=(proxy &&) = delete;

    /// Returns projection
    const Projection & base() const {
        return proj_;
    }

    /// Returns true if value of projection is null
    bool is_null() const requires nullable_observable<Projection> {
        return proj_.is_null();
    }

    /// Reads value of projection
    decltype(auto) get() const {
        assert(!mv::is_null(proj_) && "reading null proxy");
        return mv::get(proj_);
    }

    /// Reads value of projection
    decltype(auto) operator*() const {
        return get();
    }

    /// Starts mutating of projection value
    auto mut() requires model<Projection> {
        assert(!mv::is_null(proj_) && "mutating null proxy");
        return mv::mut(proj_);
    }

    /// Sets projection. Emits before and after changed signals.
    void set(Projection proj) {
        if constexpr (std::equality_comparable<Projection>) {
            if (proj == proj_) {
                return;
            }
        }

        before_changed_();
        disconnect_signals();
        proj_ = std::move(proj);
        connect_signals();
        after_changed_();
    }

    /// Returns signal emitted before value is changed
    signal<void ()> & before_changed() const {
        return before_changed_;
    }

    /// Returns signal emitted after value is changed
    signal<void ()> & after_changed() const {
        return after_changed_;
    }

private:
    /// Connects to projection signals if projection value is not null
    void connect_signals() {
        if (mv::is_null(proj_)) {
            return;
        }

        before_changed_con_ = proj_.before_changed().connect([this] { before_changed_(); });
        after_changed_con_ = proj_.after_changed().connect([this] { after_changed_(); });
    }

    /// Disconnects from projection signals
    void disconnect_signals() {
        before_changed_con_.disconnect();
        after_changed_con_.disconnect();
    }

    Projection proj_;                               ///< Projection
    mutable signal<void ()> before_changed_;        ///< Before changed signal
    mutable signal<void ()> after_changed_;         ///< After changed signal
    scoped_signal_connection before_changed_con_;   ///< Connection to projection before_changed
    scoped_signal_connection after_changed_con_;    ///< Connection to projection after_changed
};


template <projectable_observable Observable>
proxy(Observable &&) -> proxy<all_t<Observable>>;


}
