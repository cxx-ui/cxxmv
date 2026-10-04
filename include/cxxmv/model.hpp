// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file model.hpp
/// Contains definition of the model concept.

#pragma once

#include "observable.hpp"
#include <concepts>


namespace mv {


/// Mutator concept for model
template <typename Mutator, typename Value>
concept mutator = requires(const Mutator & mut) {
    /// Returns true if mutator is empty
    { mut.empty() } -> std::convertible_to<bool>;

    /// Getting reference to value
    { mut.ref() } -> std::convertible_to<Value &>;

    /// Assigning to value by reference
    { mut.ref() = std::declval<Value>() };

    /// Getting pointer to value
    { mut.ptr() } -> std::convertible_to<Value *>;

    /// Getting pointer to value for field access
    { mut.operator->() } -> std::convertible_to<Value *>;

    /// Assigns value of model
    { mut = std::declval<Value>() };
};


/// Model concept represents observable with mutator access
template <typename Model>
concept model = observable<Model> && requires (Model & mdl) {
    /// Mutator access
    { mdl.mut() };
};


/// Concept of model representing speicifed type
template <typename Model, typename Value>
concept model_of = model<Model> && observable_as<Model, Value> && requires(Model mdl) {
    /// Model mutator access
    { mdl.mut() } -> mutator<Value>;
};


}
