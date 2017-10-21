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

#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP 0x0806
#endif
#ifndef ETHERTYPE_VLAN
#define ETHERTYPE_VLAN 0x8100
#endif
#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP 0x0800
#endif
#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6 0x86dd
#endif
#ifndef ETHERTYPE_LOOPBACK
#define ETHERTYPE_LOOPBACK 0x9000
#endif
#ifndef ETHERTYPE_WLCCP /* Cisco Wireless LAN Context Control Protocol */
#define ETHERTYPE_WLCCP 0x872d
#endif
#ifndef ETHERTYPE_802_1Q
#define ETHERTYPE_802_1Q 0x8100
#endif
#ifndef ETHERTYPE_PPPOE_DISC
#define ETHERTYPE_PPPOE_DISC 0x8863
#endif
#ifndef ETHERTYPE_PPPOE_SSN
#define ETHERTYPE_PPPOE_SSN 0x8864
#endif
#ifndef ETHERTYPE_NETWARE /* Netware IPX/SPX */
#define ETHERTYPE_NETWARE 0x8137
#endif

class Ethernet : public Module {
 private:
  static const size_t ETHER_ADDR_LEN = 6;

  struct ether_header {
    uint8_t dst_[ETHER_ADDR_LEN];
    uint8_t src_[ETHER_ADDR_LEN];
    uint16_t type_;
  } __attribute__((packed));

  const ParamDef *p_type_, *p_src_, *p_dst_;
  mod_id mod_ipv4_, mod_arp_, mod_pppoe_, mod_dot1q_;

 public:
  Ethernet() {
    this->p_type_ = this->define_param("type");
    this->p_src_  = this->define_param("src");
    this->p_dst_  = this->define_param("dst");
  }

  ~Ethernet() {
  }

  void setup(const Config& config) {
    this->mod_ipv4_ = this->lookup_module("IPv4");
    this->mod_arp_  = this->lookup_module("ARP");
    this->mod_pppoe_ = this->lookup_module("PPPoE");
    this->mod_dot1q_ = this->lookup_module("Dot1Q");
  }

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct ether_header*>
               (pd->retain(sizeof(struct ether_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    prop->retain_value(this->p_type_)->set(&hdr->type_, sizeof(hdr->type_));
    prop->retain_value(this->p_src_)->set(&hdr->src_, sizeof(hdr->src_));
    prop->retain_value(this->p_dst_)->set(&hdr->dst_, sizeof(hdr->dst_));

    mod_id next = Module::NONE;
    switch (ntohs(hdr->type_)) {
      case ETHERTYPE_ARP: next = this->mod_arp_; break;
      case ETHERTYPE_IP:  next = this->mod_ipv4_; break;
      case ETHERTYPE_802_1Q: next = this->mod_dot1q_; break;
      case ETHERTYPE_PPPOE_SSN: next = this->mod_pppoe_; break;
    }

    return next;
  }
};

INIT_MODULE(Ethernet);

}   // namespace pm
