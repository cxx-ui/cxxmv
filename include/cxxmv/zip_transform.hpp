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
#include <tuple>


namespace mv {


/// Zip transformed projection of observable or model
template <typename GetFn, projectable_observable ... Observables>
class zip_transform_projection: public projection_base {
    /// Type of tuple of base observables
    using observables_tuple = std::tuple<Observables...>;

    /// Type of array of connections to base observables
    using observables_connections = std::array<scoped_signal_connection, sizeof...(Observables)>;

public:
    /// Constructs transform projection from zip function and observables
    zip_transform_projection(GetFn gf, Observables ... bases):
    bases_{std::move(bases)...},
    get_fn_{std::move(gf)} {
        // connecting to changes signals of all base observables
        std::apply([this](Observables & ... bases) {
            std::size_t idx = 0;
            ((con_[idx++] = bases.changed.connect([this] { changed(); })), ...);
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

    /// The changed signal is emitted after one of zipped observables is changed
    signal<void ()> changed;

private:
    observables_tuple bases_;           ///< Tuple of base observables
    GetFn get_fn_;                      ///< Get function
    observables_connections con_;       ///< Array of connections to base observables
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
