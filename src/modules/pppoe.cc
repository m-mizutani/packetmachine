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

#include <arpa/inet.h>
#include "../module.hpp"

namespace pm {

class PPPoE : public Module {
 private:
  struct pppoe_header {
    uint8_t version_:4;
    uint8_t type_:4;
    uint8_t code_;
    uint16_t session_id_;
    uint16_t payload_length_;
  } __attribute__((packed));

  const ParamDef* p_version_;
  const ParamDef* p_type_;
  const ParamDef* p_code_;
  const ParamDef* p_session_id_;
  const ParamDef* p_payload_length_;
  mod_id mod_ipv4_;

 public:
  PPPoE() {
    this->p_version_         = this->define_param("version");
    this->p_type_            = this->define_param("type");
    this->p_code_            = this->define_param("code");
    this->p_session_id_      = this->define_param("session_id");
    this->p_payload_length_  = this->define_param("payload_length");
  }

  ~PPPoE() {
  }

  void setup(const Config& config) {
    this->mod_ipv4_ = this->lookup_module("IPv4");
  }

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct pppoe_header*>
               (pd->retain(sizeof(struct pppoe_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

#define SET_VAL(DST, SRC)                             \
    prop->retain_value(DST)->set(&(SRC), sizeof(SRC))
#define CPY_VAL(DST, SRC)                             \
    prop->retain_value(DST)->cpy(&(SRC), sizeof(SRC))

    uint8_t version = hdr->version_;
    uint8_t type    = hdr->type_;
    CPY_VAL(this->p_version_,        version);
    CPY_VAL(this->p_type_,           type);
    SET_VAL(this->p_code_,           hdr->code_);
    SET_VAL(this->p_session_id_,     hdr->session_id_);
    SET_VAL(this->p_payload_length_, hdr->payload_length_);

    mod_id next = Module::NONE;
    auto ppp = reinterpret_cast<const uint16_t*>(pd->retain(2));
    if (ppp == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    if (ntohs(*ppp) == 0x21) {   // IPv4
      next = this->mod_ipv4_;
    }

    return next;
  }
};

INIT_MODULE(PPPoE);

}   // namespace pm
