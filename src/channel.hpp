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

#include <pthread.h>
#include <vector>
#include <deque>
#include <atomic>

namespace pm {

class Packet;

// Channel is thread-safe and high performance data channel between
// packet capture thread and packet decoding thread.

class Channel {
 private:
  // std::vector<Packet*> array_;
  std::deque<Packet*> queue_;
  pthread_mutex_t mutex_;
  std::atomic<bool> eos_;

 public:
  Channel();
  ~Channel();

  // for packet capture thread.
  Packet* retain_packet();
  void push_packet(Packet *pkt);

  // for packet decoding thread.
  Packet* pull_packet();
  void release_packet(Packet* pkt);

  void end_stream() {
    this->eos_ = false;
  }
};

}   // namespace pm

#endif   // __PACKETMACHINE_CHANNEL_HPP__
