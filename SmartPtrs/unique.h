#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <typeinfo>
template <typename T>
struct DefaultDeleter {
    DefaultDeleter() {
    }
    template <class K>
    DefaultDeleter(DefaultDeleter<K>& other) {
    }
    template <class K>
    DefaultDeleter(DefaultDeleter<K>&& other) {
    }
    void operator()(T* obj) {
        delete obj;
    }
};
template <class T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() {
    }
    void operator()(T* obj) {
        delete[] obj;
    }
};
// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(std::move(ptr), std::move(Deleter())) {
    }
    UniquePtr(T* ptr, Deleter deleter) : data_(std::move(ptr), std::move(deleter)) {
    }
    UniquePtr(UniquePtr& other) = delete;
    template <class K, class KDeleter>
    UniquePtr(UniquePtr<K, KDeleter>&& other) noexcept {
        if (this->Get() != other.Get()) {
            Reset();
            data_ =
                CompressedPair<T*, Deleter>(static_cast<T*>(std::move(other.Get())),
                                            static_cast<Deleter>(std::move(other.GetDeleter())));
            other.Release();
            other.Reset();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class K, class KDeleter>
    UniquePtr& operator=(UniquePtr<K, KDeleter>&& other) noexcept {
        if (this->Get() != other.Get()) {
            Reset();
            data_ =
                CompressedPair<T*, Deleter>(static_cast<T*>(std::move(other.Get())),
                                            static_cast<Deleter>(std::move(other.GetDeleter())));
            auto p = other.Release();
        }

        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }
    template <class KDeleter>
    UniquePtr& operator=(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        data_.GetSecond()(data_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* ret = Get();
        data_ = CompressedPair<T*, Deleter>(nullptr, std::move(data_.GetSecond()));
        return ret;
    }
    void Reset(T* ptr = nullptr) {
        auto p = Get();
        if (this != nullptr) {
            data_ = CompressedPair<T*, Deleter>(std::move(ptr), std::move(data_.GetSecond()));
        }
        GetDeleter()(p);
    }
    void Swap(UniquePtr& other) {
        std::swap(this->data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    T& operator*() const {
        return *data_.GetFirst();
    }

    T* operator->() const {
        return data_.GetFirst();
    };

private:
    CompressedPair<T*, Deleter> data_;
};

template <typename Deleter>
class UniquePtr<void, Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(void* ptr) : data_(ptr, std::move(Deleter())) {
    }
    UniquePtr(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        data_.second(data_.first);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void operator*() const {
        return data_.first;
    }

    void operator->() const {
        return data_.first;
    };

private:
    std::pair<void*, Deleter> data_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(std::move(ptr), std::move(Deleter())) {
    }
    UniquePtr(T* ptr, Deleter deleter) : data_(std::move(ptr), std::move(deleter)) {
    }
    UniquePtr(UniquePtr& other) = delete;
    template <class K, class KDeleter>
    UniquePtr(UniquePtr<K, KDeleter>&& other) noexcept {
        if (this->Get() != other.Get()) {
            Reset();
            data_ =
                CompressedPair<T*, Deleter>(static_cast<T*>(std::move(other.Get())),
                                            static_cast<Deleter>(std::move(other.GetDeleter())));
            other.Release();
            other.Reset();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class K, class KDeleter>
    UniquePtr& operator=(UniquePtr<K, KDeleter>&& other) noexcept {
        if (this->Get() != other.Get()) {
            Reset();
            data_ =
                CompressedPair<T*, Deleter>(static_cast<T*>(std::move(other.Get())),
                                            static_cast<Deleter>(std::move(other.GetDeleter())));
            auto p = other.Release();
        }

        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }
    template <class KDeleter>
    UniquePtr& operator=(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        data_.GetSecond()(data_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* ret = Get();
        data_ = CompressedPair<T*, Deleter>(nullptr, std::move(data_.GetSecond()));
        return ret;
    }
    void Reset(T* ptr = nullptr) {
        if (ptr != Get()) {
            GetDeleter()(Get());
            data_ = CompressedPair<T*, Deleter>(std::move(ptr), std::move(data_.GetSecond()));
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(this->data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }
    T& operator[](size_t ind) {
        return data_.GetFirst()[ind];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *data_.GetFirst();
    }
    T* operator->() const {
        return data_.GetFirst();
    };

private:
    CompressedPair<T*, Deleter> data_;
};
