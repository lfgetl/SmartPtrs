#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
class ControlBlock {
public:
    ControlBlock() {
        cnt_ = 0;
        weak_cnt_ = 0;
    }
    virtual ~ControlBlock() {
    }
    void Add() {
        ++(cnt_);
    }
    void Del() {
        --(cnt_);
    }
    void AddWeak() {
        ++weak_cnt_;
    }
    void DelWeak() {
        --weak_cnt_;
    }
    size_t GetCnt() const {
        return cnt_;
    }
    size_t GetWeakCnt() const {
        return weak_cnt_;
    }
    virtual void Nullify() {
    }
    bool IsObj() const {
        return obj;
    }

protected:
    size_t cnt_;
    size_t weak_cnt_;
    bool obj;
};

template <class T>
class ObjectBlock : public ControlBlock {
public:
    template <typename... Args>
    ObjectBlock(Args&&... args) {
        new (&obj_) T(std::forward<Args>(args)...);
        cnt_ = 1;
        obj = true;
        weak_cnt_ = 0;
    }
    void Nullify() {
        reinterpret_cast<T*>(&obj_)->~T();
        cleared_ = true;
    }
    ~ObjectBlock() {
        if (!cleared_) {
            reinterpret_cast<T*>(&obj_)->~T();
        }
    }
    T* Get() {
        return reinterpret_cast<T*>(&obj_);
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> obj_;
    bool cleared_ = false;
};

template <class T>
class PointerBlock : public ControlBlock {
public:
    PointerBlock(T* ptr) : ptr_(ptr) {
        if (ptr != nullptr) {
            Add();
        }
        obj = false;
    }

    void Nullify() {
        delete ptr_;
        ptr_ = nullptr;
    }
    ~PointerBlock() {
        delete ptr_;
    }
    T* Get() {
        return ptr_;
    }

private:
    T* ptr_ = nullptr;
};
class EnableBase {};
template <typename T>
class EnableSharedFromThis : public EnableBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(this->block_, dynamic_cast<T*>(ptr_));
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(this->block_, dynamic_cast<const T*>(ptr_));
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>(SharedFromThis());
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(SharedFromThis());
    }
    virtual void Null() {
    }
    void Setter(ControlBlock* block, T* ptr) {
        block_ = block;
        ptr_ = ptr;
    }

private:
    ControlBlock* block_ = nullptr;
    T* ptr_ = nullptr;

    friend class SharedPtr<T>;
};

template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    template <class Y>
    friend class ObjectBlock;
    template <typename Y>
    friend class SharedPtr;
    SharedPtr() {
    }
    SharedPtr(std::nullptr_t) {
    }
    template <class Y>
    explicit SharedPtr(Y* ptr) {
        data_ = static_cast<T*>(ptr);
        block_ = new PointerBlock<Y>(ptr);
        if constexpr (std::is_convertible_v<Y, EnableBase>) {
            ptr->Setter(block_, ptr);
        }
    }
    template <class Y>
    SharedPtr(const SharedPtr<Y>& other) {
        data_ = static_cast<T*>(other.data_);
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->Add();
        }
        if constexpr (std::is_convertible_v<Y, EnableBase>) {
            data_ > Setter(block_, data_);
        }
    }
    template <class Y>
    SharedPtr(ObjectBlock<Y>* block) {
        this->block_ = block;
        this->data_ = static_cast<T*>(block->Get());
        if constexpr (std::is_convertible_v<Y, EnableBase>) {
            data_->Setter(block_, data_);
        }
    }
    SharedPtr(const SharedPtr& other) {
        data_ = other.data_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->Add();
        }
        if constexpr (std::is_convertible_v<T, EnableBase>) {
            data_->Setter(block_, data_);
        }
    }
    template <class Y>
    SharedPtr(SharedPtr<Y>&& other) {
        data_ = static_cast<T*>(other.data_);
        block_ = other.block_;
        other.data_ = nullptr;
        other.block_ = nullptr;
    }
    SharedPtr(SharedPtr&& other) {
        data_ = other.data_;
        block_ = other.block_;
        other.data_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        this->block_ = other.block_;
        block_->Add();
        this->data_ = ptr;
    }

    SharedPtr(ControlBlock* block, T* data) : block_(block), data_(data) {
        if (block_ != nullptr) {
            block_->Add();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        this->block_ = other.GetBlock();
        this->data_ = other.Get();
        if (this->block_ != nullptr) {
            if (this->block_->GetCnt() == 0) {
                throw BadWeakPtr();
            }
            block_->Add();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (*data_ == *static_cast<T*>(other.data_)) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0) {
                delete block_;
            }
        }

        data_ = static_cast<T*>(other.data_);
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->Add();
        }
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (data_ == other.data_) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0) {
                delete block_;
            }
        }

        data_ = other.data_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->Add();
        }
        return *this;
    }
    template <class Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (*data_ == *static_cast<T*>(other.data_)) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0) {
                delete block_;
            }
        }
        data_ = static_cast<T*>(other.data_);
        block_ = other.block_;
        other.data_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (data_ == other.data_) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0) {
                delete block_;
            }
        }
        data_ = other.data_;
        block_ = other.block_;
        other.data_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        data_ = nullptr;
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0 && block_->GetWeakCnt() == 0) {
                delete block_;
                block_ = nullptr;
            } else if (block_->GetCnt() == 0) {
                block_->Nullify();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        data_ = nullptr;
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0) {
                delete block_;
                block_ = nullptr;
            }
        }
    }
    template <class Y>
    void Reset(Y* ptr) {
        data_ = static_cast<T*>(ptr);
        if (block_ != nullptr) {
            block_->Del();
            if (block_->GetCnt() == 0) {
                delete block_;
                block_ = nullptr;
            }
        }
        block_ = new PointerBlock<Y>(ptr);
    }
    void Swap(SharedPtr& other) {
        std::swap(this->block_, other.block_);
        std::swap(this->data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_;
    }
    T& operator*() const {
        return *data_;
    }
    T* operator->() const {
        return data_;
    }
    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->GetCnt();
    }
    explicit operator bool() const {
        return data_ != nullptr;
    }
    ControlBlock* GetBlock() const {
        return block_;
    }

private:
    T* data_ = nullptr;
    ControlBlock* block_ = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.GetBlock();
}
// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new ObjectBlock<T>(std::forward<Args>(args)...));
}
