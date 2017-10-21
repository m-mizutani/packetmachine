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

#include <algorithm>
#include <unistd.h>

#include "./kernel.hpp"
#include "./packetmachine/property.hpp"
#include "./debug.hpp"

namespace pm {

HandlerEntity::HandlerEntity(hdlr_id hid, Callback cb, event_id ev_id) :
    cb_(cb), ev_id_(ev_id), id_(hid), active_(true), destroyed_(false) {
}
HandlerEntity::~HandlerEntity() {
}

bool HandlerEntity::activate() {
  if (this->destroyed_ == true || this->active_ == true) {
    return false;
  }

  this->active_ = true;
  return true;
}

bool HandlerEntity::deactivate() {
  if (this->destroyed_ == true || this->active_ == false) {
    return false;
  }

  this->active_ = false;
  return true;
}

bool HandlerEntity::destroy() {
  if (this->destroyed_ == true) {
    return false;
  }

  this->active_ = false;
  this->destroyed_ = true;
  return true;
}  

bool AddHandler::change(Kernel* kernel) {
  return kernel->add_handler(this->ptr_);
}

bool DeleteHandler::change(Kernel* kernel) {
  return kernel->delete_handler(this->ptr_);
}


// --------------------------------------------------------
// Kenrel: main process of PacketMachine

Kernel::Kernel(const Config& config) :
    pkt_channel_(new RingBuffer<Packet>),
    msg_channel_(new MsgQueue<ChangeRequest*>),
    dec_(new Decoder(config)),
    recv_pkt_(0), recv_size_(0), global_hdlr_id_(0), running_(false) {
  this->handlers_.resize(this->dec_->event_size());
}
Kernel::~Kernel() {
}

void Kernel::thread_main() {
  Packet* pkt;
  Payload pd;
  Property prop;
  
  this->running_ = true;
  
  prop.set_decoder(this->dec_);
  
  while (nullptr != (pkt = this->pkt_channel_->pull())) {
    this->recv_pkt_  += 1;
    this->recv_size_ += pkt->cap_len();

    prop.init(pkt);
    pd.reset(pkt);
    this->dec_->decode(&pd, &prop);

    // Event handler
    size_t ev_size = prop.event_idx();
    for (size_t i = 0; i < ev_size; i++) {
      event_id eid = prop.event(i)->id();
      for (auto entry : this->handlers_[eid]) {
        if (entry != nullptr && entry->is_active()) {
          (entry->callback())(prop);
        }
      }
    }

    // Handle change request(s)
    if (this->msg_channel_->has_msg()) {
      ChangeRequest *req;
      while(this->msg_channel_->has_msg()) {
        req = this->msg_channel_->pull();
        req->change(this);
        delete req;
      }
    }
  }

  this->running_ = false;
}


HandlerPtr Kernel::on(const std::string& event_name, Callback&& cb) {
  event_id eid = this->dec_->lookup_event_id(event_name);

  if (eid == Event::NONE) {
    throw Exception::RunTimeError("no such event: " + event_name);
  }

  hdlr_id hid = ++(this->global_hdlr_id_);
  HandlerPtr entry(new HandlerEntity(hid, cb, eid));

  if (this->running_) {
    auto *req = new AddHandler(entry);
    this->msg_channel_->push(req);
  } else {
    this->add_handler(entry);
  }
  return entry;
}

bool Kernel::clear(hdlr_id hid) {
  auto it = this->handler_map_.find(hid);
  if (it == this->handler_map_.end()) {
    return false;  // not found
  }

  auto entry = it->second;
  this->handler_map_.erase(it);
  event_id eid = entry->ev_id();
  auto& handler_set = this->handlers_[eid];
  for (size_t i = 0; i < handler_set.size(); i++) {
    if (handler_set[i] != nullptr &&
        handler_set[i]->id() == entry->id()) {
      handler_set[i] = nullptr;
      break;
    }
  }

  return true;
}

bool Kernel::clear(HandlerPtr ptr) {
  if (this->running_) {
    auto req = new DeleteHandler(ptr);
    this->msg_channel_->push(req);
    return true;
  } else {
    return this->delete_handler(ptr);
  }
}


bool Kernel::add_handler(HandlerPtr ptr) {
  this->handler_map_.insert(std::make_pair(ptr->id(), ptr));
  this->handlers_[ptr->ev_id()].push_back(ptr);
  return true;
}

bool Kernel::delete_handler(HandlerPtr ptr) {
  event_id eid = ptr->ev_id();
  auto& arr = this->handlers_[eid];
  std::vector<HandlerPtr>::iterator tgt = std::find(arr.begin(), arr.end(), ptr);
  if (tgt == arr.end()) {
    return false;
  }

  (*tgt)->destroy();
  arr.erase(tgt);

  return true;
}


}   // namespace pm
