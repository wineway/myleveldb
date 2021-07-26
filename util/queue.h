//
// Created by yuwei20 on 2021/7/26.
//

#ifndef LEVELDB_QUEUE_H
#define LEVELDB_QUEUE_H

template<typename T>
struct Queue {
  alignas(64) unsigned long tail_;
  T** arr_;
  alignas(64) std::atomic<unsigned long> head_;
  unsigned mask_;
  unsigned max_;
  alignas(64) std::atomic<unsigned> count_;

  Queue(unsigned len): mask_(len - 1), max_(len), head_(0), tail_(0), arr_(new T*[len]), count_(0) {};

  int push(T* e) {
    unsigned count = count_.fetch_add(1, std::memory_order_relaxed);
    if (count >= max_) {
      count_.fetch_sub(1, std::memory_order_relaxed);
      return -1;
    }
    unsigned id = head_.fetch_add(1, std::memory_order_acq_rel);
    std::atomic_exchange_explicit(reinterpret_cast<std::atomic<T*>*>(&arr_[id & mask_]), e, std::memory_order_release);
    return 1;
  }

  T* pop() {
    auto res = std::atomic_exchange_explicit(reinterpret_cast<std::atomic<T*>*>(&arr_[tail_ & mask_]), nullptr, std::memory_order_acquire);
    if (!res) {
      return nullptr;
    }
    tail_ ++;
    count_.fetch_sub(1, std::memory_order_relaxed);
    return res;
  }
};

#endif  // LEVELDB_QUEUE_H
