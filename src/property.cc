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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "./packetmachine/property.hpp"
#include "./packet.hpp"
#include "./decoder.hpp"
#include "./utils/buffer.hpp"

namespace pm {

Payload::Payload() : pkt_(nullptr) {
}

Payload::~Payload() {
}


void Payload::reset(const Packet* pkt) {
  this->pkt_ = pkt;

  this->length_ = pkt->len();
  this->ptr_ = pkt->buf();
  this->end_ = this->ptr_ + this->length_;
}


const byte_t* Payload::retain(size_t size) {
  assert(this->pkt_);    // must be initialized.

  const byte_t* n = this->ptr_ + size;
  const byte_t* p = this->ptr_;

  if (size <= this->length_ && n <= this->end_) {
    this->length_ -= size;
    this->ptr_ = n;
    return p;
  } else {
    debug(false, "not enough legnth, %zd of %zu", size, this->length_);
    return nullptr;
  }
}


bool Payload::shrink(size_t length) {
  if (length > this->length_) {
    return false;
  } else {
    this->length_ = length;
    return true;
  }
}


const Value Property::null_;

Property::Property(Decoder* dec) : dec_(dec) {
  size_t psize = this->dec_->param_size();
  for (size_t i = 0; i < psize; i++) {
    this->param_.push_back(new std::vector<Value*>());
    this->param_idx_.push_back(0);
  }

  this->src_addr_ = new Buffer();
  this->dst_addr_ = new Buffer();
}

Property::~Property() {
  for (size_t i = 0; i < this->param_.size(); i++) {
    for (auto obj : *(this->param_[i])) {
      delete obj;
    }
    delete this->param_[i];
  }

  delete this->src_addr_;
  delete this->dst_addr_;
}

void Property::init(const Packet *pkt) {
  this->pkt_ = pkt;
  for (size_t i = 0; i < this->param_idx_.size(); i++) {
    this->param_idx_[i] = 0;
  }
  this->event_idx_ = 0;
}


Object* Property::retain_object(const ParamDef* def) {
  assert(0);
}


Value* Property::retain_value(const ParamDef* def) {
  // Value* val = dynamic_cast<Value*>(this->retain_object(def));
  const param_id pid = def->id();
  Value* obj;

  if (this->param_idx_[pid] < this->param_[pid]->size()) {
    obj = (*this->param_[pid])[this->param_idx_[pid]];
    obj->clear();
  } else {
    obj = dynamic_cast<Value*>(def->new_object());
    this->param_[pid]->push_back(obj);
  }

  this->param_idx_[pid]++;
  return obj;
}

void Property::push_event(const EventDef* def) {
  if (this->event_idx_ + 1 > this->event_.size()) {
    this->event_.push_back(def);
  } else {
    this->event_[this->event_idx_] = def;
  }
  this->event_idx_++;
}

const EventDef* Property::event(size_t idx) const {
  assert(this->event_idx_ <= this->event_.size());
  if (idx < this->event_idx_) {
    return this->event_[idx];
  } else {
    return nullptr;
  }
}

void Property::set_src_addr(const void* addr, size_t len) {
  this->src_addr_->clear();
  this->src_addr_->set(addr, len);
}
void Property::set_dst_addr(const void* addr, size_t len) {
  this->dst_addr_->clear();
  this->dst_addr_->set(addr, len);
}
void Property::set_src_port(uint16_t port) {
  this->src_port_ = port;
}
void Property::set_dst_port(uint16_t port) {
  this->dst_port_ = port;
}


size_t Property::pkt_size() const {
  return this->pkt_->len();
}

time_t Property::ts() const {
  return this->pkt_->tv().tv_sec;
}

double Property::ts_d() const {
  return static_cast<double>(this->pkt_->tv().tv_sec) +
      (static_cast<double>(this->pkt_->tv().tv_usec) / 1000000);
}

const struct timeval& Property::tv() const {
  return this->pkt_->tv();
}

bool Property::has_value(param_id pid) const {
  return (this->param_idx_[pid] > 0);
}

bool Property::has_value(const std::string& name) const {
  param_id pid = this->dec_->lookup_param_id(name);
  if (pid == Param::NONE) {
    return false;
  } else {
    return this->has_value(pid);
  }
}

const Value& Property::value(param_id pid) const {
  assert(0 <= pid && pid < static_cast<param_id>(this->param_.size()));
  if (this->param_idx_[pid] > 0) {
    Value* val = dynamic_cast<Value*>((*this->param_[pid])[0]);
    if (val) {
      return *val;
    } else {
      return Property::null_;
    }
  } else {
    return Property::null_;
  }
}

const Value& Property::value(const std::string& name) const {
  param_id pid = this->dec_->lookup_param_id(name);
  if (pid == Param::NONE) {
    return Property::null_;
  } else {
    return this->value(pid);
  }
}

const byte_t* Property::src_addr(size_t* len) const {
  *len = this->src_addr_->len();
  return static_cast<const byte_t*>(this->src_addr_->ptr());
}
const byte_t* Property::dst_addr(size_t* len) const {
  *len = this->dst_addr_->len();
  return static_cast<const byte_t*>(this->dst_addr_->ptr());
}
uint16_t Property::src_port() const {
  return this->src_port_;
}
uint16_t Property::dst_port() const {
  return this->dst_port_;
}

}   // namespace pm
