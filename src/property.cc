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
#include <sys/time.h>
#include "./packetmachine/property.hpp"
#include "./packet.hpp"
#include "./decoder.hpp"
#include "../external/cpp-toolbox/src/buffer.hpp"
#include "./debug.hpp"

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



ParamKey::ParamKey(ParamDef *def) : id_(Param::NONE), def_(def) {  
}

ParamKey::~ParamKey() {
}

void ParamKey::set_key(param_id pid) {
  this->id_ = pid;
}



const Value Property::null_;
const ParamKey Property::NULL_KEY;


Property::Property() {
  this->src_addr_ = new tb::Buffer();
  this->dst_addr_ = new tb::Buffer();
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

void Property::set_decoder(std::shared_ptr<Decoder> dec) {
  this->dec_ = dec;
}

void Property::init(const Packet *pkt) {
  this->pkt_ = pkt;
  for (size_t i = 0; i < this->param_idx_.size(); i++) {
    this->param_idx_[i] = 0;
  }
  this->event_idx_ = 0;
}


Value* Property::retain_value(const ParamDef* def) {
  const param_id pid = def->id();
  Value* obj;

  size_t psize = static_cast<size_t>(pid) + 1;
  if (psize > this->param_.size()) {
    for (size_t i = this->param_.size(); i < psize; i++) {
      this->param_.push_back(new std::vector<Value*>());
      this->param_idx_.push_back(0);
    }
  }

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

bool Property::has_value(const ParamKey& key) const {
  return (this->param_idx_[key.id()] > 0);
}

bool Property::has_value(const std::string& name) const {
  if (auto dec = this->dec_.lock()) {
    auto& key = dec->lookup_param_key(name);
    if (key == Property::NULL_KEY) {
      return false;
    } else {
      return this->has_value(key);
    }
  } else {
    throw Exception::RunTimeError("Decoder module is not available");
  }
}


const Value& Property::value(const ParamKey& key) const {
  if (this->param_idx_[key.id()] > 0) {
    Value* val = dynamic_cast<Value*>((*this->param_[key.id()])[0]);
    if (val) {
      ParamDef *def = key.def();
      assert(def);
      if (def->is_minor()) {
        auto mdef = dynamic_cast<MinorParamDef*>(def);
        auto sval = dynamic_cast<ValueStorage*>(val);
        assert(mdef && sval);
        if (sval->size() == 0) {
          sval->resize(mdef->parent().minor_size(),
                       mdef->constructor());
        }

        auto& v = val->get(static_cast<size_t>(mdef->minor_id()));
        // TODO: cache a result of defer evaluation
        mdef->defer(const_cast<Value*>(&v), val->raw());
        return v;
      } else {
        return *val;
      }
    } else {
      return Property::null_;
    }
  } else {
    return Property::null_;
  }
}

const Value& Property::value(const std::string& name) const {
  if (auto dec = this->dec_.lock()) {
    auto& key = dec->lookup_param_key(name);
    if (key == Property::NULL_KEY) {
      return Property::null_;
    } else {
      return this->value(key);
    }
  } else {
    throw Exception::RunTimeError("Decoder module is not available");
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
