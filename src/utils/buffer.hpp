/*
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

#ifndef PACKETMACHINE_UTILS_BUFFER_HPP__
#define PACKETMACHINE_UTILS_BUFFER_HPP__

#include <sys/types.h>

namespace pm {

class Buffer {
 private:
  void *buf_;
  size_t len_;
  size_t buflen_;
  bool finalized_;

 public:
  Buffer();
  Buffer(const Buffer& obj);
  Buffer(const void* ptr, size_t len);
  virtual ~Buffer();

  bool operator==(const Buffer& obj) const;

  void resize(size_t len);
  void clear();
  void set(const void* ptr, size_t len);
  void append(const void* ptr, size_t len);
  virtual void finalize();

  const void* ptr() const { return (this->len_ > 0) ? this->buf_ : nullptr; }
  size_t len() const { return this->len_; }
  bool finalized() const { return this->finalized_; }
};

}  // namespace pm

#endif  // PACKETMACHINE_UTILS_LRU_HPP__
