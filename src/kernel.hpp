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

#ifndef __PACKETMACHINE_KERNEL_HPP__
#define __PACKETMACHINE_KERNEL_HPP__

#include <pthread.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "./packet.hpp"
#include "./channel.hpp"
#include "./packetmachine.hpp"
#include "./decoder.hpp"

namespace pm {

class HandlerEntity {
 private:
  Callback cb_;
  event_id ev_id_;
  hdlr_id id_;
  bool active_;
  bool destroyed_;

 public:
  HandlerEntity(hdlr_id id, Callback cb, event_id ev_id);
  ~HandlerEntity();
  inline hdlr_id id() const { return this->id_; }
  inline event_id ev_id() const { return this->ev_id_; }
  inline Callback& callback() { return this->cb_; }
  inline bool is_active() const { return this->active_; }
  bool activate();
  bool deactivate();
  void destroy();
};

typedef std::shared_ptr<HandlerEntity> HandlerPtr;

class Kernel {
 private:
  Channel<Packet> channel_;
  Decoder dec_;
  uint64_t recv_pkt_;
  uint64_t recv_size_;
  std::vector< std::vector<HandlerPtr > > handlers_;
  std::map<hdlr_id, HandlerPtr > handler_map_;
  hdlr_id global_hdlr_id_;

 public:
  Kernel();
  ~Kernel();

  static void* thread(void* obj);
  void run();
  HandlerPtr on(const std::string& event_name, Callback&& callback);

  bool clear(hdlr_id hid);
  bool clear(HandlerPtr ptr);

  Channel<Packet>* channel() { return &this->channel_; }
  uint64_t recv_pkt()  const { return this->recv_pkt_; }
  uint64_t recv_size() const { return this->recv_size_; }

  const Decoder& dec() const { return this->dec_; }
};

}   // namespace pm

#endif    // __PACKETMACHINE_KERNEL_HPP__
