#pragma once

#include <cstdint>
#include <iostream>
#include <iterator>
#include <stdexcept>

template <typename T, uint32_t capacity> class List {
  private:
    T data[capacity];
    bool occupied[capacity];

  public:
    class Iterator : public std::iterator<std::input_iterator_tag, T> {
      private:
        List<T, capacity> &list;
        uint32_t index;

      public:
        Iterator(List<T, capacity> &l, uint32_t i)
            : list(l)
            , index(i) {}

        Iterator &operator++() {
            do {
                ++index;
            } while (index < capacity && !list.occupied[index]);
            return *this;
        }

        T &operator*() {
            return list.data[index];
        }

        bool operator==(Iterator &other) {
            return &list == &other.list && index == other.index;
        }

        bool operator!=(Iterator &other) {
            return !(*this == other);
        }
    };

    Iterator begin() {
        uint32_t i = 0;
        while (i < capacity && !occupied[i]) {
            ++i;
        }
        return Iterator(*this, i);
    }

    Iterator end() {
        return Iterator(*this, capacity);
    }

    List() {
        for (uint32_t i = 0; i < capacity; ++i) {
            occupied[i] = false;
        }
    }

    bool insert(T element) {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (!occupied[i]) {
                data[i] = element;
                occupied[i] = true;
                return true;
            }
        }
        return false;
    }

    void remove(T &element) {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (occupied[i] && &data[i] == &element) {
                occupied[i] = false;
                return;
            }
        }
        throw std::runtime_error("ERROR: Can't remove, object is not in the list");
    }
};
