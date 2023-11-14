#pragma once

#include <type_traits>
#include <utility>

template <typename F, bool first, bool b>
class CompElem {};

template <typename F>
class CompElem<F, true, true> : F {

public:
    CompElem() {
    }
    template <class T>
    CompElem(T&& value) {
    }
    F& Get() {
        return *this;
    }
    const F& Get() const {
        return *this;
    }
};
template <typename F>
class CompElem<F, true, false> {
public:
    CompElem() : value_() {
    }
    template <class T>
    CompElem(T&& value) : value_(std::forward<T>(value)) {
    }
    F& Get() {
        return value_;
    }
    const F& Get() const {
        return value_;
    }
    F value_;
};
template <typename F>
class CompElem<F, false, true> : F {

public:
    CompElem() {
    }
    template <class T>
    CompElem(T&& value) {
    }
    F& Get() {
        return *this;
    }
    const F& Get() const {
        return *this;
    }
};
template <typename F>
class CompElem<F, false, false> {
public:
    CompElem() : value_() {
    }
    template <class T>
    CompElem(T&& value) : value_(std::forward<T>(value)) {
    }
    F& Get() {
        return value_;
    }
    const F& Get() const {
        return value_;
    }
    F value_;
};

template <typename F, typename S>
class CompressedPair : public CompElem<F, true, std::is_empty_v<F> && !std::is_final_v<F>>,
                       public CompElem<S, false, std::is_empty_v<S> && !std::is_final_v<S>> {
public:
    CompressedPair() {
    }
    template <class U, class V>
    CompressedPair(U&& first, V&& second)
        : CompElem<F, true, std::is_empty_v<F> && !std::is_final_v<F>>(std::forward<U>(first)),
          CompElem<S, false, std::is_empty_v<S> && !std::is_final_v<S>>(std::forward<V>(second)) {
    }
    CompressedPair(CompressedPair& other) = default;
    CompressedPair(CompressedPair&& other) = default;
    CompressedPair& operator=(CompressedPair& other) = default;
    CompressedPair& operator=(CompressedPair&& other) = default;

    F& GetFirst() {
        return CompElem < F, true, std::is_empty_v<F> && !std::is_final_v < F >> ::Get();
    }
    const F& GetFirst() const {
        return CompElem < F, true, std::is_empty_v<F> && !std::is_final_v < F >> ::Get();
    }

    S& GetSecond() {
        return CompElem < S, false, std::is_empty_v<S> && !std::is_final_v < S >> ::Get();
    };

    const S& GetSecond() const {
        return CompElem < S, false, std::is_empty_v<S> && !std::is_final_v < S >> ::Get();
    };

};
