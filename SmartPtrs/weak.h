#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"
// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
        block_ = nullptr;
    }
    template <class Y>
    WeakPtr(const WeakPtr<Y>& other) {
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->AddWeak();
        }
    }

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->AddWeak();
        }
    }

    template <class Y>
    WeakPtr(WeakPtr<Y>&& other) {
        block_ = other.block_;
        other.block_ = nullptr;
    }

    WeakPtr(WeakPtr&& other) {
        block_ = other.block_;
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        if (other.GetBlock() != nullptr) {
            other.GetBlock()->AddWeak();
        }
        this->block_ = other.GetBlock();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        if (block_ != nullptr) {
            block_->DelWeak();
            if (block_->GetWeakCnt() + block_->GetCnt() == 0) {
                delete block_;
            }
        }
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->AddWeak();
        }
        return *this;
    }
    WeakPtr& operator=(const WeakPtr& other) {
        if (block_ != nullptr) {
            block_->DelWeak();
            if (block_->GetWeakCnt() + block_->GetCnt() == 0) {
                delete block_;
            }
        }
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->AddWeak();
        }
        return *this;
    }
    template <class Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        if (block_ != nullptr) {
            block_->DelWeak();
            if (block_->GetWeakCnt() + block_->GetCnt() == 0) {
                delete block_;
            }
        }
        block_ = other.block_;
        other.block_ = nullptr;
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (block_ != nullptr) {
            block_->DelWeak();
            if (block_->GetWeakCnt() + block_->GetCnt() == 0) {
                delete block_;
            }
        }
        block_ = other.block_;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_ != nullptr) {
            block_->DelWeak();
            if (block_->GetCnt() + block_->GetWeakCnt() == 0) {
                delete block_;
                block_ = nullptr;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            block_->DelWeak();
            if (block_->GetCnt() + block_->GetWeakCnt() == 0) {
                delete block_;
            }
            block_ = nullptr;
        }
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->GetCnt();
    }
    bool Expired() const {
        if (block_ == nullptr) {
            return true;
        }
        return block_->GetCnt() == 0;
    }
    SharedPtr<T> Lock() const {
        if (block_ != nullptr) {
            block_->Add();
        }
        auto ret = SharedPtr<T>(*this);
        if (block_ != nullptr) {
            block_->Del();
        }
        return ret;
    }

    ControlBlock* GetBlock() const {
        return block_;
    }

    T* Get() const {
        if (block_ == nullptr) {
            return nullptr;
        }
        if (block_->IsObj()) {
            return dynamic_cast<ObjectBlock<T>*>(block_)->Get();
        } else {
            return dynamic_cast<PointerBlock<T>*>(block_)->Get();
        }
    }

private:
    ControlBlock* block_;
};
