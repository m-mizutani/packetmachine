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


#include <unistd.h>
#include "./packet.hpp"
#include "./channel.hpp"

#include "./debug.hpp"

namespace pm {

Channel::Channel() : eos_(false) {
}

Channel::~Channel() {
}

void Channel::close() {
  this->eos_ = true;
}



MutexChannel::MutexChannel() : mutex_(PTHREAD_MUTEX_INITIALIZER) {
  pthread_cond_init(&this->cond_, nullptr);
}

MutexChannel::~MutexChannel() {
}

Packet* MutexChannel::retain_packet() {
  Packet *pkt = new Packet();
  return pkt;
}

void MutexChannel::push_packet(pm::Packet *pkt) {
  pthread_mutex_lock(&this->mutex_);
  this->queue_.push_front(pkt);
  pthread_cond_signal(&this->cond_);
  pthread_mutex_unlock(&this->mutex_);
}

Packet* MutexChannel::pull_packet() {
  Packet *pkt = nullptr;

  for (;;) {
    pthread_mutex_lock(&this->mutex_);

    if (this->queue_.size() == 0 && !this->closed()) {
      pthread_cond_wait(&this->cond_, &this->mutex_);
    }

    if (this->queue_.size() > 0) {
      pkt = this->queue_.back();
      this->queue_.pop_back();
    }

    pthread_mutex_unlock(&this->mutex_);

    if (pkt || this->closed()) {
      break;
    }

    usleep(1);
  }

  return pkt;
}

void MutexChannel::release_packet(pm::Packet *pkt) {
  delete pkt;
}



RingChannel::RingChannel() : push_idx_(0), pull_idx_(0), ring_size_(4096) {
  this->ring_.resize(this->ring_size_);
}

RingChannel::~RingChannel() {
}

Packet* RingChannel::retain_packet() {
  Packet *pkt = new Packet();
  return pkt;
}

void RingChannel::release_packet(pm::Packet *pkt) {
  delete pkt;
}


void RingChannel::push_packet(pm::Packet *pkt) {
  uint32_t n = to_idx(this->push_idx_ + 1);

  while (n == this->pull_idx_) {
    usleep(1);
  }

  this->ring_[n] = pkt;
  this->push_idx_ = n;
}

Packet* RingChannel::pull_packet() {
  uint32_t n = this->pull_idx_ + 1;
  if (n >= this->ring_size_) {
    n = 0;
  }

  while (n == to_idx(this->push_idx_ + 1)) {
    if (this->closed()) {
      return nullptr;
    }

    usleep(1);
  }

  Packet *pkt = this->ring_[n];
  this->pull_idx_ = n;

  return pkt;
}


}    // namespace pm
