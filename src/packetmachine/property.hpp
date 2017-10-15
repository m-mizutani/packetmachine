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

#ifndef __PACKETMACHINE_PROPERTY_HPP
#define __PACKETMACHINE_PROPERTY_HPP

#include <stdio.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "./common.hpp"

namespace tb {
class Buffer;
}

namespace pm {

class Packet;
class Decoder;
class Object;
class Value;
class ParamDef;
class EventDef;

class Payload {
 private:
  const Packet* pkt_;
  const byte_t *ptr_;
  const byte_t *end_;
  size_t length_;

 public:
  Payload();
  ~Payload();

  void reset(const Packet* pkt);

  // check remain size and move current pointer.
  const byte_t* retain(const size_t size);
  // just return current pointer.
  const byte_t* ptr() const { return this->ptr_; }
  // adjust remain size because of excluding footer (e.g. Ethernet)
  bool shrink(size_t length);
  // just return remain size.
  size_t length() const { return this->length_; }
  // End Of Payload.
  bool eop() const { return this->length_ == 0; }
};


class ParamKey {
 private:
  param_id id_;
  ParamDef *def_;
 public:
  explicit ParamKey(ParamDef* def = nullptr);
  ~ParamKey();
  inline param_id id() const { return this->id_; }
  void set_key(param_id pid);

  ParamDef* def() const { return this->def_; }
  bool operator==(const ParamKey& tgt) const {
    return (this->id_ == tgt.id_);
  }
  bool operator!=(const ParamKey& tgt) const {
    return !((*this) == tgt);
  }
  
};

class Property {
 private:
  std::weak_ptr<Decoder> dec_;
  std::vector<size_t> param_idx_;
  std::vector< std::vector<Value*>* > param_;
  size_t event_idx_;
  std::vector<const EventDef*> event_;

  const Packet* pkt_;
  static const Value null_;

  tb::Buffer* src_addr_;
  tb::Buffer* dst_addr_;
  uint16_t src_port_;
  uint16_t dst_port_;

 public:
  Property();
  ~Property();

  // const values
  static const ParamKey NULL_KEY;
  
  void set_decoder(std::shared_ptr<Decoder> dec);
  void init(const Packet* pkt);

  // Retain data
  Value* retain_value(const ParamDef* def);

  // Push event
  void push_event(const EventDef* def);
  size_t event_idx() const { return this->event_idx_; }
  const EventDef* event(size_t idx) const;

  // Set general parameters
  void set_src_addr(const void* addr, size_t len);
  void set_dst_addr(const void* addr, size_t len);
  void set_src_port(uint16_t port);
  void set_dst_port(uint16_t port);

  // ------------------------------------
  // const methods

  // packet size
  size_t pkt_size() const;

  // timestamp
  time_t ts() const;
  double ts_d() const;
  const struct timeval& tv() const;

  // values
  bool has_value(const ParamKey& key) const;
  bool has_value(const std::string& name) const;
  const Value& value(const ParamKey& key) const;
  const Value& value(const std::string& name) const;

  const Value& operator[](const ParamKey& key) const {
    return this->value(key);
  }
  const Value& operator[](const std::string& name) const {
    return this->value(name);
  }

  // General purpose attributes.
  const byte_t* src_addr(size_t* len) const;
  const byte_t* dst_addr(size_t* len) const;
  uint16_t src_port() const;
  uint16_t dst_port() const;
};

typedef std::function<void(const Property&)> Callback;

}    // namespace pm

#endif    // __PACKETMACHINE_PROPERTY_HPP
