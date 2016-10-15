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

class IPv4 : public Module {
 private:
  static const u_int8_t PROTO_ICMP  = 1;
  static const u_int8_t PROTO_TCP   = 6;
  static const u_int8_t PROTO_UDP   = 17;
  static const u_int8_t PROTO_IPV6  = 41;
  static const u_int8_t PROTO_ICMP6 = 58;

  struct ipv4_header {
    // little endian mode
    u_int8_t  hdrlen_:4;
    u_int8_t  ver_:4;
    u_int8_t  tos_;
    u_int16_t total_len_;  /* total length */
    u_int16_t id_;
    u_int16_t offset_;     /* fragment offset */
    u_int8_t  ttl_;        /* Time To Live */
    u_int8_t  proto_;      /* L4 Protocol */
    u_int16_t chksum_;     /* ip header check sum */
    u_int32_t src_;        /* source ip address */
    u_int32_t dst_;        /* destination ip address */
  } __attribute__((packed));

  const ParamDef* p_hdr_len_;
  const ParamDef* p_ver_;
  const ParamDef* p_tos_;
  const ParamDef* p_total_len_;
  const ParamDef* p_id_;
  const ParamDef* p_offset_;
  const ParamDef* p_ttl_;
  const ParamDef* p_proto_;
  const ParamDef* p_chksum_;
  const ParamDef* p_src_;
  const ParamDef* p_dst_;

  mod_id mod_tcp_, mod_udp_, mod_icmp_;

 public:
  IPv4() {
    this->p_hdr_len_   = this->define_param("hdr_len");
    this->p_ver_       = this->define_param("ver");
    this->p_tos_       = this->define_param("tos");
    this->p_total_len_ = this->define_param("total_len");
    this->p_id_        = this->define_param("id");
    this->p_offset_    = this->define_param("offset");
    this->p_ttl_       = this->define_param("ttl");
    this->p_proto_     = this->define_param("proto");
    this->p_chksum_    = this->define_param("chksum");
    this->p_src_       = this->define_param("src");
    this->p_dst_       = this->define_param("dst");
  }

  void setup() {
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
    uint8_t version = hdr->ver_;
    prop->retain_value(this->p_hdr_len_)->cpy(&hdrlen, sizeof(hdrlen));
    prop->retain_value(this->p_ver_)->cpy(&version, sizeof(version));

    SET_PROP(this->p_tos_,       hdr->tos_);
    SET_PROP(this->p_total_len_, hdr->total_len_);
    SET_PROP(this->p_id_,        hdr->id_);
    SET_PROP(this->p_offset_,    hdr->offset_);
    SET_PROP(this->p_ttl_,       hdr->ttl_);
    SET_PROP(this->p_proto_,     hdr->proto_);
    SET_PROP(this->p_chksum_,    hdr->chksum_);
    SET_PROP(this->p_src_,       hdr->src_);
    SET_PROP(this->p_dst_,       hdr->dst_);

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
