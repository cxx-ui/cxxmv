// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file move_signal_refs.hpp
/// Contains definition of the move_signal_refs class.

#pragma once

#include "../signal_ref.hpp"
#include "observable.hpp"


namespace mv::ranges {


/// Base class with references to move signals of base range.
/// Empty for ranges without move support.
template <typename Base>
class move_signal_refs {
public:
    /// Constructs empty instance
    move_signal_refs(Base &) {}
};


/// Base class for projections with references to move signals of base observable range
template <observable_with_move Base>
class move_signal_refs<Base> {
public:
    /// Constructs instance with references to move signals of specified base range
    move_signal_refs(Base & b):
        before_moved{b.before_moved},
        after_moved{b.after_moved} {}

    /// The signal is emitted before items moved
    signal_ref<decltype(Base::before_moved)> before_moved;

    /// The signal is emitted after items moved
    signal_ref<decltype(Base::after_moved)> after_moved;
};


}
