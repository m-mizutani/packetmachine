/*-
 * Copyright (c) 2016 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
 * All rights reserved.
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


#include "./buffer.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../debug.hpp"

namespace pm {

Buffer::Buffer()
    : buf_(nullptr), len_(0), buflen_(0) {
}

Buffer::Buffer(const void* ptr, size_t len)
    : buf_(nullptr), len_(0), buflen_(0) {
  this->set(ptr, len);
}

Buffer::Buffer(const Buffer& obj)
    : buf_(nullptr), len_(0), buflen_(0) {
  this->set(obj.buf_, obj.len_);
}

Buffer::~Buffer() {
  if (this->buf_) {
    ::free(this->buf_);
  }
}

bool Buffer::operator==(const Buffer& obj) const {
  return (this->len_ == obj.len_ &&
          ::memcmp(this->buf_, obj.buf_, this->len_) == 0);
}

void Buffer::resize(size_t len) {
  this->buf_ = realloc(this->buf_, len);
  this->buflen_ = len;
}

void Buffer::clear() {
  this->len_ = 0;
}

void Buffer::set(const void* ptr, size_t len) {
  if (this->buflen_ < len) {
    this->resize(len);
  }

  ::memcpy(this->buf_, ptr, len);
  this->len_ = len;
}

void Buffer::append(const void* ptr, size_t len) {
  if (this->buflen_ < this->len_ + len) {
    this->resize(this->len_ + len);
  }

  uint8_t* idx = static_cast<uint8_t*>(this->buf_);
  ::memcpy(idx + this->len_, ptr, len);
  this->len_ += len;
}

}  // namespace pm
