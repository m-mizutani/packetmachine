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

class IPv4 : public Module {
 private:
  static const uint8_t PROTO_ICMP  = 1;
  static const uint8_t PROTO_TCP   = 6;
  static const uint8_t PROTO_UDP   = 17;
  static const uint8_t PROTO_IPV6  = 41;
  static const uint8_t PROTO_ICMP6 = 58;

  struct ipv4_header {
    // little endian mode
    uint8_t  hdrlen_:4;
    uint8_t  ver_:4;
    uint8_t  tos_;
    uint16_t total_len_;  /* total length */
    uint16_t id_;
    uint16_t offset_;     /* fragment offset */
    uint8_t  ttl_;        /* Time To Live */
    uint8_t  proto_;      /* L4 Protocol */
    uint16_t chksum_;     /* ip header check sum */
    uint32_t src_;        /* source ip address */
    uint32_t dst_;        /* destination ip address */
  } __attribute__((packed));

  MajorParamDef *p_hdr_;
  
  const ParamDef* p_src_;
  const ParamDef* p_dst_;
  const ParamDef* p_opt_;
  const ParamDef* p_data_;
  mod_id mod_tcp_, mod_udp_, mod_icmp_;

 public:
  IPv4() {

#define DEFINE_HDR(NAME)                                                \
    this->p_hdr_->define_minor(                                         \
        #NAME, [](pm::Value* v, const pm::byte_t* ptr) {                \
          auto hdr = reinterpret_cast<const struct ipv4_header*>(ptr);  \
          v->set(&(hdr->NAME ## _), sizeof(hdr-> NAME ## _));           \
        });

    // Header parameters
    this->p_hdr_ = this->define_major_param("hdr");
    
    DEFINE_HDR(tos);
    DEFINE_HDR(total_len);
    DEFINE_HDR(id);
    DEFINE_HDR(offset);
    DEFINE_HDR(ttl);
    DEFINE_HDR(proto);
    DEFINE_HDR(chksum);

    this->p_hdr_->define_minor(
        "hdr_len", [](pm::Value* v, const pm::byte_t* ptr) {
          auto hdr = reinterpret_cast<const struct ipv4_header*>(ptr); 
          uint8_t hdrlen  = hdr->hdrlen_ << 2;
          v->cpy(&hdrlen, sizeof(hdrlen));
        });

    this->p_hdr_->define_minor(
        "ver", [](pm::Value* v, const pm::byte_t* ptr) {
          auto hdr = reinterpret_cast<const struct ipv4_header*>(ptr); 
          uint8_t version = hdr->ver_;
          v->cpy(&version, sizeof(version));
        });

    // IP addresses
    this->p_src_
        = this->define_param("src", value::IPv4Addr::new_value);
    this->p_dst_
        = this->define_param("dst", value::IPv4Addr::new_value);

    // Option and data 
    this->p_opt_       = this->define_param("opt");
    this->p_data_      = this->define_param("data");
  }

  void setup(const Config& config) {
    this->mod_tcp_  = this->lookup_module("TCP");
    this->mod_udp_  = this->lookup_module("UDP");
    this->mod_icmp_ = this->lookup_module("ICMP");
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct ipv4_header*>
               (pd->retain(sizeof(struct ipv4_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    uint8_t hdrlen  = hdr->hdrlen_ << 2;
    unsigned int total_len = ntohs(hdr->total_len_);
    unsigned int hdr_len   = hdrlen;

    if (total_len < hdr_len) {
      return Module::NONE;
    }

    prop->retain_value(this->p_hdr_)->set(hdr, sizeof(struct ipv4_header));
    
    assert(total_len >= hdr_len);
    uint16_t data_len = total_len - hdr_len;

    // Set values
    prop->set_src_addr(&hdr->src_, sizeof(hdr->src_));
    prop->set_dst_addr(&hdr->dst_, sizeof(hdr->dst_));
    SET_PROP(this->p_src_,       hdr->src_);
    SET_PROP(this->p_dst_,       hdr->dst_);

    // Set option field
    if (hdrlen > sizeof(struct ipv4_header)) {
      const size_t opt_len = hdrlen - sizeof(struct ipv4_header);
      auto opt = pd->retain(opt_len);
      prop->retain_value(this->p_opt_)->set(opt, opt_len);
    }

    // Adjust payload length
    pd->shrink(data_len);
    prop->retain_value(this->p_data_)->set(pd->ptr(), pd->length());

    mod_id next = Module::NONE;

    switch (hdr->proto_) {
      case PROTO_ICMP: next = this->mod_icmp_; break;
      case PROTO_TCP:  next = this->mod_tcp_; break;
      case PROTO_UDP:  next = this->mod_udp_; break;
    }

    return next;
  }
};

INIT_MODULE(IPv4);

}   // namespace pm
