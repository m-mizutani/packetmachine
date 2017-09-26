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


#include <stdlib.h>
#include <string.h>

#include "./packet.hpp"
#include "./debug.hpp"

namespace pm {

Packet::Packet() : buf_len_(0), buf_(nullptr) {
}

Packet::~Packet() {
  if (this->buf_) {
    ::free(this->buf_);
  }
}

bool Packet::store(const byte_t* data, uint64_t len) {
  if (this->buf_ == nullptr || this->buf_len_ < len) {
    // need memory allocation.
    this->buf_ = reinterpret_cast<byte_t*>(::realloc(this->buf_, len));

    if (this->buf_ == nullptr) {
      // memory allocation error.
      return false;
    }

    this->buf_len_ = len;
  }

  ::memcpy(this->buf_, data, len);
  this->len_ = len;
  return true;
}

void Packet::set_cap_len(unsigned int cap_len) {
  this->cap_len_ = static_cast<uint64_t>(cap_len);
}

void Packet::set_tv(const timeval& tv) {
  this->tv_.tv_sec  = tv.tv_sec;
  this->tv_.tv_usec = tv.tv_usec;
}

}   // namespace pm
