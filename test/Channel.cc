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
#include <random>
#include "../src/channel.hpp"
#include "./gtest/gtest.h"


namespace channel_test {

struct Data {
  int idx_;
  int data_;
  bool prime_;
};

bool prime(int n) {
  for (int i = 2; i < n; i++) {
    if (n % i == 0) {
      return false;
    }
  }

  return true;
}


namespace ring_buffer {

class Prop {
 public:
  pm::RingBuffer<Data> *ch_;

  int send_load_;
  int recv_load_;

  // access from only provider
  size_t send_count_;

  // access from only consumer
  int seq_mismatch_;
  int recv_count_;

  Prop(): ch_(nullptr), send_load_(0), recv_load_(0),
          send_count_(0), seq_mismatch_(0), recv_count_(0) {
  }
  ~Prop() = default;
};


void* provider(void* obj) {
  Prop *p = static_cast<Prop*>(obj);
  pm::RingBuffer<Data>* ch = p->ch_;
  std::random_device rnd;

  Data *d;
  int idx = 1;

  for (size_t i = 0; i < p->send_count_; i++) {
    d = ch->retain();
    d->idx_ = idx;
    d->data_ = rnd();

    if (p->send_load_ > 0) {
      d->prime_ = prime(d->data_ % p->send_load_);
    }

    ch->push(d);
    idx++;
  }

  ch->close();
  return nullptr;
}


void* consumer(void* obj) {
  Prop *p = static_cast<Prop*>(obj);
  pm::RingBuffer<Data>* ch = p->ch_;
  Data* d;

  int prev_idx = 0;

  while (nullptr != (d = ch->pull())) {
    if (p->recv_load_ > 0) {
      d->prime_ = prime(d->data_ % p->recv_load_);
    }

    p->recv_count_ += 1;
    if (prev_idx + 1 != d->idx_) {
      p->seq_mismatch_++;
    }
    prev_idx = d->idx_;
    ch->release(d);
  }

  return nullptr;
}

TEST(RingBuffer, ok) {
  Prop p;
  const int count = 100000;
  p.ch_ = new pm::RingBuffer<Data>();
  p.send_count_ = count;

  pthread_t t1, t2;
  pthread_create(&t1, nullptr, provider, &p);
  pthread_create(&t2, nullptr, consumer, &p);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);

  EXPECT_EQ(p.seq_mismatch_, 0);
  EXPECT_EQ(p.recv_count_, count);
  delete p.ch_;
}

TEST(RingBuffer, ok_slow_provider) {
  Prop p;
  const int count = 10000;
  p.ch_ = new pm::RingBuffer<Data>();
  p.send_count_ = count;
  p.send_load_ = 0xffff;

  pthread_t t1, t2;
  pthread_create(&t1, nullptr, provider, &p);
  pthread_create(&t2, nullptr, consumer, &p);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);

  EXPECT_EQ(p.seq_mismatch_, 0);
  EXPECT_EQ(p.recv_count_, count);
  // printf("push_wait: %llu, pull_wait: %llu\n",
  // p.ch_->push_wait(), p.ch_->pull_wait());
  delete p.ch_;
}

TEST(RingBuffer, ok_slow_consumer) {
  Prop p;
  const int count = 10000;
  p.ch_ = new pm::RingBuffer<Data>();
  p.send_count_ = count;
  p.recv_load_ = 0xffff;

  pthread_t t1, t2;
  pthread_create(&t1, nullptr, provider, &p);
  pthread_create(&t2, nullptr, consumer, &p);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);

  EXPECT_EQ(p.seq_mismatch_, 0);
  EXPECT_EQ(p.recv_count_, count);
  // printf("push_wait: %llu, pull_wait: %llu\n",
  // p.ch_->push_wait(), p.ch_->pull_wait());
  delete p.ch_;
}


}    // namespace ring_buffer

namespace msg_queue {

struct Data {
  int p_;
};

class Args {
 public:
  pm::MsgQueue<Data*> *q_;
  int wait_;
  int recv_;
  Args(pm::MsgQueue<Data*> *q) : q_(q), wait_(0), recv_(0) {}
};

int delay(int n) {
  int c = 0;
  for (int i = 2; i < n; i++) {
    if (prime(i)) {
      c++;
    }
  }
  return c;
}

void* sender(void *obj) {
  Args* args = static_cast<Args*>(obj);
  for (int i = 0; i < 10; i++) {
    auto d = new Data();
    d->p_ = delay(0x2000);
    args->q_->push(d);
  }

  return nullptr;
}

void* recver(void *obj) {
  Args* args = static_cast<Args*>(obj);
  for (int i = 0; i < 10; i++) {
    while(!args->q_->has_msg()) {
      args->wait_++;
    }

    auto d = args->q_->pull();
    args->recv_++;
    delete d;
  }
  
  return nullptr;  
}



TEST(MsgQueue, ok){
  pm::MsgQueue<Data*> q;
  Args args(&q);
  
  pthread_t t1, t2;
  pthread_create(&t1, nullptr, sender, &args);
  pthread_create(&t2, nullptr, recver, &args);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);

  EXPECT_LT(0, args.wait_);
  EXPECT_EQ(10, args.recv_);
}

}   // namspace msg_queue

}   // namespace channel_test
