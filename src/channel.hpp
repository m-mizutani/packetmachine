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
  std::atomic<bool> eos_;

 public:
  Channel();
  virtual ~Channel();

  // for packet capture thread.
  virtual Packet* retain_packet() = 0;
  virtual void push_packet(Packet *pkt) = 0;

  // for packet decoding thread.
  virtual Packet* pull_packet() = 0;
  virtual void release_packet(Packet* pkt) = 0;

  void close();
  inline bool closed() const { return this->eos_.load(); }
};

class MutexChannel : public Channel {
 private:
  std::deque<Packet*> queue_;
  pthread_mutex_t mutex_;
  pthread_cond_t cond_;

 public:
  MutexChannel();
  ~MutexChannel();

  // for packet capture thread.
  virtual Packet* retain_packet();
  virtual void push_packet(Packet *pkt);

  // for packet decoding thread.
  virtual Packet* pull_packet();
  virtual void release_packet(Packet* pkt);
};

class RingChannel : public Channel {
 private:
  std::atomic<uint32_t> push_idx_;
  std::atomic<uint32_t> pull_idx_;
  std::vector<Packet*> ring_;
  uint32_t ring_size_;
  inline uint32_t to_idx(uint32_t idx) {
    return (idx >= this->ring_size_) ? 0 : idx;
  }

 public:
  RingChannel();
  ~RingChannel();

  // for packet capture thread.
  virtual Packet* retain_packet();
  virtual void push_packet(Packet *pkt);

  // for packet decoding thread.
  virtual Packet* pull_packet();
  virtual void release_packet(Packet* pkt);
};

}   // namespace pm

#endif   // __PACKETMACHINE_CHANNEL_HPP__
