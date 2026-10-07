// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file vector.hpp
/// Contains definitin of the vector class.

#pragma once

#include "ranges/element_model.hpp"
#include "ranges/model.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <vector>


namespace mv {


/// Vector range model
template <typename T>
class vector {
    /// Type of iterator in vector storage
    using storage_iterator = std::vector<T>::iterator;

public:
    /// Type of const iterator over vector elements
    using const_iterator = std::vector<T>::const_iterator;


    /// Vector element mutator
    class mutator {
    public:
        /// Constructs mutator with specified pointer to vector model and
        /// iterator in vector storage
        mutator(vector * vec, storage_iterator it):
        vec_{vec}, it_{it} {
            assert(!empty() && "passing null vector to mutator");
            vec_->emit_before_changed(it_);
        }

        /// Reference is not copyable
        mutator(const mutator &) = delete;

        /// Move constructor
        mutator(mutator && other):
        vec_{other.vec_},
        it_{other.it_} {
            other.vec_ = nullptr;
        }

        /// Destroys reference, emits changed signal
        ~mutator() {
            if (!empty()) {
                vec_->emit_after_changed(it_);
            }
        }

        /// Returns true if reference is empty
        bool empty() const {
            return vec_ == nullptr;
        }

        /// Assigns value to element
        const mutator & operator=(const T & val) const {
            assert(!empty() && "assigning to empty mutator");
            *it_ = val;
            return *this;
        }

        /// Assigns value to model with move
        const mutator & operator=(T && val) const {
            assert(!empty() && "assigning to empty mutator");
            *it_ = std::move(val);
            return *this;
        }

        /// Assigns value of another reference
        const mutator & operator=(const mutator & other) const {
            return *this = other.ref();
        }

        /// Returns reference to object value
        T & ref() const { return *it_; }

        /// Returns pointer to object value
        T * ptr() const { return &ref(); }

        /// Returns pointer to object value
        T * operator->() const { return ptr(); }

    private:
        vector * vec_ = nullptr;    ///< Pointer to vector model
        storage_iterator it_;       ///< Iterator in vector storage
    };


    /// Iterator over vector elements
    class iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        /// Constructs singular iterator
        iterator() = default;

        /// Constructs iterator pointing to specified position in vector
        iterator(vector * vec, storage_iterator pos):
            vec_{vec}, it_{pos} {}

        /// Starts mutating element
        mutator mut() const { return mutator{vec_, it_}; }

        const T & operator*() const { return *it_; }
        const T & operator[](difference_type n) const { return *it_; }
        const T * operator->() const { return &*it_; }

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

        /// Converts to const iterator
        operator const_iterator() const { return it_; }

    private:
        vector * vec_ = nullptr;        ///< Pointer to storage vector
        storage_iterator it_;           ///< Iterator in storage vector
    };


    /// Constructs empty vector
    vector() = default;

    /// Constructs vector from initializer list
    vector(const std::initializer_list<T> & vals):
        storage_{vals} {}

    /// Move constructor
    vector(vector && other):
    storage_{std::move(other.storage_)} {
        assert(other.before_inserted.empty() && other.after_inserted.empty() &&
               other.before_erased.empty() && other.after_erased.empty() &&
               other.before_changed.empty() && other.after_changed.empty() &&
               other.before_moved.empty() && other.after_moved.empty() &&
               "moving vector with signal connections");

        assert(other.elements_.empty() && "moving vector with element models");
    }

    /// Returns true if vector is empty
    bool empty() const { return storage_.empty(); }

    /// Returns const iterator pointing to the first element
    auto begin() const { return storage_.begin(); }

    /// Returns const iterator pointing to one past the last element
    auto end() const { return storage_.end(); }

    /// Returns const iterator pointing to the first element
    const_iterator cbegin() const { return storage_.cbegin(); }

    /// Returns const iterator pointing to one past the last element
    const_iterator cend() const { return storage_.cend(); }

    /// Returns iterator pointing to the first element
    iterator begin() { return {this, storage_.begin()}; }

    /// Returns iterator pointing to one past the last element
    iterator end() { return {this, storage_.end()}; }

    /// Returns size of vector
    size_t size() const { return storage_.size(); }

    /// Inserts elements at specified position
    template <typename It>
    void insert(const const_iterator & pos, It first, It last) {
        if (first == last) {
            return;
        }

        auto idx = std::distance(storage_.cbegin(), pos);
        auto sz = std::distance(first, last);
        before_inserted(idx, sz);
        storage_.insert(pos, first, last);
        update_inserted(idx, sz);
        after_inserted(idx, sz);
    }

    /// Inserts element at specified position
    void insert(const const_iterator & pos, const T & val) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted(idx, 1);
        storage_.insert(pos, val);
        update_inserted(idx, 1);
        after_inserted(idx, 1);
    }

    /// Inserts element to specified position with move
    void insert(const const_iterator & pos, T && val) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted(idx, 1);
        storage_.insert(pos, std::move(val));
        update_inserted(idx, 1);
        after_inserted(idx, 1);
    }

    /// Inserts element at the end of vector
    void push_back(const T & val) {
        insert(end(), val);
    }

    /// Moves element to the end of vector
    void push_back(T && val) {
        emplace(end(), std::move(val));
    }

    /// Constructs and inserts element at specified position
    template <typename ... Args>
    const_iterator emplace(const const_iterator & pos, Args && ... args) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted(idx, 1);
        auto res = storage_.emplace(pos, std::forward<Args>(args)...);
        update_inserted(idx, 1);
        after_inserted(idx, 1);
        return res;
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

        auto idx = std::distance(storage_.cbegin(), first);
        auto sz = std::distance(first, last);
        before_erased(idx, sz);
        storage_.erase(first, last);
        update_erased(idx, sz);
        after_erased(idx, sz);
    }

    /// Erases all elements
    void clear() {
        erase(begin(), end());
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

        auto first_idx = std::distance(storage_.cbegin(), first);
        auto last_idx = std::distance(storage_.cbegin(), last);
        auto dest_idx = std::distance(storage_.cbegin(), dest);
        auto sz = last_idx - first_idx;

        before_moved(first_idx, sz, dest_idx);

        auto storage_first = storage_.begin() + first_idx;
        auto storage_last = storage_.begin() + last_idx;
        auto storage_dest = storage_.begin() + dest_idx;

        if (dest_idx < first_idx) {
            std::rotate(storage_dest, storage_first, storage_last);
        } else {
            std::rotate(storage_first, storage_last, storage_dest);
        }

        update_moved(first_idx, sz, dest_idx);
        after_moved(first_idx, sz, dest_idx);
    }

    /// Returns const reference to element
    const T & at(size_t idx) const {
        return storage_.at(idx);
    }

    /// Returns const reference to element
    const T & operator[](size_t idx) const {
        return storage_[idx];
    }

    /// Starts mutating element at specified index
    mutator mut(size_t idx) {
        return mutator{this, storage_.begin() + idx};
    }

    /// Starts mutating element pointed by specified iterator
    mutator mut(const iterator & it) {
        auto idx = static_cast<size_t>(std::distance(begin(), it));
        return mut(idx);
    }


    /// The signal is emitted before items added
    mutable signal<void (size_t, size_t)> before_inserted;

    /// The signal is emitted after items added
    mutable signal<void (size_t, size_t)> after_inserted;

    /// The signal is emitted before items removed
    mutable signal<void (size_t, size_t)> before_erased;

    /// The signal is emitted after items removed
    mutable signal<void (size_t, size_t)> after_erased;

    /// The signal is emitted after item is changed
    mutable signal<void (size_t)> before_changed;

    /// The signal is after after item is changed
    mutable signal<void (size_t)> after_changed;

    /// The signal is emitted before items moved
    mutable signal<void (size_t, size_t, size_t)> before_moved;

    /// The signal is emitted after items moved
    mutable signal<void (size_t, size_t, size_t)> after_moved;

private:
    /// Assigns value to element
    void set(const storage_iterator & it, const T & val) {
        auto idx = std::distance(storage_.begin(), it);
        before_changed(idx);
        storage_[idx] = val;
        emit_elements_changed(idx);
        after_changed(idx);
    }

    /// Assigns value to element with move
    void set(const storage_iterator & it, T && val) {
        auto idx = std::distance(storage_.begin(), it);
        before_changed(idx);
        storage_[idx] = std::move(val);
        emit_elements_changed(idx);
        after_changed(idx);
    }

    /// Emits before changed signal for specified element
    void emit_before_changed(const storage_iterator it) {
        auto idx = static_cast<size_t>(std::distance(storage_.begin(), it));
        before_changed(idx);
    }

    /// Emits after changed signal for specified element
    void emit_after_changed(const storage_iterator it) {
        auto idx = static_cast<size_t>(std::distance(storage_.begin(), it));
        emit_elements_changed(idx);
        after_changed(idx);
    }

    friend class ranges::element_model<vector>;

    /// Adds element model to vector
    void add_element(ranges::element_model<vector> * elem) {
        elements_.insert(elem);
    }

    /// Removes element model from vector
    void remove_element(ranges::element_model<vector> * elem) {
        elements_.erase(elem);
    }

    /// Updates indexes of element models after inserting elements
    void update_inserted(size_t idx, size_t count) {
        for (auto elem : elements_) {
            if (!elem->is_null() && elem->idx_ >= idx) {
                elem->update_index(elem->idx_ + count);
            }
        }
    }

    /// Updates indexes of element models after erasing elements
    void update_erased(size_t idx, size_t count) {
        for (auto elem : elements_) {
            if (elem->is_null() || elem->idx_ < idx) {
                continue;
            }

            if (elem->idx_ < idx + count) {
                elem->update_index(SIZE_MAX);
            } else {
                elem->update_index(elem->idx_ - count);
            }
        }
    }

    /// Updates indexes of element models after moving elements
    void update_moved(size_t first, size_t count, size_t dest) {
        for (auto elem : elements_) {
            if (elem->is_null()) {
                continue;
            }

            size_t idx = elem->idx_;
            if (idx >= first && idx < first + count) {
                elem->update_index((dest < first ? dest : dest - count) + (idx - first));
            } else if (dest < first && idx >= dest && idx < first) {
                elem->update_index(idx + count);
            } else if (dest > first && idx >= first + count && idx < dest) {
                elem->update_index(idx - count);
            }
        }
    }

    /// Emits changed signal for element models of element at specified index
    void emit_elements_changed(size_t idx) {
        for (auto elem : elements_) {
            if (elem->idx_ == idx) {
                elem->emit_changed();
            }
        }
    }

    std::vector<T> storage_;
    std::unordered_set<ranges::element_model<vector> *> elements_;    ///< Set of element models
};


/// Model of vector element. Automatically updates element index when element
/// is moved or removed in vector model.
template <typename T>
class ranges::element_model<vector<T>> {
public:
    /// Constructs model of element at specified index in vector model
    element_model(vector<T> & vec, size_t idx = SIZE_MAX):
    vec_{&vec}, idx_{idx} {
        assert(idx_ == SIZE_MAX || idx_ < vec_->size() && "invalid vector element index");
        vec_->add_element(this);
    }

    /// Model is not copyable
    element_model(const element_model &) = delete;

    /// Move constructor
    element_model(element_model && other):
    vec_{other.vec_}, idx_{other.idx_} {
        assert(other.changed.empty() && "moving model with signal connections");

        vec_->remove_element(&other);
        vec_->add_element(this);
        other.vec_ = nullptr;
    }

    /// Model is not copy-assignable
    element_model & operator=(const element_model &) = delete;

    /// Model is not move-assignable
    element_model & operator=(element_model &) = delete;

    /// Destroys model, removes it from vector
    ~element_model() {
        if (vec_) {
            vec_->remove_element(this);
        }
    }

    /// Returns true if element was removed from vector
    bool is_null() const {
        return idx_ == SIZE_MAX;
    }

    /// Reads value of element
    const T & get() const {
        assert(!is_null() && "reading null vector element");
        return (*vec_)[idx_];
    }

    /// Reads value of element
    const T & operator*() const {
        return get();
    }

    /// Starts mutating of element
    auto mut() {
        assert(!is_null() && "mutating null vector element");
        return vec_->mut(idx_);
    }

    /// Returns index of element in vector or SIZE_MAX if element is null
    size_t index() const {
        return idx_;
    }

    /// Sets index of element in vector. Emits changed signal.
    void set_index(size_t idx) {
        assert((idx == SIZE_MAX || idx < vec_->size()) && "invalid vector element index");
        idx_ = idx;
        changed();
    }

    /// The signal is emitted after element is changed
    mutable signal<void ()> changed;

private:
    friend class vector<T>;

    /// Sets current element index, emits changed signal if element becomes null or not null
    void update_index(size_t idx) {
        bool was_null = is_null();
        idx_ = idx;
        if (was_null != is_null()) {
            changed();
        }
    }

    /// Emits changed signal
    void emit_changed() {
        changed();
    }

    vector<T> * vec_ = nullptr;         ///< Pointer to vector model
    size_t idx_ = SIZE_MAX;             ///< Current vector element index
};


static_assert(ranges::observable_as<vector<int>, int>);
static_assert(ranges::model<vector<int>, int>);
static_assert(ranges::observable_with_move<vector<int>>);


}
