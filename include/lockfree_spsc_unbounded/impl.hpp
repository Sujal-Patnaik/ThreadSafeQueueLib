#ifndef LOCKFREE_SPSC_UNBOUNDED_IMPL
#define LOCKFREE_SPSC_UNBOUNDED_IMPL

#include "defs.hpp"

template <typename T>
using queue = tsfqueue::impl::lockfree_spsc_unbounded<T>;

// template <typename T> void queue<T>::push(T value) {
    
// }
// head -> stub <-tail

// template <typename T> bool queue<T>::try_pop(T &value) {
// }

// template <typename T> void queue<T>::wait_and_pop(T &value) {
// }

// template <typename T> bool queue<T>::peek(T &value) {//Not atomic as a whole, so it is not a proper peek operation in general except spsc. 

// }

// template <typename T> bool queue<T>::empty(void) {
// }

// template <typename T> size_t queue<T>::size() const {
// }

#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??