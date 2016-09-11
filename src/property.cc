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



Property::Property(Decoder* dec) : dec_(dec) {
}

Property::~Property() {
}

Object* Property::object(param_id pid) {
  Object* obj = this->dec_->new_param(this->context_, pid);
  return obj;
}

Value* Property::value(param_id pid) {
}

}   // namespace pm
