// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file assign_wrapper.hpp
/// Contains definition of the assign_wrapper class.

#pragma once

#include "model.hpp"


namespace mv {


/// Helper class for definining assignments to result of dereference operator
template <typename Model>
class assign_wrapper {
public:
    /// Constructs wrapper with specified reference to model instance
    assign_wrapper(Model & mdl):
        model_{mdl} {}

    /// Assigns to value inside model. Emits changed signal after assignment
    template <typename Arg>
    assign_wrapper & operator=(Arg && arg) {
        model_.assign(std::forward<Arg>(arg));
        return *this;
    }

    /// Auto conversion to const reference to value
    operator decltype(std::declval<const Model>().get()) () const {
        return model_.get();
    }

private:
    Model & model_;             ///< Reference to model instance
};


}
