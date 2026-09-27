// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file transform.hpp
/// Contains definition of the transform_projection adaptor and related functions.

#pragma once

#include "adaptor.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include "all.hpp"


namespace mv {


struct empty_set_fn {};


/// Transformed observable or model projection
template <projectable_observable Observable, typename GetFn, typename SetFn = empty_set_fn>
class transform_projection: public projection_base {
public:
    /// Constructs transform projection with specified get and set functions
    transform_projection(Observable b, GetFn gf, SetFn sf):
        base_{b}, get_fn_{std::move(gf)}, set_fn_{std::move(sf)},
        changed{b.changed} {}

    /// Copy constructor
    transform_projection(const transform_projection &) = default;

    /// Move constructor
    transform_projection(transform_projection &&) = default;

    /// Returns transformed value
    decltype(auto) get() const {
        return get_fn_(base_.get());
    }

    /// Returns transformed value
    decltype(auto) operator*() const {
        return get();
    }

    /// Assigns value to model
    template <typename Arg>
    void assign(Arg && val) requires (!std::same_as<std::decay_t<SetFn>, empty_set_fn>) {
        auto transformed_val = base_.get();
        set_fn_(transformed_val, std::forward<Arg>(val));
        base_.assign(transformed_val);
    }


    /// Returns assignable reference wrapper for stored value
    auto operator*() requires (!std::same_as<std::decay_t<SetFn>, empty_set_fn>) {
        return assign_wrapper{*this};
    }

    decltype(std::declval<Observable>().changed) changed;

private:
    Observable base_;
    GetFn get_fn_;
    SetFn set_fn_;
};


template <projectable_observable Observable, typename GetFn, typename SetFn>
transform_projection(Observable && obj, GetFn, SetFn) ->
    transform_projection<all_t<Observable>, GetFn, SetFn>;

template <projectable_observable Observable, typename GetFn>
transform_projection(Observable && obj, GetFn) ->
    transform_projection<all_t<Observable>, GetFn, empty_set_fn>;


template <typename SetFn, typename GetFn>
class transform_adaptor_closure {
public:
    transform_adaptor_closure(const GetFn & gf, const SetFn & sf):
    get_fn_{gf}, set_fn_{sf} {}

    template <observable Observable>
    auto operator()(Observable && obj) const {
        return transform_projection{obj, get_fn_, set_fn_};
    }

private:
    GetFn get_fn_;
    SetFn set_fn_;
};


template <observable Observable, typename SetFn, typename GetFn>
auto operator|(Observable && obj, const transform_adaptor_closure<SetFn, GetFn> & c) {
    return c(std::forward<Observable>(obj));
}


class transform_adaptor {
public:
    constexpr transform_adaptor() = default;

    template <observable Observable, typename GetFn, typename SetFn>
    auto operator()(Observable && obj, GetFn && gf, SetFn && sf) const {
        return transform_projection{obj, gf, sf};
    }

    template <typename GetFn, typename SetFn>
    auto operator()(GetFn && gf, SetFn && sf) const {
        return transform_adaptor_closure{std::forward<GetFn>(gf), std::forward<SetFn>(sf)};
    }

    template <observable Observable, typename GetFn>
    auto operator()(Observable && obj, GetFn && gf) const {
        return transform_projection{obj, gf};
    }

    template <typename GetFn>
    auto operator()(GetFn && gf) const {
        return transform_adaptor_closure{std::forward<GetFn>(gf), empty_set_fn{}};
    }
};


inline constexpr auto transform = transform_adaptor{};


}
