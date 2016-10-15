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

#include "../module.hpp"

namespace pm {

class ICMP : public Module {
 private:
  struct icmp_header {
    u_int8_t type_;  // ICMP type
    u_int8_t code_;  // ICMP code
    u_int16_t chksum_;    // checksum
  } __attribute__((packed));

  const ParamDef* p_type_;
  const ParamDef* p_code_;
  const ParamDef* p_chksum_;

 public:
  ICMP() {
    this->p_type_   = this->define_param("type");
    this->p_code_   = this->define_param("code");
    this->p_chksum_ = this->define_param("chksum");
  }

  void setup() {
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct icmp_header*>
               (pd->retain(sizeof(struct icmp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    SET_PROP(this->p_type_, hdr->type_);
    SET_PROP(this->p_code_, hdr->code_);
    SET_PROP(this->p_chksum_, hdr->chksum_);

    return Module::NONE;
  }
};

INIT_MODULE(ICMP);

}   // namespace pm
