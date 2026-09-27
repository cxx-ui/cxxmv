// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file model.hpp
/// Contains definition of the model concept.

#pragma once

#include "observable.hpp"


namespace mv {


/// The Model concept describes observable value that can be modified
template <typename Model, typename Value>
concept model = observable_as<Model, Value> && requires(Model mdl) {

    /// Assigns value to model
    { mdl.assign(std::declval<Value>()) };

    /// Assigns value to model
    //{ *mdl = std::declval<Value>() };
};


}
