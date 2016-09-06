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

#include <sys/time.h>
#include <pthread.h>

#include <iostream>

#include "perf.hpp"

#include "../src/packet.hpp"
#include "../src/channel.hpp"
#include "../src/debug.hpp"

class ChannelPerf : public PerfTest {
 public:
  struct Prop {
    pm::Channel<pm::Packet> *ch_;
    int count_;
  } prop_;

  static void* provider(void *obj) {
    Prop *p = static_cast<Prop*>(obj);
    pm::Channel<pm::Packet>* ch = p->ch_;

    pm::Packet *pkt;

    for (size_t i = 0; i < p->count_; i++) {
      pkt = ch->retain();
      ch->push(pkt);
    }
    printf("sent: %d\n", p->count_);
    ch->close();
    return nullptr;
  }

  static void* consumer(void *obj) {
    Prop* p = static_cast<Prop*>(obj);
    pm::Channel<pm::Packet>* ch = p->ch_;
    pm::Packet *pkt;
    int c = 0;

    while (nullptr != (pkt = ch->pull())) {
      c++;
      ch->release(pkt);
    }
    printf("recv: %d\n", c);

    return nullptr;
  }

 public:
  explicit ChannelPerf(int count) {
    this->prop_.count_ = count;
    this->prop_.ch_ = new pm::Channel<pm::Packet>();
  }
  ~ChannelPerf() {
    delete this->prop_.ch_;
  }

  void run() {
    StopWatch sw;

    pthread_t t1, t2;
    pthread_create(&t1, nullptr, provider, &this->prop_);
    pthread_create(&t2, nullptr, consumer, &this->prop_);

    sw.start();
    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
    sw.stop();

    double qps = static_cast<double>(this->prop_.count_) / sw.delta();

    printf("%d => %.6f (%f sec)\n", this->prop_.count_, qps, sw.delta());
  }
};

ChannelPerf t7_(64000000);




