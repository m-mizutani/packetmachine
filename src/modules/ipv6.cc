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
#include <arpa/inet.h>
#include "../module.hpp"

namespace pm {

class IPv6 : public Module {
 private:
  static const uint8_t PROTO_ICMP  = 1;
  static const uint8_t PROTO_TCP   = 6;
  static const uint8_t PROTO_UDP   = 17;
  static const uint8_t PROTO_IPV6  = 41;
  static const uint8_t PROTO_ICMP6 = 58;

  struct ipv6_header {
    uint32_t flags_;      // version, traffic class, flow label
    uint16_t data_len_;   // dat length
    uint8_t  next_hdr_;   // next header
    uint8_t  hop_limit_;  // hop limit
    uint32_t src_[4];     // source address
    uint32_t dst_[4];     // dest address
  } __attribute__((packed));

  struct ipv6_option {
    uint8_t next_hdr_;
    uint8_t hdr_len_;
  } __attribute__((packed));
  
  MajorParamDef *p_hdr_;
  
  // const ParamDef* p_src_;
  // const ParamDef* p_dst_;
  const ParamDef* p_opt_;
  const ParamDef* p_data_;
  mod_id mod_tcp_, mod_udp_;
  // mod_id mod_icmp6_;

 public:
  IPv6() {

#define DEFINE_HDR(NAME)                                                \
    this->p_hdr_->define_minor(                                         \
        #NAME, [](pm::Value* v, const pm::byte_t* ptr) {                \
          auto hdr = reinterpret_cast<const struct ipv6_header*>(ptr);  \
          v->set(&(hdr->NAME ## _), sizeof(hdr-> NAME ## _));           \
        });

    // Header parameters
    this->p_hdr_ = this->define_major_param("hdr");
    
    DEFINE_HDR(flags);
    DEFINE_HDR(data_len);
    DEFINE_HDR(next_hdr);
    DEFINE_HDR(hop_limit);

    this->p_hdr_->define_minor(
        "version", [](pm::Value* v, const pm::byte_t* ptr) {
          auto hdr = reinterpret_cast<const struct ipv6_header*>(ptr);
          uint32_t ver = (ntohl(hdr->flags_) & 0xf0000000) >> 28;
          v->cpy(&ver, sizeof(ver));
        });

    this->p_hdr_->define_minor(
        "traffic_class", [](pm::Value* v, const pm::byte_t* ptr) {
          auto hdr = reinterpret_cast<const struct ipv6_header*>(ptr);
          uint32_t tfc = (ntohl(hdr->flags_) & 0x0ff00000) >> 20;
          v->cpy(&tfc, sizeof(tfc));
        });

    this->p_hdr_->define_minor(
        "flow_label", [](pm::Value* v, const pm::byte_t* ptr) {
          auto hdr = reinterpret_cast<const struct ipv6_header*>(ptr);
          uint32_t flow_label = ntohl(hdr->flags_) & 0xfffff;
          v->cpy(&flow_label, sizeof(flow_label));
        });
    
    // IP addresses
    /*
    this->p_src_
        = this->define_param("src", value::IPv6Addr::new_value);
    this->p_dst_
        = this->define_param("dst", value::IPv6Addr::new_value);
    */
    
    // Option and data 
    this->p_opt_       = this->define_param("opt");
    this->p_data_      = this->define_param("data");
  }

  void setup(const Config& config) {
    this->mod_tcp_  = this->lookup_module("TCP");
    this->mod_udp_  = this->lookup_module("UDP");
  }

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct ipv6_header*>
               (pd->retain(sizeof(struct ipv6_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    // Set values
    prop->set_src_addr(hdr->src_, sizeof(hdr->src_));
    prop->set_dst_addr(hdr->dst_, sizeof(hdr->dst_));
    /*
    prop->retain(this->p_src_)->set(hdr->src_, sizeof(hdr->src_));
    prop->retain(this->p_dst_)->set(hdr->dst_, sizeof(hdr->dst_));
    */

    mod_id next = Module::NONE;

    switch (hdr->next_hdr_) {
      case PROTO_TCP:  next = this->mod_tcp_; break;
      case PROTO_UDP:  next = this->mod_udp_; break;
    }

    return next;
  }
};

INIT_MODULE(IPv6);

}   // namespace pm
