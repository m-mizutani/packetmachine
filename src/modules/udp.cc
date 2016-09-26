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

class Udp : public Module {
 private:
  struct udp_header {
    u_int16_t src_port_;  // source port
    u_int16_t dst_port_;  // destination port
    u_int16_t length_;    // length
    u_int16_t chksum_;    // checksum
  } __attribute__((packed));

  const ParamDef* p_src_port_;
  const ParamDef* p_dst_port_;
  const ParamDef* p_length_;
  const ParamDef* p_chksum_;

  mod_id mod_dns_;

 public:
  Udp() {
    this->p_src_port_ = this->define_param("src_port");
    this->p_dst_port_ = this->define_param("dst_port");
    this->p_length_   = this->define_param("length");
    this->p_chksum_   = this->define_param("chksum");
  }

  void setup() {
    this->mod_dns_  = this->lookup_module("Dns");
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct udp_header*>
               (pd->retain(sizeof(struct udp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    SET_PROP(this->p_src_port_, hdr->src_port_);
    SET_PROP(this->p_dst_port_, hdr->dst_port_);
    SET_PROP(this->p_length_,   hdr->length_);
    SET_PROP(this->p_chksum_,   hdr->chksum_);

    mod_id next = Module::NONE;
    return next;
  }
};

INIT_MODULE(Udp);

}   // namespace pm
