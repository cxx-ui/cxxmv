// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file model_vector.hpp
/// Contains definition of the model_vector class.

#pragma once

#include "observable.hpp"
#include "signals.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>


namespace mv {


/// Vector of models
template <observable T>
class model_vector {
    /// Vector storage entry
    struct entry {
        std::unique_ptr<T> obj;     ///< Object
        size_t idx;                 ///< Current index of object in vector
    };

public:
    class iterator;

    /// Const iterator over vector elements
    class const_iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        /// Constructs invalid iterator
        const_iterator() = default;

        /// Constructs iterator pointing to specified entry of vector
        const_iterator(const model_vector * vec, const entry * ent):
            vec_{vec}, ent_{ent} {}

        const T & operator*() const { return *ent_->obj; }
        const T & operator[](difference_type n) const { return *(*this + n); }
        const T * operator->() const { return ent_->obj.get(); }

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
        friend class model_vector;
        friend class iterator;

        /// Returns index of element
        difference_type index() const {
            return static_cast<difference_type>(ent_ ? ent_->idx : (vec_ ? vec_->size() : 0));
        }

        const model_vector * vec_ = nullptr;    ///< Pointer to vector model
        const entry * ent_ = nullptr;           ///< Pointer to vector entry, null for end iterator
    };

    /// Vector element mutator
    class mutator {
    public:
        /// Constructs mutator with specified pointer to object
        mutator(T * obj):
        obj_{obj} {
            assert(!empty() && "passing null object to mutator");
        }

        /// Mutator is not copyable
        mutator(const mutator &) = delete;

        /// Move constructor
        mutator(mutator && other):
        obj_{other.obj_} {
            other.obj_ = nullptr;
        }

        /// Returns true if mutator is empty
        bool empty() const {
            return obj_ == nullptr;
        }

        /// Returns reference to object value
        T & ref() const { return *obj_; }

        /// Returns pointer to object value
        T * ptr() const { return obj_; }

        /// Returns pointer to object value
        T * operator->() const { return ptr(); }

    private:
        T * obj_ = nullptr;     ///< Pointer to object
    };


    /// Iterator over vector elements
    class iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        /// Constructs invalid iterator
        iterator() = default;

        /// Constructs iterator pointing to specified entry of vector
        iterator(model_vector * vec, entry * ent):
            vec_{vec}, ent_{ent} {}

        /// Starts mutating element
        mutator mut() const { return mutator{ent_->obj.get()}; }

        const T & operator*() const { return *ent_->obj; }
        const T & operator[](difference_type n) const { return *(*this + n); }
        const T * operator->() const { return ent_->obj.get(); }

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
        friend class model_vector;

        model_vector * vec_ = nullptr;      ///< Pointer to vector model
        entry * ent_ = nullptr;             ///< Pointer to vector entry, null for end iterator
    };

    /// Constructs empty vector
    model_vector() = default;

    /// Vector is not copyable
    model_vector(const model_vector &) = delete;

    /// Vector is not movable
    model_vector(model_vector &&) = delete;

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

    /// Inserts copy of element at specified position
    void insert(const const_iterator & pos, const T & val) {
        emplace(pos, val);
    }

    /// Inserts element at specified position with move
    void insert(const const_iterator & pos, T && val) {
        emplace(pos, std::move(val));
    }

    /// Inserts object owned by specified pointer at specified position
    void insert(const const_iterator & pos, std::unique_ptr<T> && obj) {
        insert_object(pos, std::move(obj));
    }

    /// Inserts copy of element at the end of vector
    void push_back(const T & val) {
        emplace(cend(), val);
    }

    /// Moves element to the end of vector
    void push_back(T && val) {
        emplace(cend(), std::move(val));
    }

    /// Inserts object owned by specified pointer at the end of vector
    void push_back(std::unique_ptr<T> && obj) {
        insert_object(cend(), std::move(obj));
    }

    /// Constructs and inserts element at specified position
    template <typename ... Args>
    const_iterator emplace(const const_iterator & pos, Args && ... args) {
        return insert_object(pos, std::make_unique<T>(std::forward<Args>(args)...));
    }

    /// Constructs and inserts element at the end of vector
    template <typename ... Args>
    const_iterator emplace_back(Args && ... args) {
        return emplace(cend(), std::forward<Args>(args)...);
    }

    /// Erases elements
    void erase(const const_iterator & first, const const_iterator & last) {
        if (first == last) {
            return;
        }

        auto idx = static_cast<size_t>(first.index());
        auto sz = static_cast<size_t>(last - first);
        before_erased_(idx, sz);
        storage_.erase(storage_.begin() + idx, storage_.begin() + idx + sz);
        update_indexes(idx, storage_.size());
        after_erased_(idx, sz);
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
        return *storage_.at(idx)->obj;
    }

    /// Returns const reference to element
    const T & operator[](size_t idx) const {
        return *storage_[idx]->obj;
    }

    /// Returns const reference to element pointed by specified iterator
    const T & get(const const_iterator & it) const {
        assert(it.vec_ == this && it.ent_ && "invalid iterator of vector element");
        return *it;
    }

    /// Starts mutating element at specified index
    mutator mut(size_t idx) {
        return mutator{storage_[idx]->obj.get()};
    }

    /// Starts mutating element pointed by specified iterator
    mutator mut(const iterator & it) {
        assert(it.vec_ == this && it.ent_ && "invalid iterator of vector element");
        return mutator{it.ent_->obj.get()};
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
    /// Inserts object at specified position, connects to its changed signals
    const_iterator insert_object(const const_iterator & pos, std::unique_ptr<T> obj) {
        assert(obj && "inserting null object");

        auto idx = static_cast<size_t>(pos.index());
        before_inserted_(idx, 1);

        auto ent = std::make_unique<entry>(std::move(obj), idx);

        // connections are disconnected automatically when object is destroyed
        ent->obj->before_changed().connect([this, e = ent.get()] {
            before_changed_(const_iterator{this, e});
        });

        ent->obj->after_changed().connect([this, e = ent.get()] {
            after_changed_(const_iterator{this, e});
        });

        auto res = storage_.insert(storage_.begin() + idx, std::move(ent));
        update_indexes(idx + 1, storage_.size());
        after_inserted_(idx, 1);
        return {this, res->get()};
    }

    /// Returns pointer to entry at specified index or nullptr if index is out of range
    entry * entry_at(std::ptrdiff_t idx) const {
        return idx >= 0 && static_cast<size_t>(idx) < storage_.size() ? storage_[idx].get() : nullptr;
    }

    /// Updates stored indexes of entries in range [first, last)
    void update_indexes(size_t first, size_t last) {
        for (size_t idx = first; idx < last; ++idx) {
            storage_[idx]->idx = idx;
        }
    }

    std::vector<std::unique_ptr<entry>> storage_;   ///< Vector storage

    mutable signal<void (size_t, size_t)> before_inserted_;          ///< Before inserted signal
    mutable signal<void (size_t, size_t)> after_inserted_;           ///< After inserted signal
    mutable signal<void (size_t, size_t)> before_erased_;            ///< Before erased signal
    mutable signal<void (size_t, size_t)> after_erased_;             ///< After erased signal
    mutable signal<void (const_iterator)> before_changed_;           ///< Before changed signal
    mutable signal<void (const_iterator)> after_changed_;            ///< After changed signal
    mutable signal<void (size_t, size_t, size_t)> before_moved_;     ///< Before moved signal
    mutable signal<void (size_t, size_t, size_t)> after_moved_;      ///< After moved signal
};


}
