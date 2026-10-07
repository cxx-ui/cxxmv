// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file vector.hpp
/// Contains definitin of the vector class.

#pragma once

#include "ranges/element_handle.hpp"
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

    /// Handle of vector element
    class handle;


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
        assert(other.before_inserted_.empty() && other.after_inserted_.empty() &&
               other.before_erased_.empty() && other.after_erased_.empty() &&
               other.before_changed_.empty() && other.after_changed_.empty() &&
               other.before_moved_.empty() && other.after_moved_.empty() &&
               "moving vector with signal connections");

        assert(other.handles_.empty() && "moving vector with element models");
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
        before_inserted_(idx, sz);
        storage_.insert(pos, first, last);
        update_inserted(idx, sz);
        after_inserted_(idx, sz);
    }

    /// Inserts element at specified position
    void insert(const const_iterator & pos, const T & val) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted_(idx, 1);
        storage_.insert(pos, val);
        update_inserted(idx, 1);
        after_inserted_(idx, 1);
    }

    /// Inserts element to specified position with move
    void insert(const const_iterator & pos, T && val) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted_(idx, 1);
        storage_.insert(pos, std::move(val));
        update_inserted(idx, 1);
        after_inserted_(idx, 1);
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
        before_inserted_(idx, 1);
        auto res = storage_.emplace(pos, std::forward<Args>(args)...);
        update_inserted(idx, 1);
        after_inserted_(idx, 1);
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
        before_erased_(idx, sz);
        storage_.erase(first, last);
        update_erased(idx, sz);
        after_erased_(idx, sz);
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

        before_moved_(first_idx, sz, dest_idx);

        auto storage_first = storage_.begin() + first_idx;
        auto storage_last = storage_.begin() + last_idx;
        auto storage_dest = storage_.begin() + dest_idx;

        if (dest_idx < first_idx) {
            std::rotate(storage_dest, storage_first, storage_last);
        } else {
            std::rotate(storage_first, storage_last, storage_dest);
        }

        update_moved(first_idx, sz, dest_idx);
        after_moved_(first_idx, sz, dest_idx);
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

    /// Returns const reference to element referenced by handle
    const T & get(const handle & h) const {
        assert(h.is_valid() && &h.model() == this && "invalid handle of vector element");
        return storage_[h.index()];
    }

    /// Starts mutating element referenced by handle
    mutator mut(const handle & h) {
        assert(h.is_valid() && &h.model() == this && "invalid handle of vector element");
        return mut(h.index());
    }

    /// Returns handle of element at specified index
    handle handle_at(size_t idx) {
        assert(idx < size() && "invalid vector element index");
        return {*this, idx};
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
    /// Assigns value to element
    void set(const storage_iterator & it, const T & val) {
        auto idx = std::distance(storage_.begin(), it);
        before_changed_(idx);
        storage_[idx] = val;
        after_changed_(idx);
    }

    /// Assigns value to element with move
    void set(const storage_iterator & it, T && val) {
        auto idx = std::distance(storage_.begin(), it);
        before_changed_(idx);
        storage_[idx] = std::move(val);
        after_changed_(idx);
    }

    /// Emits before changed signal for specified element
    void emit_before_changed(const storage_iterator it) {
        auto idx = static_cast<size_t>(std::distance(storage_.begin(), it));
        before_changed_(idx);
    }

    /// Emits after changed signal for specified element
    void emit_after_changed(const storage_iterator it) {
        auto idx = static_cast<size_t>(std::distance(storage_.begin(), it));
        after_changed_(idx);
    }

    /// Adds element handle to vector
    void add_element(handle * elem) {
        handles_.insert(elem);
    }

    /// Removes element handle from vector
    void remove_element(handle * elem) {
        handles_.erase(elem);
    }

    /// Updates indexes of element models after inserting elements
    void update_inserted(size_t idx, size_t count) {
        for (auto elem : handles_) {
            if (elem->is_valid() && elem->idx_ >= idx) {
                elem->update_index(elem->idx_ + count);
            }
        }
    }

    /// Updates indexes of element models after erasing elements
    void update_erased(size_t idx, size_t count) {
        for (auto elem : handles_) {
            if (!elem->is_valid() || elem->idx_ < idx) {
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
        for (auto elem : handles_) {
            if (!elem->is_valid()) {
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

    std::vector<T> storage_;                                    ///< Vector storage
    std::unordered_set<handle *> handles_;                     ///< Set of element handles

    mutable signal<void (size_t, size_t)> before_inserted_;          ///< Before inserted signal
    mutable signal<void (size_t, size_t)> after_inserted_;           ///< After inserted signal
    mutable signal<void (size_t, size_t)> before_erased_;            ///< Before erased signal
    mutable signal<void (size_t, size_t)> after_erased_;             ///< After erased signal
    mutable signal<void (size_t)> before_changed_;                   ///< Before changed signal
    mutable signal<void (size_t)> after_changed_;                    ///< After changed signal
    mutable signal<void (size_t, size_t, size_t)> before_moved_;     ///< Before moved signal
    mutable signal<void (size_t, size_t, size_t)> after_moved_;      ///< After moved signal
};


/// Vector model element handles. Automatically updates element index when element is moved
/// or removed in vector model.
template <typename T>
class vector<T>::handle {
public:
    /// Constructs invalid handle
    handle() = default;

    /// Copy constructor
    handle(const handle & other):
    vec_{other.vec_}, idx_{other.idx_} {
        if (vec_) {
            vec_->add_element(this);
        }
    }

    /// Copy assignment operator
    handle & operator=(const handle & other) {
        if (vec_ != other.vec_) {
            if (vec_) {
                vec_->remove_element(this);
            }

            vec_ = other.vec_;

            if (vec_) {
                vec_->add_element(this);
            }
        }

        idx_ = other.idx_;
        return *this;
    }

    /// Destroys handle, removes it from vector
    ~handle() {
        if (vec_) {
            vec_->remove_element(this);
        }
    }

    /// Returns true if handle is valid
    bool is_valid() const {
        return idx_ != SIZE_MAX;
    }

    /// Returns reference to vector model
    vector<T> & model() const {
        return *vec_;
    }

    /// Returns index of element in vector or SIZE_MAX if handle is invalid
    size_t index() const {
        return idx_;
    }


private:
    friend class vector<T>;

    /// Constructs handle of element at specified index in vector model
    handle(vector<T> & vec, size_t idx = SIZE_MAX):
    vec_{&vec}, idx_{idx} {
        assert((idx_ == SIZE_MAX || idx_ < vec_->size()) && "invalid vector element index");
        vec_->add_element(this);
    }

    /// Sets current element index without checks
    void update_index(size_t idx) {
        idx_ = idx;
    }

    vector<T> * vec_ = nullptr;         ///< Pointer to vector model
    size_t idx_ = SIZE_MAX;             ///< Current vector element index
};


/// Element handle type for vector model
template <typename T>
struct ranges::element_handle_impl<vector<T>> {
    using type = vector<T>::handle;
};


/// Model of vector element. Automatically updates element index when element
/// is moved or removed in vector model.
template <typename T>
class ranges::element_model<vector<T>> {
public:
    /// Type of element handle
    using handle_type = vector<T>::handle;

    /// Constructs model of element referenced by specified handle
    element_model(const handle_type & handle = {}):
    handle_{handle} {
        connect_signals();
    }

    /// Constructs model of element in specified vector referenced by specified handle
    element_model(vector<T> & vec, const handle_type & handle = {}):
    element_model{handle} {
        assert((!handle.is_valid() || &handle.model() == &vec) &&
               "handle references element of another vector");
    }

    /// Model is not copyable
    element_model(const element_model &) = delete;

    /// Move constructor
    element_model(element_model && other):
    handle_{other.handle_} {
        assert(other.changed_.empty() && "moving model with signal connections");
        connect_signals();
    }

    /// Model is not copy-assignable
    element_model & operator=(const element_model &) = delete;

    /// Model is not move-assignable
    element_model & operator=(element_model &) = delete;

    /// Returns true if element was removed from vector
    bool is_null() const {
        return !handle_.is_valid();
    }

    /// Returns index of element in vector or SIZE_MAX if element is null
    size_t index() const {
        return handle_.index();
    }

    /// Reads value of element
    const T & get() const {
        assert(!is_null() && "reading null vector element");
        return handle_.model()[index()];
    }

    /// Reads value of element
    const T & operator*() const {
        return get();
    }

    /// Starts mutating of element
    auto mut() {
        assert(!is_null() && "mutating null vector element");
        return handle_.model().mut(index());
    }

    /// Sets handle of element in vector. Emits changed signal.
    void set(const handle_type & handle) {
        disconnect_signals();
        handle_ = handle;
        connect_signals();
        changed_();
    }

    /// Returns signal emitted after element is changed
    signal<void ()> & changed() const {
        return changed_;
    }

private:
    /// Connects to vector signals if handle is valid
    void connect_signals() {
        if (!handle_.is_valid()) {
            return;
        }

        before_erased_con_ =
                handle_.model().before_erased().connect([this](size_t idx, size_t count) {
            if (index() >= idx && index() < idx + count) {
                disconnect_signals();
                handle_ = {};
                changed_();
            }
        });

        after_changed_con_ = handle_.model().after_changed().connect([this](size_t idx) {
            if (index() == idx) {
                changed_();
            }
        });
    }

    /// Disconnects from vector signals
    void disconnect_signals() {
        before_erased_con_.disconnect();
        after_changed_con_.disconnect();
    }

    handle_type handle_;                            ///< Handle pointing to vector element
    mutable signal<void ()> changed_;               ///< Changed signal
    scoped_signal_connection before_erased_con_;    ///< Connection to vector before_erased signal
    scoped_signal_connection after_changed_con_;    ///< Connection to vector after_changed signal
};


static_assert(ranges::observable_as<vector<int>, int>);
static_assert(ranges::model<vector<int>, int>);
static_assert(ranges::observable_with_move<vector<int>>);
static_assert(ranges::range_element_handle<ranges::element_handle<vector<int>>>);


}
