// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file access.hpp
/// Contains definition of the get and mut functions for accessing values of observables.

#pragma once


namespace mv {


/// Mutator that references object directly. Does not emit any signals.
template <typename T>
class ref_mutator {
public:
    /// Constructs mutator with specified reference to object
    ref_mutator(T & obj):
        obj_{&obj} {}

    /// Returns true if mutator is empty
    bool empty() const { return obj_ == nullptr; }

    /// Returns reference to object
    T & ref() const { return *obj_; }

    /// Returns pointer to object
    T * ptr() const { return obj_; }

    /// Returns pointer to object
    T * operator->() const { return ptr(); }

private:
    T * obj_;       ///< Pointer to object
};


/// Returns value of observable. Returns observable itself if it does not have get method.
template <typename T>
decltype(auto) get(const T & obj) {
    if constexpr (requires { obj.get(); }) {
        return obj.get();
    } else {
        return static_cast<const T &>(obj);
    }
}


/// Returns mutator of model
template <typename T>
requires requires (T & obj) { obj.mut(); }
auto mut(T & obj) {
    return obj.mut();
}


/// Returns mutator referencing object itself for object without get and mut methods
template <typename T>
requires (!requires (T & obj) { obj.mut(); } && !requires (const T & obj) { obj.get(); })
ref_mutator<T> mut(T & obj) {
    return {obj};
}


}
