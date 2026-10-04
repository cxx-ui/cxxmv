// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file basic_model.hpp
/// Contains definition of the basic_model class.

#pragma once

#include "model.hpp"
#include <initializer_list>
#include <type_traits>


namespace mv {


/// Basic implementation of model for specified value type.
template <typename Value>
class basic_model {
public:
    /// Model mutator
    class mutator {
    public:
        /// Constructs mutator with specified pointer to projection
        mutator(basic_model * mdl):
            mdl_{mdl} {}

        /// Mutator is not copyable
        mutator(const mutator &) = delete;

        /// Move constructor
        mutator(mutator && other):
        mdl_{other.mdl_} {
            other.mdl_ = nullptr;
        }

        /// Destroys mutator, emits changed signal
        ~mutator() {
            if (!empty()) {
                mdl_->changed();
            }
        }

        /// Returns true if mutator is empty
        bool empty() const {
            return mdl_ == nullptr;
        }

        /// Assigns value to model
        const mutator & operator=(const Value & val) const {
            assert(mdl_ && "assigning to empty reference");
            mdl_->value_ = val;
            return *this;
        }

        /// Assigns value to model with move
        const mutator & operator=(Value && val) const {
            assert(mdl_ && "assigning to empty reference");
            mdl_->value_ = std::move(val);
            return *this;
        }

        /// Assigns value of another mutator
        const mutator & operator=(const mutator & other) const {
            return *this = static_cast<const Value &>(other);
        }

        /// Returns reference to object value
        Value & ref() const { return mdl_->value_; }

        /// Returns pointer to object value
        Value * ptr() const { return &ref(); }

        /// Returns pointer to object value
        Value * operator->() const { return ptr(); }

    private:
        basic_model * mdl_;         ///< Pointer to model
    };


    /// The changed signal is emitted after value is changed in the model
    mutable signal<void()> changed;

    /// Constructs model with stored value constructed from specified arguments
    template <typename ... Args>
    requires (std::constructible_from<Value, Args...>)
    basic_model(Args && ... args):
        value_{std::forward<Args>(args)...} {}

    /// Constructs model with stored value constructed from initializer list
    template <typename T>
    requires (std::constructible_from<Value, std::initializer_list<T>>)
    basic_model(std::initializer_list<T> init):
        value_(init) {}

    /// Returns const reference to stored value
    const Value & get() const {
        return value_;
    }

    /// Returns const reference to stored value
    const Value & operator*() const {
        return get();
    }

    /// Returns const pointer to stored value
    const Value * operator->() const {
        return &get();
    }

    /// Returns model mutator
    auto mut() {
        return mutator{this};
    }

private:
    Value value_;           ///< Stored value
};


static_assert(mutator<basic_model<int>::mutator, int>);
static_assert(model_of<basic_model<int>, int>);


}
