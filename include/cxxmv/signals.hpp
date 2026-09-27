// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// signals.hpp
/// Contains definition of signal classes.

#pragma once

#include <boost/signals2/signal.hpp>


namespace mv {


/// Signal type alias
template <typename Signature>
using signal = boost::signals2::signal<Signature>;

/// Signal connection type alias
using signal_connection = boost::signals2::connection;

/// Scoped signal connection type alias
using scoped_signal_connection = boost::signals2::scoped_connection;


/// Signal concept
template <typename T, typename ... Args>
concept Signal = requires(T s, std::function<void (Args...)> f) {
    { s.connect(f) } -> std::same_as<signal_connection>;
};


}
