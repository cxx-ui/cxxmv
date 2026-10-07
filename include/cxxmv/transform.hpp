// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file transform.hpp
/// Contains definition of the transform_projection adaptor and related functions.

#pragma once

#include "adaptor.hpp"
#include "model.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include "all.hpp"
#include <concepts>
#include <type_traits>


namespace mv {


struct empty_set_fn {};


/// Mutator for transform projection
template <typename BaseMutator, typename GetFn, typename SetFn>
class transform_mutator {
    /// Type of value
    using value_type =
        std::decay_t<decltype(std::declval<GetFn>()(std::declval<BaseMutator>().ref()))>;

public:
    /// Constructs mutator with specified base mutator and references to get/set functions
    transform_mutator(BaseMutator mut, GetFn & get_fn, SetFn & set_fn):
        mut_{std::move(mut)},
        val_{get_fn(mut_.ref())},
        set_fn_{set_fn} {}

    /// Mutator is not copyable
    transform_mutator(const transform_mutator &) = delete;

    /// Move constructor
    transform_mutator(transform_mutator && other) = default;

    /// Destroys reference, assigns value in base model and emits changed signsl
    ~transform_mutator() {
        if (!empty()) {
            set_fn_(mut_.ref(), std::move(val_));
        }
    }

    /// Returns true if mutator is empty
    bool empty() const {
        return mut_.empty();
    }

    /// Returns reference to value
    value_type & ref() const {
        return val_;
    }

    /// Returns pointer to value
    value_type * ptr() const {
        return &ref();
    }

    /// Returns pointer to value
    value_type * operator->() const {
        return ptr();
    }

    /// Assigns model value
    const transform_mutator & operator=(const value_type & val) const {
        ref() = val;
        return *this;
    }

    /// Assigns model value with move
    const transform_mutator & operator=(value_type && val) const {
        ref() = std::move(val);
        return *this;
    }

private:
    BaseMutator mut_;           ///< Base model mutator
    mutable value_type val_;    ///< Temporary value object
    SetFn & set_fn_;            ///< Reference to set function
};


/// Transformed observable or model projection
template <projectable_observable Observable, typename GetFn, typename SetFn = empty_set_fn>
class transform_projection: public projection_base {

public:
    /// Constructs transform projection with specified get and set functions
    transform_projection(Observable b, GetFn gf, SetFn sf):
        base_{std::move(b)}, get_fn_{std::move(gf)}, set_fn_{std::move(sf)} {}

    /// Copy constructor
    transform_projection(const transform_projection & other) = default;

    /// Move constructor
    transform_projection(transform_projection && other) = default;

    /// Returns transformed value
    decltype(auto) get() const {
        return get_fn_(base_.get());
    }

    /// Returns transformed value
    decltype(auto) operator*() const {
        return get();
    }

    /// Returns true if value of base observable is null
    bool is_null() const requires nullable_observable<Observable> {
        return base_.is_null();
    }

    /// Returns model value mutator
    auto mut() {
        return transform_mutator{base_.mut(), get_fn_, set_fn_};
    }

    /// Returns changed signal of base observable
    decltype(auto) changed() const {
        return base_.changed();
    }

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
        return transform_projection{std::forward<Observable>(obj), get_fn_, set_fn_};
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
