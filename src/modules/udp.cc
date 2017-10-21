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
#include "../debug.hpp"


namespace pm {

class UDP : public Module {
 private:
  struct udp_header {
    uint16_t src_port_;  // source port
    uint16_t dst_port_;  // destination port
    uint16_t length_;    // length
    uint16_t chksum_;    // checksum
  } __attribute__((packed));

  const ParamDef* p_src_port_;
  const ParamDef* p_dst_port_;
  const ParamDef* p_length_;
  const ParamDef* p_chksum_;
  MajorParamDef* p_hdr_;

  mod_id mod_dns_;
  mod_id mod_mdns_;
  mod_id mod_dhcp_;

 public:
  UDP() {
    this->p_src_port_ = this->define_param("src_port",
                                           value::PortNumber::new_value);
    this->p_dst_port_ = this->define_param("dst_port",
                                           value::PortNumber::new_value);
    this->p_hdr_ = this->define_major_param("hdr");

#define SET_VAL(PARAM) {                                          \
      auto hdr = reinterpret_cast<const struct udp_header*>(ptr); \
      v->set(&(PARAM), sizeof(PARAM));                            \
    }      
    
    this->p_hdr_->define_minor("length", [](pm::Value* v, const pm::byte_t* ptr) {
        SET_VAL(hdr->length_);
      });
    this->p_hdr_->define_minor("chksum", [](pm::Value* v, const pm::byte_t* ptr) {
        SET_VAL(hdr->chksum_);
      });
    this->p_length_   = this->define_param("length");
    this->p_chksum_   = this->define_param("chksum");

  }

  void setup(const Config& config) {
    this->mod_dns_  = this->lookup_module("DNS");
    this->mod_mdns_ = this->lookup_module("MDNS");
    this->mod_dhcp_ = this->lookup_module("DHCP");
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct udp_header*>
               (pd->retain(sizeof(struct udp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    prop->set_src_port(ntohs(hdr->src_port_));
    prop->set_dst_port(ntohs(hdr->dst_port_));
    prop->retain_value(this->p_hdr_)->set(hdr, sizeof(struct udp_header));
    SET_PROP(this->p_src_port_, hdr->src_port_);
    SET_PROP(this->p_dst_port_, hdr->dst_port_);
    SET_PROP(this->p_length_,   hdr->length_);
    SET_PROP(this->p_chksum_,   hdr->chksum_);

    mod_id next = Module::NONE;
    uint16_t sport = ntohs(hdr->src_port_);
    uint16_t dport = ntohs(hdr->dst_port_);

    if (sport == 53 || dport == 53) {
      next = this->mod_dns_;
    } else if (sport == 5353 || dport == 5353) {
      next = this->mod_mdns_;
    } else if (sport == 67 || sport == 68 || dport == 67 || dport == 68) {
      next = this->mod_dhcp_;
    }

    return next;
  }
};

INIT_MODULE(UDP);

}   // namespace pm
