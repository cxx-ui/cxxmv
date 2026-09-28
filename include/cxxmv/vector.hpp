
/// \file vector.hpp
/// Contains definitin of the vector class.

#pragma once

#include "ranges/model.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>


namespace mv {


/// Vector range model
template <typename T>
class vector {
public:
    /// Type of const iterator over vector elements
    using const_iterator = std::vector<T>::const_iterator;

    /// Iterator over vector elements
    class iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        /// Proxy reference to vector element
        class reference {
        public:
            /// Constructs reference to element with specified storage iterator
            reference(vector * vec, const_iterator it):
                vec_{vec}, it_{it} {}

            /// Assigns value to element. Emits changed signals.
            const reference & operator=(const T & val) const {
                vec_->set(it_, val);
                return *this;
            }

            /// Assigns value to element with move. Emits changed signals.
            const reference & operator=(T && val) const {
                vec_->set(it_, std::move(val));
                return *this;
            }

            /// Assigns value of another element. Emits changed signals.
            const reference & operator=(const reference & other) const {
                return *this = static_cast<const T &>(other);
            }

            /// Returns const reference to element
            operator const T & () const { return *it_; }

        private:
            vector * vec_;              ///< Reference to vector
            const_iterator it_;         ///< Iterator in vector
        };

        /// Constructs singular iterator
        iterator() = default;

        /// Constructs iterator pointing to specified position in vector
        iterator(vector * vec, const_iterator pos):
            vec_{vec}, it_{pos} {}

        reference operator*() const { return {vec_, it_}; }
        reference operator[](difference_type n) const { return {vec_, it_ + n}; }
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
        vector * vec_ = nullptr;        ///< Pointer to vector
        const_iterator it_;            ///< Current iterator in vector
    };

    /// Constructs empty vector
    vector() = default;

    /// Constructs vector from initializer list
    vector(const std::initializer_list<T> & vals):
        storage_{vals} {}

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
    iterator begin() { return {this, storage_.cbegin()}; }

    /// Returns iterator pointing to one past the last element
    iterator end() { return {this, storage_.cend()}; }

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
        before_inserted(pos, sz);
        storage_.insert(pos, first, last);
        after_inserted(begin() + idx, sz);
    }

    /// Inserts element at specified position
    void insert(const const_iterator & pos, const T & val) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted(pos, 1);
        storage_.insert(pos, val);
        after_inserted(begin() + idx, 1);
    }

    /// Inserts element to specified position with move
    void insert(const const_iterator & pos, T && val) {
        auto idx = std::distance(storage_.cbegin(), pos);
        before_inserted(pos, 1);
        storage_.insert(pos, std::move(val));
        after_inserted(begin() + idx, 1);
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
        before_inserted(pos, 1);
        auto res = storage_.emplace(pos, std::forward<Args>(args)...);
        after_inserted(begin() + idx, 1);
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
        before_erased(begin() + idx, sz);
        storage_.erase(first, last);
        after_erased(begin() + idx, sz);
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

        before_moved(cbegin() + first_idx, sz, cbegin() + dest_idx);

        auto storage_first = storage_.begin() + first_idx;
        auto storage_last = storage_.begin() + last_idx;
        auto storage_dest = storage_.begin() + dest_idx;

        if (dest_idx < first_idx) {
            std::rotate(storage_dest, storage_first, storage_last);
        } else {
            std::rotate(storage_first, storage_last, storage_dest);
        }

        after_moved(cbegin() + first_idx, sz, cbegin() + dest_idx);
    }

    /// Returns const reference to element
    const T & at(size_t idx) const {
        return storage_.at(idx);
    }

    /// Returns reference wrapper for element. Assignment to it emits changed signals.
    /// Throws std::out_of_range if index is out of range.
    iterator::reference at(size_t idx) {
        if (idx >= size()) {
            throw std::out_of_range{"mv::vector::at: index out of range"};
        }

        return {this, storage_.cbegin() + idx};
    }

    /// Returns const reference to element
    const T & operator[](size_t idx) const {
        return storage_[idx];
    }


    /// The signal is emitted before items added
    mutable signal<void (const_iterator, size_t)> before_inserted;

    /// The signal is emitted after items added
    mutable signal<void (const_iterator, size_t)> after_inserted;

    /// The signal is emitted before items removed
    mutable signal<void (const_iterator, size_t)> before_erased;

    /// The signal is emitted after items removed
    mutable signal<void (const_iterator, size_t)> after_erased;

    /// The signal is emitted after item is changed
    mutable signal<void (const_iterator)> before_changed;

    /// The signal is after after item is changed
    mutable signal<void (const_iterator)> after_changed;

    /// The signal is emitted before items moved
    mutable signal<void (const_iterator, size_t, const_iterator)> before_moved;

    /// The signal is emitted after items moved
    mutable signal<void (const_iterator, size_t, const_iterator)> after_moved;

private:
    /// Assigns value to element
    void set(const const_iterator & cit, const T & val) {
        auto it = storage_.begin() + std::distance(storage_.cbegin(), cit);
        before_changed(cit);
        *it = val;
        after_changed(cit);
    }

    /// Assigns value to element with move
    void set(const const_iterator & cit, T && val) {
        auto it = storage_.begin() + std::distance(storage_.cbegin(), cit);
        before_changed(cit);
        *it = std::move(val);
        after_changed(cit);
    }

    std::vector<T> storage_;
};


static_assert(ranges::observable_as<vector<int>, int>);
static_assert(ranges::model<vector<int>, int>);
static_assert(ranges::observable_with_move<vector<int>>);


}
