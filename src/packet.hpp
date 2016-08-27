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

#ifndef __PACKETMACHINE_PACKET_HPP__
#define __PACKETMACHINE_PACKET_HPP__

#include <sys/time.h>
#include "./packetmachine/common.hpp"

namespace pm {

// Packet is to store captured packet data(raw data, captured length,
// actual packet length, timestamp).

class Packet {
 private:
  uint64_t len_;       // actual captured data length.
  uint64_t cap_len_;   // length this packet.
  uint64_t buf_len_;   // allocated buffer length.
  byte_t *buf_;        // buffer memory pointer.
  struct timeval tv_;  // timestamp of packet arrived.

 public:
  Packet();
  ~Packet();

  bool store(const byte_t* data, uint64_t len);
  void set_cap_len(unsigned int cap_len_);
  void set_tv(const timeval& tv);
};

}   // namespace pm

#endif   // __PACKETMACHINE_PACKET_HPP__
