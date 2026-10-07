// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file transform.hpp
/// Contains definition of the transform_projection adaptor for observable ranges.

#pragma once

#include "../transform.hpp"
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


/// Base class of transform projection containing base range
template <typename Range>
struct transform_projection_base {
    Range base_;            ///< Base range
};


/// Transformed observable range or range model projection
template <
    projectable_observable Range,
    std::copy_constructible GetFn,
    std::copy_constructible SetFn = empty_set_fn
>
class transform_projection: public projection_base,
                            private transform_projection_base<Range>,
                            public move_signal_refs<Range> {
    /// Type of const iterator over base range elements
    using base_const_iterator = std::ranges::iterator_t<const Range>;

    /// Type of iterator over base range elements
    using base_iterator = std::ranges::iterator_t<Range>;

    /// Type of base range elements
    using base_value = std::ranges::range_value_t<Range>;

public:
    /// Type of transformed value
    using value_type = std::remove_cvref_t<std::invoke_result_t<const GetFn &, const base_value &>>;

    /// Const iterator over transformed elements
    class const_iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_const_iterator>;

        /// Constructs invalid iterator
        const_iterator() = default;

        /// Constructs iterator from base iterator and get function
        const_iterator(base_const_iterator it, const GetFn & fn):
            it_{it}, fn_{fn} {}

        /// Copy constructor
        const_iterator(const const_iterator &) = default;

        /// Copy assignment operator
        const_iterator & operator=(const const_iterator & other) {
            it_ = other.it_;
            assign_fn(fn_, other.fn_);
            return *this;
        }

        const value_type operator*() const { return (*fn_)(*it_); }
        const value_type operator[](difference_type n) const { return (*fn_)(it_[n]); }

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
        std::optional<GetFn> fn_;           ///< Get function
    };

    /// Iterator over transformed elements that allows modification of elements with mutator
    class iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_iterator>;

        /// Constructs iterator
        iterator() = default;

        /// Constructs iterator from base iterator, get and set functions
        iterator(base_iterator it, const GetFn & gf, const SetFn & sf):
            it_{it}, get_fn_{gf}, set_fn_{sf} {}

        /// Copy constructor
        iterator(const iterator &) = default;

        /// Copy assignment operator
        iterator & operator=(const iterator & other) {
            it_ = other.it_;
            assign_fn(get_fn_, other.get_fn_);
            assign_fn(set_fn_, other.set_fn_);
            return *this;
        }

        const value_type operator*() const { return (*get_fn_)(*it_); }
        const value_type operator[](difference_type n) const { return (*get_fn_)(*(it_ + n)); }
        auto mut() const { return transform_mutator{it_.mut(), *get_fn_, *set_fn_}; }

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
        std::optional<GetFn> get_fn_;       ///< Get function
        std::optional<SetFn> set_fn_;       ///< Set function
    };

    /// Constructs transform projection with specified base range, get and set functions
    transform_projection(Range b, GetFn gf, SetFn sf = {}):
        transform_projection_base<Range>{std::move(b)},
        move_signal_refs<Range>{this->base_},
        get_fn_{std::move(gf)},
        set_fn_{std::move(sf)},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Copy constructor
    transform_projection(const transform_projection & other):
        transform_projection_base<Range>{other.base_},
        move_signal_refs<Range>{this->base_},
        get_fn_{other.get_fn_},
        set_fn_{other.set_fn_},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Move constructor
    transform_projection(transform_projection && other):
        transform_projection_base<Range>{std::move(other.base_)},
        move_signal_refs<Range>{this->base_},
        get_fn_{std::move(other.get_fn_)},
        set_fn_{std::move(other.set_fn_)},
        before_inserted{this->base_.before_inserted},
        after_inserted{this->base_.after_inserted},
        before_erased{this->base_.before_erased},
        after_erased{this->base_.after_erased},
        before_changed{this->base_.before_changed},
        after_changed{this->base_.after_changed} {}

    /// Returns const iterator pointing to the first transformed element
    const_iterator begin() const {
        return {std::ranges::begin(this->base_), get_fn_};
    }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator end() const {
        return {std::ranges::end(this->base_), get_fn_};
    }

    /// Returns const iterator pointing to the first transformed element
    const_iterator cbegin() const { return begin(); }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator cend() const { return end(); }

    /// Returns iterator pointing to the first transformed element
    iterator begin() requires model<Range, base_value> {
        return {std::ranges::begin(this->base_), get_fn_, set_fn_};
    }

    /// Returns iterator pointing to one past the last transformed element
    iterator end() requires model<Range, base_value> {
        return {std::ranges::end(this->base_), get_fn_, set_fn_};
    }

    /// Returns number of elements
    auto size() const { return std::ranges::size(this->base_); }

    /// Starts mutating of element at specified index
    auto mut(size_t idx) requires (!std::same_as<SetFn, empty_set_fn>) {
        return transform_mutator{this->base_.mut(idx), get_fn_, set_fn_};
    }

    /// Starts mutating of element pointed by iterator
    auto mut(const iterator & it) requires (!std::same_as<SetFn, empty_set_fn>) {
        auto idx = static_cast<size_t>(std::distance(begin(), it));
        return transform_mutator{this->base_.mut(idx), get_fn_, set_fn_};
    }

    /// Returns handle of element at specified index
    auto handle(size_t idx) requires has_element_handle<Range> {
        return this->base_.handle(idx);
    }

private:
    GetFn get_fn_;                                      ///< Get function
    SetFn set_fn_;                                      ///< Set function

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
    friend class element_model<transform_projection>;

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


template <projectable_observable Range, typename GetFn, typename SetFn>
transform_projection(Range && r, GetFn, SetFn) ->
    transform_projection<all_t<Range>, GetFn, SetFn>;

template <projectable_observable Range, typename GetFn>
transform_projection(Range && r, GetFn) ->
    transform_projection<all_t<Range>, GetFn, empty_set_fn>;


/// Model of element in transform projection, defined if element model is defined
/// for base range
template <typename Range, typename GetFn, typename SetFn>
requires requires { sizeof(element_model<Range>); }
class element_model<transform_projection<Range, GetFn, SetFn>> {
public:
    /// Type of element handle
    using handle_type = element_model<Range>::handle_type;

    /// Constructs model of element referenced by handle in base range
    element_model(transform_projection<Range, GetFn, SetFn> & proj,
                  const handle_type & handle = {}):
        base_{handle},
        get_fn_{proj.get_fn_},
        set_fn_{proj.set_fn_},
        changed{base_.changed} {}

    /// Move constructor
    element_model(element_model && other):
        base_{std::move(other.base_)},
        get_fn_{std::move(other.get_fn_)},
        set_fn_{std::move(other.set_fn_)},
        changed{base_.changed} {}

    /// Returns true if element was removed from base range
    bool is_null() const {
        return base_.is_null();
    }

    /// Reads transformed value of element
    decltype(auto) get() const {
        return get_fn_(base_.get());
    }

    /// Reads transformed value of element
    decltype(auto) operator*() const {
        return get();
    }

    /// Starts mutating of transformed value of element
    auto mut() requires (!std::same_as<SetFn, empty_set_fn>) {
        return transform_mutator{base_.mut(), get_fn_, set_fn_};
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
    element_model<Range> base_;                 ///< Model of element in base range
    GetFn get_fn_;                              ///< Get function
    SetFn set_fn_;                              ///< Set function

public:
    /// The signal is emitted after element is changed
    signal_ref<decltype(element_model<Range>::changed)> changed;
};


/// Element handle type for transform projection
template <typename Range, typename GetFn, typename SetFn>
requires requires { typename element_handle<Range>; }
struct element_handle_impl<transform_projection<Range, GetFn, SetFn>> {
    using type = element_handle<Range>;
};


template <typename GetFn, typename SetFn>
class transform_adaptor_closure {
public:
    transform_adaptor_closure(const GetFn & gf, const SetFn & sf):
    get_fn_{gf}, set_fn_{sf} {}

    template <projectable_observable Range>
    auto operator()(Range && r) const {
        return transform_projection{std::forward<Range>(r), get_fn_, set_fn_};
    }

private:
    GetFn get_fn_;
    SetFn set_fn_;
};


template <projectable_observable Range, typename GetFn, typename SetFn>
auto operator|(Range && r, const transform_adaptor_closure<GetFn, SetFn> & c) {
    return c(std::forward<Range>(r));
}


class transform_adaptor {
public:
    constexpr transform_adaptor() = default;

    template <projectable_observable Range, typename GetFn, typename SetFn>
    auto operator()(Range && r, GetFn && gf, SetFn && sf) const {
        return transform_projection{std::forward<Range>(r), std::forward<GetFn>(gf),
                                    std::forward<SetFn>(sf)};
    }

    template <typename GetFn, typename SetFn>
    auto operator()(GetFn && gf, SetFn && sf) const {
        return transform_adaptor_closure<std::decay_t<GetFn>, std::decay_t<SetFn>>{
            std::forward<GetFn>(gf), std::forward<SetFn>(sf)};
    }

    template <projectable_observable Range, typename GetFn>
    auto operator()(Range && r, GetFn && gf) const {
        return transform_projection{std::forward<Range>(r), std::forward<GetFn>(gf)};
    }

    template <typename GetFn>
    auto operator()(GetFn && gf) const {
        return transform_adaptor_closure<std::decay_t<GetFn>, empty_set_fn>{
            std::forward<GetFn>(gf), empty_set_fn{}};
    }
};


inline constexpr auto transform = transform_adaptor{};


}


template <typename Range, typename GetFn, typename SetFn>
inline constexpr bool std::ranges::enable_borrowed_range <
    mv::ranges::transform_projection<Range, GetFn, SetFn>
> = std::ranges::enable_borrowed_range<Range>;

