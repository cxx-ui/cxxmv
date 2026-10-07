// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_transform.hpp
/// Contains definition of the ref_transform_projection adaptor for observable ranges.

#pragma once

#include "../ref_ransform.hpp"
#include "all.hpp"
#include "element_handle.hpp"
#include "element_model.hpp"
#include "model.hpp"
#include "move_signal_refs.hpp"
#include "projection.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>


namespace mv::ranges {


/// Base class of ref transform projection containing base range
template <typename Range>
struct ref_transform_projection_base {
    Range base_;            ///< Base range
};


/// Projection for observable range or range model that transforms references
/// to base range elements
template <projectable_observable Range, std::copy_constructible GetRefFn>
requires std::is_lvalue_reference_v<std::iter_reference_t<std::ranges::iterator_t<const Range>>>
class ref_transform_projection: public projection_base,
                                private ref_transform_projection_base<Range>,
                                public move_signal_refs<Range> {
    /// Type of const iterator over base range elements
    using base_const_iterator = std::ranges::iterator_t<const Range>;

    /// Type of iterator over base range elements
    using base_iterator = std::ranges::iterator_t<Range>;

    /// Type of base range elements
    using base_value = std::ranges::range_value_t<Range>;

public:
    /// Type of transformed value
    using value_type =
        std::remove_cvref_t<std::invoke_result_t<const GetRefFn &, const base_value &>>;

    /// Const iterator over transformed elements
    class const_iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = ref_transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_const_iterator>;

        /// Constructs invalid iterator
        const_iterator() = default;

        /// Constructs iterator from base iterator and get reference function
        const_iterator(base_const_iterator it, const GetRefFn & fn):
            it_{it}, fn_{fn} {}

        /// Copy constructor
        const_iterator(const const_iterator &) = default;

        /// Copy assignment operator
        const_iterator & operator=(const const_iterator & other) {
            it_ = other.it_;
            assign_fn(fn_, other.fn_);
            return *this;
        }

        decltype(auto) operator*() const { return (*fn_)(*it_); }
        decltype(auto) operator[](difference_type n) const { return (*fn_)(it_[n]); }

        const_iterator & operator++() { ++it_; return *this; }
        const_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
        const_iterator & operator--() { --it_; return *this; }
        const_iterator operator--(int) { auto tmp = *this; --it_; return tmp; }

        const_iterator & operator+=(difference_type n) { it_ += n; return *this; }
        const_iterator & operator-=(difference_type n) { it_ -= n; return *this; }

        friend const_iterator operator+(const_iterator it, difference_type n) {
            return it += n;
        }

        friend const_iterator operator+(difference_type n, const_iterator it) {
            return it += n;
        }

        friend const_iterator operator-(const_iterator it, difference_type n) {
            return it -= n;
        }

        friend difference_type operator-(const const_iterator & a, const const_iterator & b) {
            return a.it_ - b.it_;
        }

        friend bool operator==(const const_iterator & a, const const_iterator & b) {
            return a.it_ == b.it_;
        }

        friend auto operator<=>(const const_iterator & a, const const_iterator & b) {
            return a.it_ <=> b.it_;
        }

        /// Returns base iterator
        const base_const_iterator & base() const { return it_; }

    private:
        base_const_iterator it_;            ///< Iterator in base range
        std::optional<GetRefFn> fn_;        ///< Get reference function
    };

    /// Iterator over transformed elements that allows modification of elements with mutator
    class iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = ref_transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_iterator>;

        /// Constructs invalid iterator
        iterator() = default;

        /// Constructs iterator from base iterator and get reference function
        iterator(base_iterator it, const GetRefFn & fn):
            it_{it}, fn_{fn} {}

        /// Copy constructor
        iterator(const iterator &) = default;

        /// Copy assignment operator
        iterator & operator=(const iterator & other) {
            it_ = other.it_;
            assign_fn(fn_, other.fn_);
            return *this;
        }

        decltype(auto) operator*() const {
            const base_value & base_val = *it_;
            return (*fn_)(base_val);
        }

        decltype(auto) operator[](difference_type n) const { return *(*this + n); }

        auto mut() const { return ref_transform_mutator{it_.mut(), *fn_}; }

        iterator & operator++() { ++it_; return *this; }
        iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
        iterator & operator--() { --it_; return *this; }
        iterator operator--(int) { auto tmp = *this; --it_; return tmp; }

        iterator & operator+=(difference_type n) { it_ += n; return *this; }
        iterator & operator-=(difference_type n) { it_ -= n; return *this; }

        friend iterator operator+(iterator it, difference_type n) { return it += n; }
        friend iterator operator+(difference_type n, iterator it) { return it += n; }
        friend iterator operator-(iterator it, difference_type n) { return it -= n; }

        friend difference_type operator-(const iterator & a, const iterator & b) {
            return a.it_ - b.it_;
        }

        friend bool operator==(const iterator & a, const iterator & b) { return a.it_ == b.it_; }
        friend auto operator<=>(const iterator & a, const iterator & b) { return a.it_ <=> b.it_; }

        /// Returns base iterator
        const base_iterator & base() const { return it_; }

    private:
        base_iterator it_;                  ///< Iterator in base range
        std::optional<GetRefFn> fn_;        ///< Get reference function
    };

    /// Constructs ref transform projection with specified base range and get reference function
    ref_transform_projection(Range b, GetRefFn fn):
        ref_transform_projection_base<Range>{std::move(b)},
        move_signal_refs<Range>{this->base_},
        get_ref_fn_{std::move(fn)},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Copy constructor
    ref_transform_projection(const ref_transform_projection & other):
        ref_transform_projection_base<Range>{other.base_},
        move_signal_refs<Range>{this->base_},
        get_ref_fn_{other.get_ref_fn_},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Move constructor
    ref_transform_projection(ref_transform_projection && other):
        ref_transform_projection_base<Range>{std::move(other.base_)},
        move_signal_refs<Range>{this->base_},
        get_ref_fn_{std::move(other.get_ref_fn_)},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Returns const iterator pointing to the first transformed element
    const_iterator begin() const {
        return {std::ranges::begin(this->base_), get_ref_fn_};
    }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator end() const {
        return {std::ranges::end(this->base_), get_ref_fn_};
    }

    /// Returns const iterator pointing to the first transformed element
    const_iterator cbegin() const { return begin(); }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator cend() const { return end(); }

    /// Returns iterator pointing to the first transformed element
    iterator begin() requires model<Range, base_value> {
        return {std::ranges::begin(this->base_), get_ref_fn_};
    }

    /// Returns iterator pointing to one past the last transformed element
    iterator end() requires model<Range, base_value> {
        return {std::ranges::end(this->base_), get_ref_fn_};
    }

    /// Returns number of elements
    auto size() const { return std::ranges::size(this->base_); }

    /// Starts mutating of element at specified index
    auto mut(size_t idx) requires model<Range, base_value> {
        return ref_transform_mutator{this->base_.mut(idx), get_ref_fn_};
    }

    /// Starts mutating of element pointed by iterator
    auto mut(const iterator & it) requires model<Range, base_value> {
        auto idx = static_cast<size_t>(std::distance(begin(), it));
        return ref_transform_mutator{this->base_.mut(idx), get_ref_fn_};
    }

    /// Returns handle of element at specified index
    auto handle(size_t idx) requires has_element_handle<Range> {
        return this->base_.handle(idx);
    }

private:
    GetRefFn get_ref_fn_;                               ///< Get reference function

public:
    /// The signal is emitted before items added
    signal_ref<decltype(Range::before_inserted)> before_inserted;

    /// The signal is emitted after items added
    signal_ref<decltype(Range::after_inserted)> after_inserted;

    /// The signal is emitted before items removed
    signal_ref<decltype(Range::before_erased)> before_erased;

    /// The signal is emitted after items removed
    signal_ref<decltype(Range::after_erased)> after_erased;

    /// The signal is emitted before item is changed
    signal_ref<decltype(Range::before_changed)> before_changed;

    /// The signal is emitted after item is changed
    signal_ref<decltype(Range::after_changed)> after_changed;

private:
    friend class element_model<ref_transform_projection>;

    /// Assigns function stored in optional
    template <typename Fn>
    static void assign_fn(std::optional<Fn> & dst, const std::optional<Fn> & src) {
        if (src) {
            dst.emplace(*src);
        } else {
            dst.reset();
        }
    }
};


template <projectable_observable Range, typename GetRefFn>
ref_transform_projection(Range && r, GetRefFn) ->
    ref_transform_projection<all_t<Range>, GetRefFn>;


/// Model of element in ref transform projection, defined if element model is defined
/// for base range
template <typename Range, typename GetRefFn>
requires requires { sizeof(element_model<Range>); }
class element_model<ref_transform_projection<Range, GetRefFn>> {
public:
    /// Type of element handle
    using handle_type = element_model<Range>::handle_type;

    /// Constructs model of element referenced by handle in base range projection
    element_model(ref_transform_projection<Range, GetRefFn> & proj, const handle_type & handle = {}):
        base_{handle},
        get_ref_fn_{proj.get_ref_fn_},
        changed{base_.changed} {}

    /// Move constructor
    element_model(element_model && other):
        base_{std::move(other.base_)},
        get_ref_fn_{std::move(other.get_ref_fn_)},
        changed{base_.changed} {}

    /// Returns true if element was removed from base range
    bool is_null() const {
        return base_.is_null();
    }

    /// Reads transformed value of element
    decltype(auto) get() const {
        return get_ref_fn_(base_.get());
    }

    /// Reads transformed value of element
    decltype(auto) operator*() const {
        return get();
    }

    /// Starts mutating of transformed value of element
    auto mut() {
        return ref_transform_mutator{base_.mut(), get_ref_fn_};
    }

    /// Returns index of element in base range or SIZE_MAX if element is null
    size_t index() const {
        return base_.index();
    }

    /// Sets handle of element in base range. Emits changed signal.
    void set(const handle_type & handle) {
        base_.set(handle);
    }

private:
    element_model<Range> base_;                                   ///< Model of element in base range
    GetRefFn get_ref_fn_;                                   ///< Get reference function

public:
    /// The signal is emitted after element is changed
    signal_ref<decltype(element_model<Range>::changed)> changed;
};


/// Element handle type for ref transform projection
template <typename Range, typename GetRefFn>
requires requires { typename element_handle<Range>; }
struct element_handle_impl<ref_transform_projection<Range, GetRefFn>> {
    using type = element_handle<Range>;
};


template <typename GetRefFn>
class ref_transform_adaptor_closure {
public:
    ref_transform_adaptor_closure(const GetRefFn & fn):
    get_ref_fn_{fn} {}

    template <projectable_observable Range>
    auto operator()(Range && r) const {
        return ref_transform_projection{std::forward<Range>(r), get_ref_fn_};
    }

private:
    GetRefFn get_ref_fn_;
};


template <projectable_observable Range, typename GetRefFn>
auto operator|(Range && r, const ref_transform_adaptor_closure<GetRefFn> & c) {
    return c(std::forward<Range>(r));
}


class ref_transform_adaptor {
public:
    constexpr ref_transform_adaptor() = default;

    template <projectable_observable Range, typename GetRefFn>
    auto operator()(Range && r, GetRefFn && fn) const {
        return ref_transform_projection{std::forward<Range>(r), std::forward<GetRefFn>(fn)};
    }

    template <typename GetRefFn>
    auto operator()(GetRefFn && fn) const {
        return ref_transform_adaptor_closure<std::decay_t<GetRefFn>>{std::forward<GetRefFn>(fn)};
    }
};


inline constexpr auto ref_transform = ref_transform_adaptor{};


}


template <typename Range, typename GetRefFn>
inline constexpr bool std::ranges::enable_borrowed_range <
    mv::ranges::ref_transform_projection<Range, GetRefFn>
> = std::ranges::enable_borrowed_range<Range>;
