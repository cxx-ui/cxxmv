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
#include "model.hpp"
#include "projection.hpp"
#include <cassert>
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
                                private ref_transform_projection_base<Range> {
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
    class const_iterator: public mv::projection_base {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = ref_transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_const_iterator>;

        /// Constructs invalid iterator
        const_iterator() = default;

        /// Constructs iterator from base iterator and get reference function
        const_iterator(base_const_iterator it, const GetRefFn & fn):
            it_{it}, fn_{fn} {}

        /// Constructs iterator from base iterator and optional get reference function
        const_iterator(base_const_iterator it, const std::optional<GetRefFn> & fn):
            it_{it}, fn_{fn} {}

        /// Copy constructor
        const_iterator(const const_iterator &) = default;

        /// Copy assignment operator
        const_iterator & operator=(const const_iterator & other) {
            it_ = other.it_;
            assign_fn(fn_, other.fn_);
            return *this;
        }

        /// Returns true if iterator does not point to element
        bool is_null() const { return it_.is_null(); }

        /// Returns reference to transformed value of element
        decltype(auto) get() const { return (*fn_)(*it_); }

        /// Returns signal of base element emitted before element is changed
        decltype(auto) before_changed() const { return it_.before_changed(); }

        /// Returns signal of base element emitted after element is changed
        decltype(auto) after_changed() const { return it_.after_changed(); }

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
    class iterator: public mv::projection_base {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = ref_transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_iterator>;

        /// Constructs invalid iterator
        iterator() = default;

        /// Constructs iterator from base iterator without function, can be used only
        /// for referencing elements in projection
        iterator(base_iterator it):
            it_{it} {}

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

        /// Returns true if iterator does not point to element
        bool is_null() const { return it_.is_null(); }

        /// Returns reference to transformed value of element
        decltype(auto) get() const { return **this; }

        /// Starts mutating transformed value of element
        auto mut() const { return ref_transform_mutator{it_.mut(), *fn_}; }

        /// Returns signal of base element emitted before element is changed
        decltype(auto) before_changed() const { return it_.before_changed(); }

        /// Returns signal of base element emitted after element is changed
        decltype(auto) after_changed() const { return it_.after_changed(); }

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

        /// Converts to const iterator
        operator const_iterator() const { return {it_, fn_}; }

    private:
        base_iterator it_;                  ///< Iterator in base range
        std::optional<GetRefFn> fn_;        ///< Get reference function
    };

    /// Signal emitted before elements are inserted with iterator pointing to insert position
    /// and count of elements
    class before_inserted_signal {
    public:
        /// Constructs signal for specified projection
        before_inserted_signal(const ref_transform_projection * proj):
            proj_{proj} {}

        /// Connects function to signal
        signal_connection connect(
                const std::function<void (const const_iterator &, size_t)> & fn) const {
            return proj_->base_.before_inserted().connect(
            [get_ref_fn = proj_->get_ref_fn_, fn](const auto & pos, size_t count) {
                fn(const_iterator{pos, get_ref_fn}, count);
            });
        }

    private:
        const ref_transform_projection * proj_;     ///< Pointer to projection
    };

    /// Signal emitted after elements are inserted with range of inserted elements
    class after_inserted_signal {
    public:
        /// Constructs signal for specified projection
        after_inserted_signal(const ref_transform_projection * proj):
            proj_{proj} {}

        /// Connects function to signal
        signal_connection connect(
                const std::function<void (const const_iterator &,
                                          const const_iterator &)> & fn) const {
            return proj_->base_.after_inserted().connect(
            [get_ref_fn = proj_->get_ref_fn_, fn](const auto & first, const auto & last) {
                fn(const_iterator{first, get_ref_fn}, const_iterator{last, get_ref_fn});
            });
        }

    private:
        const ref_transform_projection * proj_;     ///< Pointer to projection
    };

    /// Signal emitted before elements are erased with range of elements to be erased
    class before_erased_signal {
    public:
        /// Constructs signal for specified projection
        before_erased_signal(const ref_transform_projection * proj):
            proj_{proj} {}

        /// Connects function to signal
        signal_connection connect(
                const std::function<void (const const_iterator &,
                                          const const_iterator &)> & fn) const {
            return proj_->base_.before_erased().connect(
            [get_ref_fn = proj_->get_ref_fn_, fn](const auto & first, const auto & last) {
                fn(const_iterator{first, get_ref_fn}, const_iterator{last, get_ref_fn});
            });
        }

    private:
        const ref_transform_projection * proj_;     ///< Pointer to projection
    };

    /// Signal emitted after elements are erased with iterator pointing to element following
    /// erased elements and count of erased elements
    class after_erased_signal {
    public:
        /// Constructs signal for specified projection
        after_erased_signal(const ref_transform_projection * proj):
            proj_{proj} {}

        /// Connects function to signal
        signal_connection connect(
                const std::function<void (const const_iterator &, size_t)> & fn) const {
            return proj_->base_.after_erased().connect(
            [get_ref_fn = proj_->get_ref_fn_, fn](const auto & pos, size_t count) {
                fn(const_iterator{pos, get_ref_fn}, count);
            });
        }

    private:
        const ref_transform_projection * proj_;     ///< Pointer to projection
    };

    /// Signal emitted before element is changed with iterator pointing to element
    class before_changed_signal {
    public:
        /// Constructs signal for specified projection
        before_changed_signal(const ref_transform_projection * proj):
            proj_{proj} {}

        /// Connects function to signal
        signal_connection connect(const std::function<void (const const_iterator &)> & fn) const {
            return proj_->base_.before_changed().connect(
            [get_ref_fn = proj_->get_ref_fn_, fn](const auto & it) {
                fn(const_iterator{it, get_ref_fn});
            });
        }

    private:
        const ref_transform_projection * proj_;     ///< Pointer to projection
    };

    /// Signal emitted after element is changed with iterator pointing to element
    class after_changed_signal {
    public:
        /// Constructs signal for specified projection
        after_changed_signal(const ref_transform_projection * proj):
            proj_{proj} {}

        /// Connects function to signal
        signal_connection connect(const std::function<void (const const_iterator &)> & fn) const {
            return proj_->base_.after_changed().connect(
            [get_ref_fn = proj_->get_ref_fn_, fn](const auto & it) {
                fn(const_iterator{it, get_ref_fn});
            });
        }

    private:
        const ref_transform_projection * proj_;     ///< Pointer to projection
    };

    /// Constructs ref transform projection with specified base range and get reference function
    ref_transform_projection(Range b, GetRefFn fn):
        ref_transform_projection_base<Range>{std::move(b)},
        get_ref_fn_{std::move(fn)} {}

    /// Copy constructor
    ref_transform_projection(const ref_transform_projection & other) = default;

    /// Move constructor
    ref_transform_projection(ref_transform_projection && other) = default;

    /// Copy assignment operator
    ref_transform_projection & operator=(const ref_transform_projection & other) = default;

    /// Move assignment operator
    ref_transform_projection & operator=(ref_transform_projection && other) = default;

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
        return ref_transform_mutator{this->base_.mut(it.base()), get_ref_fn_};
    }

    /// Reads transformed element at specified index
    decltype(auto) get(size_t idx) const {
        return *(begin() + idx);
    }

    /// Reads transformed element pointed by specified iterator
    decltype(auto) get(const iterator & it) const {
        return get_ref_fn_(this->base_.get(it.base()));
    }

    /// Returns signal emitted before items added
    before_inserted_signal before_inserted() const { return {this}; }

    /// Returns signal emitted after items added
    after_inserted_signal after_inserted() const { return {this}; }

    /// Returns signal emitted before items removed
    before_erased_signal before_erased() const { return {this}; }

    /// Returns signal emitted after items removed
    after_erased_signal after_erased() const { return {this}; }

    /// Returns signal emitted before item is changed
    before_changed_signal before_changed() const { return {this}; }

    /// Returns signal emitted after item is changed
    after_changed_signal after_changed() const { return {this}; }

    /// Returns signal of base range emitted before items moved
    decltype(auto) before_moved() const requires observable_with_move<Range> {
        return this->base_.before_moved();
    }

    /// Returns signal of base range emitted after items moved
    decltype(auto) after_moved() const requires observable_with_move<Range> {
        return this->base_.after_moved();
    }

private:
    GetRefFn get_ref_fn_;                               ///< Get reference function

private:
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
