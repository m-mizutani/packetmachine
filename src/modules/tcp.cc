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

class TCP : public Module {
 private:
  struct tcp_header {
    uint16_t src_port_;  // source port
    uint16_t dst_port_;  // destination port
    uint32_t seq_;       // tcp sequence number
    uint32_t ack_;       // tcp ack number

    // ToDo(Masa): 4 bit data field should be updated for little-endian
    uint8_t offset_;

    uint8_t flags_;      // flags
    uint16_t window_;    // window
    uint16_t chksum_;    // checksum
    uint16_t urgptr_;    // urgent pointer
  } __attribute__((packed));

  const ParamDef* p_src_port_;
  const ParamDef* p_dst_port_;
  const ParamDef* p_seq_;
  const ParamDef* p_ack_;
  const ParamDef* p_offset_;
  const ParamDef* p_flags_;
  const ParamDef* p_window_;
  const ParamDef* p_chksum_;
  const ParamDef* p_urgptr_;

  // Flags
  const ParamDef* p_flag_fin_;
  const ParamDef* p_flag_syn_;
  const ParamDef* p_flag_rst_;
  const ParamDef* p_flag_push_;
  const ParamDef* p_flag_ack_;
  const ParamDef* p_flag_urg_;
  const ParamDef* p_flag_ece_;
  const ParamDef* p_flag_cwr_;

  const ParamDef* p_optdata_;
  const ParamDef* p_segment_;

  static const uint8_t FIN  = 0x01;
  static const uint8_t SYN  = 0x02;
  static const uint8_t RST  = 0x04;
  static const uint8_t PUSH = 0x08;
  static const uint8_t ACK  = 0x10;
  static const uint8_t URG  = 0x20;
  static const uint8_t ECE  = 0x40;
  static const uint8_t CWR  = 0x80;


 public:
  TCP() {
    this->p_src_port_ = this->define_param("src_port",
                                        value::PortNumber::new_value);
    this->p_dst_port_ = this->define_param("dst_port",
                                        value::PortNumber::new_value);

#define DEFINE_PARAM(name) \
    this->p_ ## name ## _ = this->define_param(#name);

    DEFINE_PARAM(seq);
    DEFINE_PARAM(ack);
    DEFINE_PARAM(offset);
    DEFINE_PARAM(flags);
    DEFINE_PARAM(window);
    DEFINE_PARAM(chksum);
    DEFINE_PARAM(urgptr);

    // Flags
    DEFINE_PARAM(flag_fin);
    DEFINE_PARAM(flag_syn);
    DEFINE_PARAM(flag_rst);
    DEFINE_PARAM(flag_push);
    DEFINE_PARAM(flag_ack);
    DEFINE_PARAM(flag_urg);
    DEFINE_PARAM(flag_ece);
    DEFINE_PARAM(flag_cwr);

    // Option
    DEFINE_PARAM(optdata);

    // Segment
    DEFINE_PARAM(segment);
  }

  mod_id mod_tcpssn_;
  void setup() {
    this->mod_tcpssn_ = this->lookup_module("TCPSession");
  }


  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct tcp_header*>
               (pd->retain(sizeof(struct tcp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    prop->set_src_port(hdr->src_port_);
    prop->set_dst_port(hdr->dst_port_);

#define SET_PROP_FROM_HDR(NAME)                                         \
    prop->retain_value(this->p_ ## NAME)->set(&(hdr->NAME), sizeof(hdr->NAME));

    SET_PROP_FROM_HDR(src_port_);
    SET_PROP_FROM_HDR(dst_port_);
    SET_PROP_FROM_HDR(seq_);
    SET_PROP_FROM_HDR(ack_);

    const uint8_t offset = (hdr->offset_ & 0xf0) >> 2;
    prop->retain_value(this->p_offset_)->cpy(&offset, sizeof(offset));

    SET_PROP_FROM_HDR(offset_);
    SET_PROP_FROM_HDR(flags_);
    SET_PROP_FROM_HDR(window_);
    SET_PROP_FROM_HDR(chksum_);
    SET_PROP_FROM_HDR(urgptr_);

#define SET_FLAGS(FNAME, PNAME)                                         \
    {                                                                   \
      byte_t f = ((hdr->flags_ & (FNAME)) > 0);                         \
      prop->retain_value(this->p_flag_ ## PNAME ## _)->cpy(&f, sizeof(f)); \
    }

    SET_FLAGS(FIN, fin);
    SET_FLAGS(SYN, syn);
    SET_FLAGS(RST, rst);
    SET_FLAGS(PUSH, push);
    SET_FLAGS(ACK, ack);
    SET_FLAGS(URG, urg);
    SET_FLAGS(ECE, ece);
    SET_FLAGS(CWR, cwr);

    // Set option data.
    const size_t optlen = offset - sizeof(tcp_header);
    if (optlen > 0) {
      const byte_t* opt = pd->retain(optlen);
      if (opt == nullptr) {
        return Module::NONE;
      }

      prop->retain_value(this->p_optdata_)->set(opt, optlen);
    }

    // Set segment data.
    if (pd->length() > 0) {
      prop->retain_value(this->p_segment_)->set(pd->ptr(), pd->length());
    }

    return this->mod_tcpssn_;
  }
};

INIT_MODULE(TCP);

}   // namespace pm
