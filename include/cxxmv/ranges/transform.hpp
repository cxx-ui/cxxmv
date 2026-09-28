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
#include <ranges>
#include <type_traits>
#include <utility>


namespace mv::ranges {


/// Const iterator over elements of transform projection
template <typename Range, typename GetFn>
class transform_const_iterator {
    /// Type of const iterator over base range elements
    using base_const_iterator = std::ranges::iterator_t<const Range>;

public:
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = std::remove_cvref_t<
        std::invoke_result_t<const GetFn &, const std::ranges::range_value_t<Range> &>>;
    using difference_type = std::iter_difference_t<base_const_iterator>;

    /// Constructs invalid iterator
    transform_const_iterator() = default;

    /// Constructs iterator from base iterator and get function
    transform_const_iterator(base_const_iterator it, const GetFn * fn):
        it_{it}, fn_{fn} {}

    decltype(auto) operator*() const { return (*fn_)(*it_); }
    decltype(auto) operator[](difference_type n) const { return (*fn_)(it_[n]); }

    transform_const_iterator & operator++() { ++it_; return *this; }
    transform_const_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    transform_const_iterator & operator--() { --it_; return *this; }
    transform_const_iterator operator--(int) { auto tmp = *this; --it_; return tmp; }

    transform_const_iterator & operator+=(difference_type n) { it_ += n; return *this; }
    transform_const_iterator & operator-=(difference_type n) { it_ -= n; return *this; }

    friend transform_const_iterator operator+(transform_const_iterator it, difference_type n) {
        return it += n;
    }

    friend transform_const_iterator operator+(difference_type n, transform_const_iterator it) {
        return it += n;
    }

    friend transform_const_iterator operator-(transform_const_iterator it, difference_type n) {
        return it -= n;
    }

    friend difference_type operator-(const transform_const_iterator & a,
                                     const transform_const_iterator & b) {
        return a.it_ - b.it_;
    }

    friend bool operator==(const transform_const_iterator & a, const transform_const_iterator & b) {
        return a.it_ == b.it_;
    }

    friend auto operator<=>(const transform_const_iterator & a,
                            const transform_const_iterator & b) {
        return a.it_ <=> b.it_;
    }

    /// Returns base iterator
    const base_const_iterator & base() const { return it_; }

private:
    base_const_iterator it_;            ///< Iterator in base range
    const GetFn * fn_ = nullptr;        ///< Get function
};


/// Base class of transform projection with move signals. Empty for ranges without move support.
template <typename Range, typename GetFn>
class transform_move_signals {};


/// Base class of transform projection with move signals
template <observable_with_move Range, typename GetFn>
class transform_move_signals<Range, GetFn> {
public:
    /// Type of const iterator over transformed elements
    using const_iterator = transform_const_iterator<Range, GetFn>;

    /// The signal is emitted before items moved
    mutable signal<void (const_iterator, size_t, const_iterator)> before_moved;

    /// The signal is emitted after items moved
    mutable signal<void (const_iterator, size_t, const_iterator)> after_moved;

protected:
    scoped_signal_connection before_moved_con_;         ///< Connection to base before_moved
    scoped_signal_connection after_moved_con_;          ///< Connection to base after_moved
};


/// Transformed observable range or range model projection
template <projectable_observable Range, typename GetFn, typename SetFn = empty_set_fn>
class transform_projection: public projection_base, public transform_move_signals<Range, GetFn> {
    /// Type of iterator over base range elements
    using base_iterator = std::ranges::iterator_t<Range>;

    /// Type of base range elements
    using base_value = std::ranges::range_value_t<Range>;

    /// Is projection modifiable?
    static constexpr bool is_model = !std::same_as<SetFn, empty_set_fn> && model<Range, base_value>;

public:
    /// Const iterator over transformed elements
    using const_iterator = transform_const_iterator<Range, GetFn>;

    /// Type of transformed elements
    using value_type = typename const_iterator::value_type;

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

    /// Constructs transform projection with specified base range, get and set functions
    transform_projection(Range b, GetFn gf, SetFn sf = {}):
        base_{std::move(b)},
        get_fn_{std::move(gf)},
        set_fn_{std::move(sf)} {

        connect_base();
    }

    /// Copy constructor
    transform_projection(const transform_projection & other):
        base_{other.base_},
        get_fn_{other.get_fn_},
        set_fn_{other.set_fn_} {

        connect_base();
    }

    /// Move constructor
    transform_projection(transform_projection && other):
        transform_move_signals<Range, GetFn>{std::move(other)},
        base_{std::move(other.base_)},
        get_fn_{std::move(other.get_fn_)},
        set_fn_{std::move(other.set_fn_)},
        before_inserted{std::move(other.before_inserted)},
        after_inserted{std::move(other.after_inserted)},
        before_erased{std::move(other.before_erased)},
        after_erased{std::move(other.after_erased)},
        before_changed{std::move(other.before_changed)},
        after_changed{std::move(other.after_changed)} {

        other.disconnect_base();
        connect_base();
    }

    /// Returns const iterator pointing to the first transformed element
    const_iterator begin() const {
        return {std::ranges::begin(base_), &get_fn_};
    }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator end() const {
        return {std::ranges::end(base_), &get_fn_};
    }

    /// Returns const iterator pointing to the first transformed element
    const_iterator cbegin() const { return begin(); }

    /// Returns const iterator pointing to one past the last transformed element
    const_iterator cend() const { return end(); }

    /// Returns iterator pointing to the first transformed element
    iterator begin() requires is_model {
        return {std::ranges::begin(base_), &get_fn_, &set_fn_};
    }

    /// Returns iterator pointing to one past the last transformed element
    iterator end() requires is_model {
        return {std::ranges::end(base_), &get_fn_, &set_fn_};
    }

    /// Returns number of elements
    auto size() const { return std::ranges::size(base_); }

    /// The signal is emitted before items added
    mutable signal<void (const_iterator, size_t)> before_inserted;

    /// The signal is emitted after items added
    mutable signal<void (const_iterator, size_t)> after_inserted;

    /// The signal is emitted before items removed
    mutable signal<void (const_iterator, size_t)> before_erased;

    /// The signal is emitted after items removed
    mutable signal<void (const_iterator, size_t)> after_erased;

    /// The signal is emitted before item is changed
    mutable signal<void (const_iterator)> before_changed;

    /// The signal is emitted after item is changed
    mutable signal<void (const_iterator)> after_changed;

private:
    /// Connects to signals of base range to emit signals of this projection
    void connect_base() {
        before_inserted_con_ = base_.before_inserted.connect([this](auto && pos, size_t count) {
            before_inserted(const_iterator{pos, &get_fn_}, count);
        });

        after_inserted_con_ = base_.after_inserted.connect([this](auto && pos, size_t count) {
            after_inserted(const_iterator{pos, &get_fn_}, count);
        });

        before_erased_con_ = base_.before_erased.connect([this](auto && pos, size_t count) {
            before_erased(const_iterator{pos, &get_fn_}, count);
        });

        after_erased_con_ = base_.after_erased.connect([this](auto && pos, size_t count) {
            after_erased(const_iterator{pos, &get_fn_}, count);
        });

        before_changed_con_ = base_.before_changed.connect([this](auto && pos) {
            before_changed(const_iterator{pos, &get_fn_});
        });

        after_changed_con_ = base_.after_changed.connect([this](auto && pos) {
            after_changed(const_iterator{pos, &get_fn_});
        });

        if constexpr (observable_with_move<Range>) {
            this->before_moved_con_ = base_.before_moved.connect(
                [this](auto && first, size_t count, auto && dest) {
                    this->before_moved(const_iterator{first, &get_fn_},
                                       count,
                                       const_iterator{dest, &get_fn_});
                });

            this->after_moved_con_ = base_.after_moved.connect(
                [this](auto && first, size_t count, auto && dest) {
                    this->after_moved(const_iterator{first, &get_fn_},
                                      count,
                                      const_iterator{dest, &get_fn_});
                });
        }
    }

    /// Disconnects from signals of base range
    void disconnect_base() {
        before_inserted_con_.disconnect();
        after_inserted_con_.disconnect();
        before_erased_con_.disconnect();
        after_erased_con_.disconnect();
        before_changed_con_.disconnect();
        after_changed_con_.disconnect();

        if constexpr (observable_with_move<Range>) {
            this->before_moved_con_.disconnect();
            this->after_moved_con_.disconnect();
        }
    }

    Range base_;                                        ///< Base range
    GetFn get_fn_;                                      ///< Get function
    SetFn set_fn_;                                      ///< Set function

    scoped_signal_connection before_inserted_con_;      ///< Connection to base before_inserted
    scoped_signal_connection after_inserted_con_;       ///< Connection to base after_inserted
    scoped_signal_connection before_erased_con_;        ///< Connection to base before_erased
    scoped_signal_connection after_erased_con_;         ///< Connection to base after_erased
    scoped_signal_connection before_changed_con_;       ///< Connection to base before_changed
    scoped_signal_connection after_changed_con_;        ///< Connection to base after_changed
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
