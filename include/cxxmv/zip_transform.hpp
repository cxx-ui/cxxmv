// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file zip_transform.hpp
/// Contains definition of the zip_transform_projection adaptor and related classes.

#pragma once

#include "all.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include "signals.hpp"
#include <array>
#include <memory>
#include <tuple>


namespace mv {


/// Zip transformed projection of observable or model
template <typename GetFn, projectable_observable ... Observables>
class zip_transform_projection: public projection_base {
    /// Type of tuple of base observables
    using observables_tuple = std::tuple<Observables...>;

    /// Type of array of connections to base observables
    using observables_connections = std::array<scoped_signal_connection, sizeof...(Observables)>;

    /// Shared state for all zip transform projection copies, contains changed signal
    /// and connection to all base observables
    struct shared_state_t {
        signal<void ()> changed;        ///< Changed signal
        observables_connections con;    ///< Array of connections to base observables
    };

public:
    /// Constructs transform projection from zip function and observables
    zip_transform_projection(GetFn gf, Observables ... bases):
    bases_{std::move(bases)...},
    get_fn_{std::move(gf)},
    state_{std::make_shared<shared_state_t>()} {
        // connecting to changes signals of all base observables. Slots capture raw pointer
        // to state because connections are owned by state and can't outlive it.
        std::apply([state = state_.get()](Observables & ... bases) {
            std::size_t idx = 0;
            ((state->con[idx++] = bases.changed().connect([state] { state->changed(); })), ...);
        }, bases_);
    }

    /// Returns transformed value
    decltype(auto) get() const {
        return std::apply([this](const Observables & ... bases) -> decltype(auto) {
            return get_fn_(bases.get()...);
        }, bases_);
    }

    /// Returns transformed value
    decltype(auto) operator*() const {
        return get();
    }

    /// Returns true if value of any nullable base observable is null
    bool is_null() const requires (nullable_observable<Observables> || ...) {
        return std::apply([](const Observables & ... bases) {
            return (mv::is_null(bases) || ...);
        }, bases_);
    }

    /// Returns signal emitted after any of base observables is changed
    signal<void ()> & changed() const {
        return state_->changed;
    }

private:
    observables_tuple bases_;                   ///< Tuple of base observables
    GetFn get_fn_;                              ///< Get function
    std::shared_ptr<shared_state_t> state_;     ///< State shared by all copies
};


template <typename GetFn, projectable_observable ... Observables>
zip_transform_projection(GetFn, Observables && ... observables) ->
    zip_transform_projection<GetFn, all_t<Observables>...>;


class zip_transform_adaptor {
public:
    constexpr zip_transform_adaptor() = default;

    template <typename GetFn, observable ... Observables>
    auto operator()(GetFn && gf, Observables && ... observables) const {
        return zip_transform_projection{gf, std::forward<Observables>(observables)...};
    }
};


inline constexpr auto zip_transform = zip_transform_adaptor{};


}
