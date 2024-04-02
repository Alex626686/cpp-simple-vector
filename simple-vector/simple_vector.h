#pragma once

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include "array_ptr.h"
#include <array>
#include <iterator>
#include <utility>

using namespace std;
class ReserveProxyObj {
public:
    ReserveProxyObj() = default;
    ReserveProxyObj(size_t capacity_to_reserve) :
        obj_capacity_(capacity_to_reserve)
    {}
    size_t obj_capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) :
        size_(size), capacity_(size), arr_(size)
    {
        for (auto it = begin(); it < end(); ++it) {
            *it = Type{};
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) :
        size_(size), capacity_(size), arr_(size)
    {
        fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :
        size_(init.size()), capacity_(init.size()), arr_(init.size())
    {
        copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other) :
        size_(other.size_), capacity_(other.size_), arr_(other.size_)
    {
        copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) :
        size_(other.size_), capacity_(other.size_), arr_(other.size_)
    {
        move(other.begin(), other.end(), begin());
        other.size_ = 0;
    }

    SimpleVector(ReserveProxyObj obj) :
        size_(0), capacity_(obj.obj_capacity_), arr_(obj.obj_capacity_)
    {}

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return arr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return arr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw out_of_range("out of range");
        }
        return arr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw out_of_range("out of range");
        }
        return arr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;

    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size < capacity_) {
            for (auto it = begin() + size_; it < begin() + new_size; ++it) {
                *it = Type{};
            }
            size_ = new_size;
            return;
        }
        SimpleVector new_vec(max(new_size, capacity_ * 2));
        move(begin(), end(), new_vec.arr_.Get());
        arr_.swap(new_vec.arr_);
        size_ = new_size;
        capacity_ = new_vec.capacity_;


    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector new_vec(new_capacity);
            copy(begin(), end(), new_vec.arr_.Get());
            arr_.swap(new_vec.arr_);
            capacity_ = new_capacity;
        }
    }


    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector vec(rhs);
        swap(vec);
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            size_t tmp_size = size_;
            Resize(capacity_ + 1);
            size_ = tmp_size;
        }
        arr_[size_++] = item;
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            size_t tmp_size = size_;
            Resize(capacity_ + 1);
            size_ = tmp_size;
        }
        arr_[size_++] = move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        auto p = distance(begin(), const_cast<Iterator>(pos));
        size_t tmp_size = size_;
        if (size_ == capacity_) {
            Resize(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        size_ = tmp_size + 1;
        copy_backward(begin() + p, end() - 1, end());
        arr_[p] = value;
        return begin() + p;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        auto p = distance(begin(), const_cast<Iterator>(pos));
        size_t tmp_size = size_;
        if (size_ == capacity_) {
            Resize(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        size_ = tmp_size + 1;
        move_backward(begin() + p, end() - 1, end());
        arr_[p] = move(value);
        return begin() + p;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0)
            --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto p = distance(begin(), const_cast<Iterator>(pos));
        move(const_cast<Iterator>(pos) + 1, end(), const_cast<Iterator>(pos));
        --size_;
        return begin() + p;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        arr_.swap(other.arr_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return arr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return arr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return arr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return arr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return arr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return arr_.Get() + size_;
    }



private:
    size_t size_ = 0, capacity_ = 0;
    ArrayPtr<Type> arr_;

};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize())
    {
        return false;
    }
    return equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || lhs < rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}