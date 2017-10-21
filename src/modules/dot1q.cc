/*
 * Copyright (c) 2017 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
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

class Dot1Q : public Module {
 private:
  static const uint16_t ETHERTYPE_ARP       = 0x0806;
  static const uint16_t ETHERTYPE_IP        = 0x0800;
  static const uint16_t ETHERTYPE_PPPOE_SSN = 0x8864;
  
  struct dot1q_header {
    uint16_t info_;
    uint16_t type_;
  } __attribute__((packed));

  const ParamDef *p_vlan_id_, *p_type_;
  mod_id mod_ipv4_, mod_arp_, mod_pppoe_;

 public:
  Dot1Q() {
    this->p_type_    = this->define_param("type");
    this->p_vlan_id_ = this->define_param("vlan_id");
  }

  void setup(const Config& config) {
    this->mod_ipv4_  = this->lookup_module("IPv4");
    this->mod_arp_   = this->lookup_module("ARP");
    this->mod_pppoe_ = this->lookup_module("PPPoE");
  }

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct dot1q_header*>
               (pd->retain(sizeof(struct dot1q_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    uint16_t vlan_id = ntohs(hdr->info_) & 0x7ff;
    prop->retain_value(this->p_type_)->set(&hdr->type_, sizeof(hdr->type_));
    prop->retain_value(this->p_vlan_id_)->cpy(&vlan_id, sizeof(vlan_id),
                                              Value::LITTLE);

    mod_id next = Module::NONE;
    switch (ntohs(hdr->type_)) {
      case ETHERTYPE_ARP: next = this->mod_arp_; break;
      case ETHERTYPE_IP:  next = this->mod_ipv4_; break;
      case ETHERTYPE_PPPOE_SSN: next = this->mod_pppoe_; break;
    }

    return next;
  }
};

INIT_MODULE(Dot1Q);

}   // namespace pm
