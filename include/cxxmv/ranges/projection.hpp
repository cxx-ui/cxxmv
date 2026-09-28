// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file view.hpp
/// Contains definition of observable range projections concepts.

#pragma once

#include "model.hpp"
#include "observable.hpp"


namespace mv::ranges {


/// Base class for projection. All projections should be derived from this to indicate
/// them confirm to projection concept.
struct projection_base {};


template <typename Projection>
inline constexpr bool enable_projection = std::derived_from<Projection, projection_base>;


/// Observable projection concept
template <typename Projection>
concept observable_projection = observable<Projection> && enable_projection<Projection>;


/// Observable projection for specified type concept
template <typename Projection, typename Value>
concept observable_projection_as = observable_as<Projection, Value> &&
                                   observable_projection<Projection>;


template <typename Observable>
concept projectable_observable = observable<Observable> && 
                                 (borrowed_observable<Observable> ||
                                  observable_projection<Observable>);

template <typename Observable, typename Value>
concept projectable_observable_as = projectable_observable<Observable> &&
                                    observable_as<Observable, Value>;



template <typename Projection, typename Value>
concept model_projection = observable_projection<Projection> && model<Projection, Value>;

template <typename Model, typename Value>
concept projectable_model = projectable_observable<Model> && model<Model, Value>;


}
