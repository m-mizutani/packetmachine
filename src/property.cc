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
#include "./packetmachine/property.hpp"
#include "./packet.hpp"
#include "./decoder.hpp"

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

  if (size <= this->length_ && n < this->end_) {
    this->length_ -= size;
    this->ptr_ = n;
    return p;
  } else {
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
    this->param_.push_back(new std::vector<Object*>());
    this->param_idx_.push_back(0);
  }
}

Property::~Property() {
  for (size_t i = 0; i < this->param_.size(); i++) {
    for (auto obj : *(this->param_[i])) {
      delete obj;
    }
    delete this->param_[i];
  }
}

void Property::init(const Packet* pkt) {
  this->pkt_ = pkt;
  for (size_t i = 0; i < this->param_idx_.size(); i++) {
    this->param_idx_[i] = 0;
  }
}


Object* Property::retain_object(const ParamDef* def) {
  const param_id pid = def->id();
  Object* obj;

  if (this->param_idx_[pid] < this->param_[pid]->size()) {
    obj = (*this->param_[pid])[this->param_idx_[pid]];
  } else {
    obj = def->new_object();
    this->param_[pid]->push_back(obj);
  }

  this->param_idx_[pid]++;
  return obj;
}

Value* Property::retain_value(const ParamDef* def) {
  Value* val = dynamic_cast<Value*>(this->retain_object(def));
  return val;
}

const Value& Property::value(param_id pid) {
  return Property::null_;
}

const Value& Property::value(const std::string& name) {
  param_id pid = this->dec_->lookup_param_id(name);
  if (pid == Param::NONE) {
    return Property::null_;
  } else {
    return this->value(pid);
  }
}

const Object& Property::object(param_id pid) {
  return Property::null_;
}

const Object& Property::object(const std::string& name) {
  return Property::null_;
}

}   // namespace pm
