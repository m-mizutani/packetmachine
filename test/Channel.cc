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

#include <pthread.h>
#include "./gtest/gtest.h"
#include "../src/channel.hpp"

namespace channel_test {

class Data {
 public:
  int idx_;
};

struct Prop {
  pm::Channel<Data> *ch_;

  // access from only provider
  int send_count_;

  // access from only consumer
  int seq_mismatch_;
  int recv_count_;
} prop_;


void* provider(void* obj) {
  Prop *p = static_cast<Prop*>(obj);
  pm::Channel<Data>* ch = p->ch_;

  Data *d;
  int idx = 1;

  for (size_t i = 0; i < p->send_count_; i++) {
    d = ch->retain();
    d->idx_ = idx;
    ch->push(d);
    idx++;
  }

  ch->close();
  return nullptr;
}


void* consumer(void* obj) {
  Prop *p = static_cast<Prop*>(obj);
  pm::Channel<Data>* ch = p->ch_;
  Data* d;

  int prev_idx = 0;

  while (nullptr != (d = ch->pull())) {
    p->recv_count_ += 1;
    if (prev_idx + 1 != d->idx_) {
      p->seq_mismatch_++;
    }
    prev_idx = d->idx_;
    ch->release(d);
  }

  return nullptr;
}

TEST(Channel, ok) {
  const int count = 100000;
  prop_.ch_ = new pm::Channel<Data>();
  prop_.send_count_ = count;
  prop_.seq_mismatch_ = 0;
  prop_.recv_count_ = 0;

  pthread_t t1, t2;
  pthread_create(&t1, nullptr, provider, &prop_);
  pthread_create(&t2, nullptr, consumer, &prop_);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);

  EXPECT_EQ(prop_.seq_mismatch_, 0);
  EXPECT_EQ(prop_.recv_count_, count);
  delete prop_.ch_;
}

}    // namespace channel_test
