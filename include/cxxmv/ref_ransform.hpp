// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_transform.hpp
/// Contains definition of the ref_transform_projection adaptor and related functions.

#pragma once

#include "adaptor.hpp"
#include "model.hpp"
#include "observable.hpp"
#include "projection.hpp"
#include "all.hpp"
#include <type_traits>


namespace mv {


struct ref_empty_set_fn {};


/// Mutator for ref_transform projection
template <typename BaseMutator, typename GetRefFn>
class ref_transform_mutator {
public:
    using value_t =
        std::decay_t<decltype(std::declval<GetRefFn>()(std::declval<BaseMutator>().ref()))>;

    /// Constructs mutator with specified base mutator and references to get/set functions
    ref_transform_mutator(BaseMutator ref, GetRefFn & get_ref_fn):
        mut_{std::move(ref)}, get_ref_fn_{get_ref_fn} {}

    /// Mutator is not copyable
    ref_transform_mutator(const ref_transform_mutator &) = delete;

    /// Move constructor
    ref_transform_mutator(ref_transform_mutator && other) = default;

    /// Destroys reference, emits changed signal
    ~ref_transform_mutator() = default;

    /// Returns true if mutator is empty
    bool empty() const {
        return mut_.empty();
    }

    /// Returns reference to value
    decltype(auto) ref() const {
        return get_ref_fn_(mut_.ref());
    }

    /// Returns pointer to value
    decltype(auto) ptr() const {
        return &ref();
    }

    /// Returns pointer to value
    decltype(auto) operator->() const {
        return ptr();
    }

    /// Assigns model value
    const ref_transform_mutator & operator=(const value_t & val) const {
        ref() = val;
        return *this;
    }

    /// Assigns model value with move
    const ref_transform_mutator & operator=(value_t && val) const {
        ref() = std::move(val);
        return *this;
    }

private:
    BaseMutator mut_;               ///< Base model reference
    GetRefFn & get_ref_fn_;         ///< Reference to get reference function
};


/// Projection for observable or model that transforms reference of the base model
template <projectable_observable Observable, typename GetRefFn>
class ref_transform_projection: public projection_base {

public:
    /// Constructs transform projection with specified get and set functions
    ref_transform_projection(Observable b, GetRefFn gf):
        base_{std::move(b)}, get_ref_fn_{std::move(gf)} {}

    /// Copy constructor
    ref_transform_projection(const ref_transform_projection &) = default;

    /// Move constructor
    ref_transform_projection(ref_transform_projection && other) = default;

    /// Copy assignment operator
    ref_transform_projection & operator=(const ref_transform_projection & other) = default;

    /// Move assignment operator
    ref_transform_projection & operator=(ref_transform_projection && other) = default;

    /// Returns transformed value
    decltype(auto) get() const {
        return get_ref_fn_(base_.get());
    }

    /// Returns transformed value
    decltype(auto) operator*() const {
        return get();
    }

    /// Returns true if value of base observable is null
    bool is_null() const requires nullable_observable<Observable> {
        return base_.is_null();
    }

    /// Returns mutator for model value
    auto mut() {
        return ref_transform_mutator{base_.mut(), get_ref_fn_};
    }

    /// Returns before changed signal of base observable
    decltype(auto) before_changed() const {
        return base_.before_changed();
    }

    /// Returns after changed signal of base observable
    decltype(auto) after_changed() const {
        return base_.after_changed();
    }

private:
    Observable base_;
    GetRefFn get_ref_fn_;
};


template <projectable_observable Observable, typename GetRefFn>
ref_transform_projection(Observable && obj, GetRefFn) ->
    ref_transform_projection<all_t<Observable>, GetRefFn>;


template <typename GetFn>
class ref_transform_adaptor_closure {
public:
    ref_transform_adaptor_closure(const GetFn & gf):
        get_fn_{gf}{}

    template <observable Observable>
    auto operator()(Observable && obj) const {
        return ref_transform_projection{std::forward<Observable>(obj), get_fn_};
    }

private:
    GetFn get_fn_;
};


template <observable Observable, typename GetFn>
auto operator|(Observable && obj, const ref_transform_adaptor_closure<GetFn> & c) {
    return c(std::forward<Observable>(obj));
}


class ref_transform_adaptor {
public:
    constexpr ref_transform_adaptor() = default;

    template <observable Observable, typename GetFn>
    auto operator()(Observable && obj, GetFn && gf) const {
        return ref_transform_projection{obj, gf};
    }

    template <typename GetFn>
    auto operator()(GetFn && gf) const {
        return ref_transform_adaptor_closure{std::forward<GetFn>(gf)};
    }
};


inline constexpr auto ref_transform = ref_transform_adaptor{};


}
