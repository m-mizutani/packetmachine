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

Channel::Channel() : mutex_(PTHREAD_MUTEX_INITIALIZER), eos_(false) {
  pthread_cond_init(&this->cond_, nullptr);
}

Channel::~Channel() {
}

Packet* Channel::retain_packet() {
  Packet *pkt = new Packet();
  return pkt;
}

void Channel::push_packet(pm::Packet *pkt) {
  pthread_mutex_lock(&this->mutex_);
  this->queue_.push_front(pkt);
  pthread_cond_signal(&this->cond_);
  pthread_mutex_unlock(&this->mutex_);
}

Packet* Channel::pull_packet() {
  Packet *pkt = nullptr;

  for (;;) {
    pthread_mutex_lock(&this->mutex_);

    if (this->queue_.size() == 0 && !this->eos_.load()) {
      pthread_cond_wait(&this->cond_, &this->mutex_);
    }

    if (this->queue_.size() > 0) {
      pkt = this->queue_.back();
      this->queue_.pop_back();
    }

    pthread_mutex_unlock(&this->mutex_);

    if (pkt || this->eos_.load()) {
      break;
    }

    usleep(1);
  }

  return pkt;
}

void Channel::release_packet(pm::Packet *pkt) {
  delete pkt;
}

void Channel::close() {
  this->eos_ = true;
}

}    // namespace pm
