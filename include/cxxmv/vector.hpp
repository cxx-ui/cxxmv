// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file vector.hpp
/// Contains definitin of the vector class.

#pragma once

#include "projection.hpp"
#include "ranges/model.hpp"
#include <algorithm>
#include <compare>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


namespace mv {


/// Vector range model
template <typename T>
class vector {
    /// Vector storage entry
    struct entry {
        /// Constructs entry with specified index and value constructed from arguments
        template <typename ... Args>
        entry(size_t i, Args && ... args):
            value(std::forward<Args>(args)...), idx{i} {}

        T value;            ///< Element value
        size_t idx;         ///< Current index of element in vector
    };

public:
    class iterator;
    class before_changed_signal;
    class after_changed_signal;

    /// Const iterator over vector elements
    class const_iterator: public projection_base {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        /// Constructs invalid iterator
        const_iterator() = default;

        /// Constructs iterator pointing to specified entry of vector
        const_iterator(const vector * vec, const entry * ent):
            vec_{vec}, ent_{ent} {}

        /// Returns true if iterator does not point to element
        bool is_null() const { return ent_ == nullptr; }

        /// Returns const reference to element
        const T & get() const { return ent_->value; }

        /// Returns signal emitted before element is changed
        before_changed_signal before_changed() const { return {vec_, *this}; }

        /// Returns signal emitted after element is changed
        after_changed_signal after_changed() const { return {vec_, *this}; }

        const T & operator*() const { return ent_->value; }
        const T & operator[](difference_type n) const { return *(*this + n); }
        const T * operator->() const { return &ent_->value; }

        const_iterator & operator++() { return *this += 1; }
        const_iterator operator++(int) { auto tmp = *this; *this += 1; return tmp; }
        const_iterator & operator--() { return *this -= 1; }
        const_iterator operator--(int) { auto tmp = *this; *this -= 1; return tmp; }

        const_iterator & operator+=(difference_type n) {
            ent_ = vec_->entry_at(index() + n);
            return *this;
        }

        const_iterator & operator-=(difference_type n) { return *this += -n; }

        friend const_iterator operator+(const_iterator it, difference_type n) { return it += n; }
        friend const_iterator operator+(difference_type n, const_iterator it) { return it += n; }
        friend const_iterator operator-(const_iterator it, difference_type n) { return it -= n; }

        friend difference_type operator-(const const_iterator & a, const const_iterator & b) {
            return a.index() - b.index();
        }

        friend bool operator==(const const_iterator & a, const const_iterator & b) {
            return a.ent_ == b.ent_;
        }

        friend auto operator<=>(const const_iterator & a, const const_iterator & b) {
            return a.index() <=> b.index();
        }

    private:
        friend class vector;
        friend class iterator;

        /// Returns index of element
        difference_type index() const {
            return static_cast<difference_type>(ent_ ? ent_->idx : (vec_ ? vec_->size() : 0));
        }

        const vector * vec_ = nullptr;      ///< Pointer to vector model
        const entry * ent_ = nullptr;       ///< Pointer to vector entry, null for end iterator
    };


    /// Signal emitted before element pointed by iterator is changed
    class before_changed_signal {
    public:
        /// Constructs signal for element of specified vector
        before_changed_signal(const vector * vec, const const_iterator & elem):
            vec_{vec}, elem_{elem} {}

        /// Connects function to signal
        signal_connection connect(const std::function<void ()> & fn) const {
            assert(vec_ && "connecting to signal of null iterator");
            return vec_->before_changed_.connect([elem = elem_, fn](const const_iterator & it) {
                if (it == elem) {
                    fn();
                }
            });
        }

    private:
        const vector * vec_;        ///< Pointer to vector model
        const_iterator elem_;       ///< Iterator pointing to element
    };


    /// Signal emitted after element pointed by iterator is changed
    class after_changed_signal {
    public:
        /// Constructs signal for element of specified vector
        after_changed_signal(const vector * vec, const const_iterator & elem):
            vec_{vec}, elem_{elem} {}

        /// Connects function to signal
        signal_connection connect(const std::function<void ()> & fn) const {
            assert(vec_ && "connecting to signal of null iterator");
            return vec_->after_changed_.connect([elem = elem_, fn](const const_iterator & it) {
                if (it == elem) {
                    fn();
                }
            });
        }

    private:
        const vector * vec_;        ///< Pointer to vector model
        const_iterator elem_;       ///< Iterator pointing to element
    };


    /// Vector element mutator
    class mutator {
    public:
        /// Constructs mutator with specified pointer to vector model and entry
        mutator(vector * vec, entry * ent):
        vec_{vec}, ent_{ent} {
            assert(!empty() && "passing null vector to mutator");
            vec_->before_changed_(const_iterator{vec_, ent_});
        }

        /// Reference is not copyable
        mutator(const mutator &) = delete;

        /// Move constructor
        mutator(mutator && other):
        vec_{other.vec_},
        ent_{other.ent_} {
            other.vec_ = nullptr;
        }

        /// Destroys reference, emits changed signal
        ~mutator() {
            if (!empty()) {
                vec_->after_changed_(const_iterator{vec_, ent_});
            }
        }

        /// Returns true if reference is empty
        bool empty() const {
            return vec_ == nullptr;
        }

        /// Assigns value to element
        const mutator & operator=(const T & val) const {
            assert(!empty() && "assigning to empty mutator");
            ent_->value = val;
            return *this;
        }

        /// Assigns value to model with move
        const mutator & operator=(T && val) const {
            assert(!empty() && "assigning to empty mutator");
            ent_->value = std::move(val);
            return *this;
        }

        /// Assigns value of another reference
        const mutator & operator=(const mutator & other) const {
            return *this = other.ref();
        }

        /// Returns reference to object value
        T & ref() const { return ent_->value; }

        /// Returns pointer to object value
        T * ptr() const { return &ref(); }

        /// Returns pointer to object value
        T * operator->() const { return ptr(); }

    private:
        vector * vec_ = nullptr;    ///< Pointer to vector model
        entry * ent_ = nullptr;     ///< Pointer to vector entry
    };


    /// Iterator over vector elements
    class iterator: public projection_base {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        /// Constructs singular iterator
        iterator() = default;

        /// Constructs iterator pointing to specified entry of vector
        iterator(vector * vec, entry * ent):
            vec_{vec}, ent_{ent} {}

        /// Returns true if iterator does not point to element
        bool is_null() const { return ent_ == nullptr; }

        /// Returns const reference to element
        const T & get() const { return ent_->value; }

        /// Starts mutating element
        mutator mut() const { return mutator{vec_, ent_}; }

        /// Returns signal emitted before element is changed
        before_changed_signal before_changed() const { return {vec_, *this}; }

        /// Returns signal emitted after element is changed
        after_changed_signal after_changed() const { return {vec_, *this}; }

        const T & operator*() const { return ent_->value; }
        const T & operator[](difference_type n) const { return *(*this + n); }
        const T * operator->() const { return &ent_->value; }

        iterator & operator++() { return *this += 1; }
        iterator operator++(int) { auto tmp = *this; *this += 1; return tmp; }
        iterator & operator--() { return *this -= 1; }
        iterator operator--(int) { auto tmp = *this; *this -= 1; return tmp; }

        iterator & operator+=(difference_type n) {
            ent_ = vec_->entry_at(const_iterator{*this}.index() + n);
            return *this;
        }

        iterator & operator-=(difference_type n) { return *this += -n; }

        friend iterator operator+(iterator it, difference_type n) { return it += n; }
        friend iterator operator+(difference_type n, iterator it) { return it += n; }
        friend iterator operator-(iterator it, difference_type n) { return it -= n; }

        friend difference_type operator-(const iterator & a, const iterator & b) {
            return const_iterator{a} - const_iterator{b};
        }

        friend bool operator==(const iterator & a, const iterator & b) { return a.ent_ == b.ent_; }

        friend auto operator<=>(const iterator & a, const iterator & b) {
            return const_iterator{a} <=> const_iterator{b};
        }

        /// Converts to const iterator
        operator const_iterator() const { return {vec_, ent_}; }

    private:
        friend class vector;

        vector * vec_ = nullptr;        ///< Pointer to vector model
        entry * ent_ = nullptr;         ///< Pointer to vector entry, null for end iterator
    };


    /// Constructs empty vector
    vector() = default;

    /// Constructs vector from initializer list
    vector(const std::initializer_list<T> & vals) {
        storage_.reserve(vals.size());
        for (const auto & val : vals) {
            storage_.push_back(std::make_unique<entry>(storage_.size(), val));
        }
    }

    /// Move constructor
    vector(vector && other):
    storage_{std::move(other.storage_)} {
        assert(other.before_inserted_.empty() && other.after_inserted_.empty() &&
               other.before_erased_.empty() && other.after_erased_.empty() &&
               other.before_changed_.empty() && other.after_changed_.empty() &&
               other.before_moved_.empty() && other.after_moved_.empty() &&
               "moving vector with signal connections");
    }

    /// Returns true if vector is empty
    bool empty() const { return storage_.empty(); }

    /// Returns const iterator pointing to the first element
    const_iterator begin() const { return {this, entry_at(0)}; }

    /// Returns const iterator pointing to one past the last element
    const_iterator end() const { return {this, nullptr}; }

    /// Returns const iterator pointing to the first element
    const_iterator cbegin() const { return begin(); }

    /// Returns const iterator pointing to one past the last element
    const_iterator cend() const { return end(); }

    /// Returns iterator pointing to the first element
    iterator begin() { return {this, entry_at(0)}; }

    /// Returns iterator pointing to one past the last element
    iterator end() { return {this, nullptr}; }

    /// Returns size of vector
    size_t size() const { return storage_.size(); }

    /// Inserts elements at specified position
    template <typename It>
    void insert(const const_iterator & pos, It first, It last) {
        if (first == last) {
            return;
        }

        auto idx = static_cast<size_t>(pos.index());
        auto sz = static_cast<size_t>(std::distance(first, last));
        before_inserted_(pos, sz);

        std::vector<std::unique_ptr<entry>> ents;
        ents.reserve(sz);
        for (; first != last; ++first) {
            ents.push_back(std::make_unique<entry>(0, *first));
        }

        storage_.insert(storage_.begin() + idx,
                        std::make_move_iterator(ents.begin()),
                        std::make_move_iterator(ents.end()));

        update_indexes(idx, storage_.size());
        after_inserted_(cbegin() + idx, cbegin() + idx + sz);
    }

    /// Inserts element at specified position
    void insert(const const_iterator & pos, const T & val) {
        emplace(pos, val);
    }

    /// Inserts element to specified position with move
    void insert(const const_iterator & pos, T && val) {
        emplace(pos, std::move(val));
    }

    /// Inserts element at the end of vector
    void push_back(const T & val) {
        emplace(end(), val);
    }

    /// Moves element to the end of vector
    void push_back(T && val) {
        emplace(end(), std::move(val));
    }

    /// Constructs and inserts element at specified position
    template <typename ... Args>
    const_iterator emplace(const const_iterator & pos, Args && ... args) {
        auto idx = static_cast<size_t>(pos.index());
        before_inserted_(pos, 1);
        auto res = storage_.insert(storage_.begin() + idx,
                                   std::make_unique<entry>(idx, std::forward<Args>(args)...));
        update_indexes(idx + 1, storage_.size());
        after_inserted_(cbegin() + idx, cbegin() + idx + 1);
        return {this, res->get()};
    }

    /// Constructs and inserts element at the end of vector
    template <typename ... Args>
    const_iterator emplace_back(Args && ... args) {
        return emplace(end(), std::forward<Args>(args)...);
    }

    /// Erases elements
    void erase(const const_iterator & first, const const_iterator & last) {
        if (first == last) {
            return;
        }

        auto idx = static_cast<size_t>(first.index());
        auto sz = static_cast<size_t>(last - first);
        before_erased_(first, last);
        storage_.erase(storage_.begin() + idx, storage_.begin() + idx + sz);
        update_indexes(idx, storage_.size());
        after_erased_(cbegin() + idx, sz);
    }

    /// Erases all elements
    void clear() {
        erase(cbegin(), cend());
    }

    /// Moves elements from [first, last) to position before dest
    void move(const const_iterator & first,
              const const_iterator & last,
              const const_iterator & dest) {

        assert((dest <= first || dest >= last) && "destination should not be inside moved range");

        if (first == last || dest == first || dest == last) {
            // no move required
            return;
        }

        auto first_idx = static_cast<size_t>(first.index());
        auto last_idx = static_cast<size_t>(last.index());
        auto dest_idx = static_cast<size_t>(dest.index());
        auto sz = last_idx - first_idx;

        before_moved_(first_idx, sz, dest_idx);

        auto storage_first = storage_.begin() + first_idx;
        auto storage_last = storage_.begin() + last_idx;
        auto storage_dest = storage_.begin() + dest_idx;

        if (dest_idx < first_idx) {
            std::rotate(storage_dest, storage_first, storage_last);
            update_indexes(dest_idx, last_idx);
        } else {
            std::rotate(storage_first, storage_last, storage_dest);
            update_indexes(first_idx, dest_idx);
        }

        after_moved_(first_idx, sz, dest_idx);
    }

    /// Returns const reference to element
    const T & at(size_t idx) const {
        return storage_.at(idx)->value;
    }

    /// Returns const reference to element
    const T & operator[](size_t idx) const {
        return storage_[idx]->value;
    }

    /// Returns const reference to element pointed by specified iterator
    const T & get(const const_iterator & it) const {
        assert(it.vec_ == this && it.ent_ && "invalid iterator of vector element");
        return *it;
    }

    /// Starts mutating element at specified index
    mutator mut(size_t idx) {
        return mutator{this, storage_[idx].get()};
    }

    /// Starts mutating element pointed by specified iterator
    mutator mut(const iterator & it) {
        assert(it.vec_ == this && it.ent_ && "invalid iterator of vector element");
        return mutator{this, it.ent_};
    }

    /// Returns signal emitted before items added
    auto & before_inserted() const { return before_inserted_; }

    /// Returns signal emitted after items added
    auto & after_inserted() const { return after_inserted_; }

    /// Returns signal emitted before items removed
    auto & before_erased() const { return before_erased_; }

    /// Returns signal emitted after items removed
    auto & after_erased() const { return after_erased_; }

    /// Returns signal emitted before item is changed
    auto & before_changed() const { return before_changed_; }

    /// Returns signal emitted after item is changed
    auto & after_changed() const { return after_changed_; }

    /// Returns signal emitted before items moved
    auto & before_moved() const { return before_moved_; }

    /// Returns signal emitted after items moved
    auto & after_moved() const { return after_moved_; }

private:
    /// Returns pointer to entry at specified index or nullptr if index is out of range
    entry * entry_at(std::ptrdiff_t idx) const {
        if (idx < 0 || static_cast<size_t>(idx) >= storage_.size()) {
            return nullptr;
        }

        return storage_[idx].get();
    }

    /// Updates stored indexes of entries in range [first, last)
    void update_indexes(size_t first, size_t last) {
        for (size_t idx = first; idx < last; ++idx) {
            storage_[idx]->idx = idx;
        }
    }

    std::vector<std::unique_ptr<entry>> storage_;                   ///< Vector storage

    /// Before inserted signal
    mutable signal<void (const const_iterator &, size_t)> before_inserted_;

    /// After inserted signal
    mutable signal<void (const const_iterator &, const const_iterator &)> after_inserted_;

    /// Before erased signal
    mutable signal<void (const const_iterator &, const const_iterator &)> before_erased_;

    /// After erased signal
    mutable signal<void (const const_iterator &, size_t)> after_erased_;

    /// Before changed signal
    mutable signal<void (const const_iterator &)> before_changed_;

    /// After changed signal
    mutable signal<void (const const_iterator &)> after_changed_;

    mutable signal<void (size_t, size_t, size_t)> before_moved_;     ///< Before moved signal
    mutable signal<void (size_t, size_t, size_t)> after_moved_;      ///< After moved signal
};


static_assert(ranges::observable_as<vector<int>, int>);
static_assert(ranges::model<vector<int>, int>);
static_assert(ranges::observable_with_move<vector<int>>);
static_assert(model_of<vector<int>::iterator, int>);
static_assert(nullable_observable_as<vector<int>::iterator, int>);
static_assert(nullable_observable_as<vector<int>::const_iterator, int>);


}
