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
#include "model.hpp"
#include "projection.hpp"
#include <compare>
#include <concepts>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>


namespace mv::ranges {


/// Transformed observable range or range model projection
template <projectable_observable Range, typename GetFn, typename SetFn = empty_set_fn>
class transform_projection: public projection_base {
    /// Type of const iterator over base range elements
    using base_const_iterator = std::ranges::iterator_t<const Range>;

    /// Type of iterator over base range elements
    using base_iterator = std::ranges::iterator_t<Range>;

    /// Type of base range elements
    using base_value = std::ranges::range_value_t<Range>;

    /// Is projection modifiable?
    static constexpr bool is_model = !std::same_as<SetFn, empty_set_fn> && model<Range, base_value>;

public:
    /// Type of transformed elements
    using value_type = std::remove_cvref_t<std::invoke_result_t<const GetFn &, const base_value &>>;

    /// Const iterator over transformed elements
    class const_iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_const_iterator>;

        /// Constructs singular iterator
        const_iterator() = default;

        /// Constructs iterator from base iterator and get function
        const_iterator(base_const_iterator it, const GetFn * fn):
            it_{it}, fn_{fn} {}

        decltype(auto) operator*() const { return (*fn_)(*it_); }
        decltype(auto) operator[](difference_type n) const { return (*fn_)(it_[n]); }

        const_iterator & operator++() { ++it_; return *this; }
        const_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
        const_iterator & operator--() { --it_; return *this; }
        const_iterator operator--(int) { auto tmp = *this; --it_; return tmp; }

        const_iterator & operator+=(difference_type n) { it_ += n; return *this; }
        const_iterator & operator-=(difference_type n) { it_ -= n; return *this; }

        friend const_iterator operator+(const_iterator it, difference_type n) { return it += n; }
        friend const_iterator operator+(difference_type n, const_iterator it) { return it += n; }
        friend const_iterator operator-(const_iterator it, difference_type n) { return it -= n; }
        friend difference_type operator-(const const_iterator & a, const const_iterator & b) { return a.it_ - b.it_; }

        friend bool operator==(const const_iterator & a, const const_iterator & b) { return a.it_ == b.it_; }
        friend auto operator<=>(const const_iterator & a, const const_iterator & b) { return a.it_ <=> b.it_; }

        /// Returns base iterator
        const base_const_iterator & base() const { return it_; }

    private:
        base_const_iterator it_;            ///< Iterator in base range
        const GetFn * fn_ = nullptr;        ///< Get function
    };

    /// Iterator over transformed elements that allows modification of elements.
    /// Assignment through dereferenced iterator modifies base element with set function.
    class iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = transform_projection::value_type;
        using difference_type = std::iter_difference_t<base_iterator>;

        /// Proxy reference to transformed element
        class reference {
        public:
            /// Constructs reference to element with specified base iterator
            reference(base_iterator it, const GetFn * gf, const SetFn * sf):
                it_{it}, get_fn_{gf}, set_fn_{sf} {}

            /// Assigns value to element with set function
            template <typename Arg>
            requires (!std::same_as<std::remove_cvref_t<Arg>, reference>)
            const reference & operator=(Arg && val) const {
                base_value base_val = *it_;
                (*set_fn_)(base_val, std::forward<Arg>(val));
                *it_ = std::move(base_val);
                return *this;
            }

            /// Assigns value of another element
            const reference & operator=(const reference & other) const {
                return *this = static_cast<value_type>(other);
            }

            /// Returns transformed value of element
            operator value_type() const {
                const base_value & base_val = *it_;
                return (*get_fn_)(base_val);
            }

        private:
            base_iterator it_;              ///< Iterator in base range
            const GetFn * get_fn_;          ///< Get function
            const SetFn * set_fn_;          ///< Set function
        };

        /// Constructs iterator
        iterator() = default;

        /// Constructs iterator from base iterator, get and set functions
        iterator(base_iterator it, const GetFn * gf, const SetFn * sf):
            it_{it}, get_fn_{gf}, set_fn_{sf} {}

        reference operator*() const { return {it_, get_fn_, set_fn_}; }
        reference operator[](difference_type n) const { return {it_ + n, get_fn_, set_fn_}; }

        iterator & operator++() { ++it_; return *this; }
        iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
        iterator & operator--() { --it_; return *this; }
        iterator operator--(int) { auto tmp = *this; --it_; return tmp; }

        iterator & operator+=(difference_type n) { it_ += n; return *this; }
        iterator & operator-=(difference_type n) { it_ -= n; return *this; }

        friend iterator operator+(iterator it, difference_type n) { return it += n; }
        friend iterator operator+(difference_type n, iterator it) { return it += n; }
        friend iterator operator-(iterator it, difference_type n) { return it -= n; }
        friend difference_type operator-(const iterator & a, const iterator & b) { return a.it_ - b.it_; }

        friend bool operator==(const iterator & a, const iterator & b) { return a.it_ == b.it_; }
        friend auto operator<=>(const iterator & a, const iterator & b) { return a.it_ <=> b.it_; }

        /// Returns base iterator
        const base_iterator & base() const { return it_; }

    private:
        base_iterator it_;                  ///< Iterator in base range
        const GetFn * get_fn_ = nullptr;    ///< Get function
        const SetFn * set_fn_ = nullptr;    ///< Set function
    };

    /// Signal forwarding base range signal with base iterators replaced by transformed ones
    template <typename BaseSig>
    class transform_signal {
    public:
        /// Constructs signal from base signal and get function
        transform_signal(BaseSig base, const GetFn * fn):
            base_{std::move(base)}, fn_{fn} {}

        /// Connects function to base signal
        template <typename F>
        signal_connection connect(const F & f) const {
            return base_.connect([f, fn = fn_](const base_const_iterator & it, auto ... args) {
                f(const_iterator{it, fn}, args...);
            });
        }

    private:
        BaseSig base_;                      ///< Base range signal
        const GetFn * fn_;                  ///< Get function
    };

    /// Constructs transform projection with specified base range, get and set functions
    transform_projection(Range b, GetFn gf, SetFn sf = {}):
        base_{std::move(b)},
        get_fn_{std::make_shared<const GetFn>(std::move(gf))},
        set_fn_{std::make_shared<const SetFn>(std::move(sf))},
        before_inserted{base_.before_inserted, get_fn_.get()},
        after_inserted{base_.after_inserted, get_fn_.get()},
        before_erased{base_.before_erased, get_fn_.get()},
        after_erased{base_.after_erased, get_fn_.get()},
        before_changed{base_.before_changed, get_fn_.get()},
        after_changed{base_.after_changed, get_fn_.get()} {}

    /// Copy constructor
    transform_projection(const transform_projection &) = default;

    /// Move constructor
    transform_projection(transform_projection && other):
        base_{std::move(other.base_)},
        get_fn_{std::move(other.get_fn_)},
        set_fn_{std::move(other.set_fn_)},
        before_inserted{base_.before_inserted, get_fn_.get()},
        after_inserted{base_.after_inserted, get_fn_.get()},
        before_erased{base_.before_erased, get_fn_.get()},
        after_erased{base_.after_erased, get_fn_.get()},
        before_changed{base_.before_changed, get_fn_.get()},
        after_changed{base_.after_changed, get_fn_.get()} {}

    /// Returns const iterator pointing to the first transformed element
    const_iterator begin() const { return {std::ranges::begin(base_), get_fn_.get()}; }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator end() const { return {std::ranges::end(base_), get_fn_.get()}; }

    /// Returns const iterator pointing to the first transformed element
    const_iterator cbegin() const { return begin(); }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator cend() const { return end(); }

    /// Returns iterator pointing to the first transformed element
    iterator begin() requires is_model {
        return {std::ranges::begin(base_), get_fn_.get(), set_fn_.get()};
    }

    /// Returns iterator pointing to one past the last transformed element
    iterator end() requires is_model {
        return {std::ranges::end(base_), get_fn_.get(), set_fn_.get()};
    }

    /// Returns number of elements
    auto size() const { return std::ranges::size(base_); }

private:
    Range base_;                                ///< Base range
    std::shared_ptr<const GetFn> get_fn_;       ///< Get function shared by all copies
    std::shared_ptr<const SetFn> set_fn_;       ///< Set function shared by all copies

public:
    /// The signal is emitted before items added
    transform_signal<decltype(Range::before_inserted)> before_inserted;

    /// The signal is emitted after items added
    transform_signal<decltype(Range::after_inserted)> after_inserted;

    /// The signal is emitted before items removed
    transform_signal<decltype(Range::before_erased)> before_erased;

    /// The signal is emitted after items removed
    transform_signal<decltype(Range::after_erased)> after_erased;

    /// The signal is emitted before item is changed
    transform_signal<decltype(Range::before_changed)> before_changed;

    /// The signal is emitted after item is changed
    transform_signal<decltype(Range::after_changed)> after_changed;
};


template <projectable_observable Range, typename GetFn, typename SetFn>
transform_projection(Range && r, GetFn, SetFn) ->
    transform_projection<all_t<Range>, GetFn, SetFn>;

template <projectable_observable Range, typename GetFn>
transform_projection(Range && r, GetFn) ->
    transform_projection<all_t<Range>, GetFn, empty_set_fn>;


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
