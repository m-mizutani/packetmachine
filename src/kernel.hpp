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
#include "./thread.hpp"

namespace pm {

class Kernel;

class HandlerEntity {
 private:
  Callback cb_;
  event_id ev_id_;
  hdlr_id id_;
  std::atomic<bool> active_;
  std::atomic<bool> destroyed_;
  
 public:
  HandlerEntity(hdlr_id id, Callback cb, event_id ev_id);
  ~HandlerEntity();
  inline hdlr_id id() const { return this->id_; }
  inline event_id ev_id() const { return this->ev_id_; }
  inline Callback& callback() { return this->cb_; }
  inline bool is_active() const { return this->active_; }
  bool activate();
  bool deactivate();
  bool destroy();
};

class ChangeRequest {
 public:
  ChangeRequest() = default;
  virtual ~ChangeRequest() = default;
  virtual bool change(Kernel* kernel) = 0;
};


typedef std::shared_ptr<HandlerEntity> HandlerPtr;
typedef std::shared_ptr<RingBuffer<Packet> > PktChannel;
typedef std::shared_ptr<MsgQueue<ChangeRequest*> > MsgChannel;



class AddHandler : public ChangeRequest {
 private:
  HandlerPtr ptr_;
 public:
  AddHandler(HandlerPtr ptr) : ptr_(ptr) {}
  bool change(Kernel *kernel);
};

class DeleteHandler : public ChangeRequest {
 private:
  HandlerPtr ptr_;
 public:
  DeleteHandler(HandlerPtr ptr) : ptr_(ptr) {}
  bool change(Kernel *kernel);
};



class Kernel : public Thread {
 private:
  PktChannel pkt_channel_;
  MsgChannel msg_channel_;
  // Channel<Packet> pkt_channel_;
  // Channel<Property> prop_channel_;
  std::shared_ptr<Decoder> dec_;
  uint64_t recv_pkt_;
  uint64_t recv_size_;
  std::vector< std::vector<HandlerPtr > > handlers_;
  std::map<hdlr_id, HandlerPtr > handler_map_;
  hdlr_id global_hdlr_id_;
  std::atomic<bool> running_;

 public:
  Kernel(const Config& config);
  ~Kernel();

  static void* thread(void* obj);
  void thread_main();
  HandlerPtr on(const std::string& event_name, Callback&& ev_callback);
  bool clear(hdlr_id hid);
  bool clear(HandlerPtr ptr);

  bool add_handler(HandlerPtr ptr);
  bool delete_handler(HandlerPtr ptr);

  
  PktChannel pkt_channel() { return this->pkt_channel_; }
  
  uint64_t recv_pkt()  const { return this->recv_pkt_; }
  uint64_t recv_size() const { return this->recv_size_; }

  const Decoder& dec() const { return *(this->dec_); }
};

}   // namespace pm

#endif    // __PACKETMACHINE_KERNEL_HPP__
