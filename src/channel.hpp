/*
 * Copyright (c) 2016 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PACKETMACHINE_CHANNEL_HPP__
#define __PACKETMACHINE_CHANNEL_HPP__

#include <unistd.h>
#include <vector>
#include <atomic>

namespace pm {

// Channel is thread-safe and high performance data channel between
// packet capture thread and packet decoding thread.

template <typename T>
class Channel {
 private:
  std::atomic<uint32_t> push_idx_;
  std::atomic<uint32_t> pull_idx_;
  std::vector<T*> ring_;
  uint32_t ring_size_;
  uint64_t push_wait_, pull_wait_;
  bool eos_;

  inline uint32_t to_idx(uint32_t idx) {
    return (idx >= this->ring_size_) ? 0 : idx;
  }

 public:
  Channel() : push_idx_(0), pull_idx_(0), ring_size_(4096),
              push_wait_(0), pull_wait_(0), eos_(false) {
    this->ring_.resize(this->ring_size_);
    for (uint32_t i = 0; i < this->ring_size_; i++) {
      this->ring_[i] = new T();
    }
  }
  ~Channel() {
  }

  // for data capture thread.
  T* retain() {
    uint32_t n = to_idx(this->push_idx_ + 1);
    return this->ring_[n];
  }

  void push(T *data) {
    uint32_t n = to_idx(this->push_idx_ + 1);

    while (n == this->pull_idx_) {
      this->push_wait_ += 1;
      usleep(1);
    }

    this->ring_[n] = data;
    this->push_idx_ = n;
  }


  // for data processing thread.
  T* pull() {
    uint32_t n = this->pull_idx_ + 1;
    if (n >= this->ring_size_) {
      n = 0;
    }

    while (n == to_idx(this->push_idx_ + 1)) {
      if (this->closed()) {
        return nullptr;
      }

      this->pull_wait_ += 1;
      usleep(1);
    }

    T* pkt = this->ring_[n];
    this->pull_idx_ = n;

    return pkt;
  }

  void release(T* data) {
    // pass
  }

  void close() {
    this->eos_ = true;
  }

  bool closed() const {
    return this->eos_;
  }
};

}   // namespace pm

#endif   // __PACKETMACHINE_CHANNEL_HPP__
