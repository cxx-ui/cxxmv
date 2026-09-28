// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file signal_ref.hpp
/// Contains definition of the signal_ref class.

#pragma once

#include "signals.hpp"


namespace mv {


/// Proxu reference to signal
template <typename Base, typename ... Args>
requires Signal<Base, Args...>
class signal_ref {
public:
    signal_ref(Base & b): base_{b} {}

    template <typename F>
    signal_connection connect(const F & f) const {
        return base_.connect(f);
    }

private:
    Base & base_;
};


}
