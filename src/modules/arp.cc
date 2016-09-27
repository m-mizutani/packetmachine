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

class ARP : public Module {
 private:
  struct ArpHeader {
#define ARPHRD_ETHER           1  /* ethernet hardware format */
#define ARPHRD_IEEE802         6  /* token-ring hardware format */
#define ARPHRD_FRELAY         15  /* frame relay hardware format */
#define ARPHRD_IEEE1394       24  /* IEEE1394 hardware address */
#define ARPHRD_IEEE1394_EUI64 27  /* IEEE1394 EUI-64 */

#define ARPOP_REQUEST    1      /* request to resolve address */
#define ARPOP_REPLY      2      /* response to previous request */
#define ARPOP_REVREQUEST 3      /* request protocol address given hardware */
#define ARPOP_REVREPLY   4      /* response giving protocol address */
#define ARPOP_INVREQUEST 8      /* request to identify peer */
#define ARPOP_INVREPLY   9      /* response identifying peer */

    u_int16_t hw_addr_fmt_;
    u_int16_t pr_addr_fmt_;
    u_int8_t  hw_addr_len_;
    u_int8_t  pr_addr_len_;
    u_int16_t op_;
  } __attribute__((packed));

  const ParamDef* p_hw_addr_fmt_;
  const ParamDef* p_pr_addr_fmt_;
  const ParamDef* p_hw_addr_len_;
  const ParamDef* p_pr_addr_len_;
  const ParamDef* p_op_;
  const ParamDef* p_hw_src_;
  const ParamDef* p_hw_dst_;
  const ParamDef* p_pr_src_;
  const ParamDef* p_pr_dst_;

 public:
  ARP() {
    this->p_hw_addr_fmt_ = this->define_param("hw_addr_fmt");
    this->p_pr_addr_fmt_ = this->define_param("pr_addr_fmt");
    this->p_hw_addr_len_ = this->define_param("hw_addr_len");
    this->p_pr_addr_len_ = this->define_param("pr_addr_len");
    this->p_op_          = this->define_param("op");
    this->p_hw_src_      = this->define_param("hw_src");
    this->p_hw_dst_      = this->define_param("hw_dst");
    this->p_pr_src_      = this->define_param("pr_src");
    this->p_pr_dst_      = this->define_param("pr_dst");
  }
  ~ARP() {
    delete this->p_hw_addr_fmt_;
    delete this->p_pr_addr_fmt_;
    delete this->p_hw_addr_len_;
    delete this->p_pr_addr_len_;
    delete this->p_op_;
    delete this->p_hw_src_;
    delete this->p_hw_dst_;
    delete this->p_pr_src_;
    delete this->p_pr_dst_;
  }

  void setup() {
    // pass
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const ArpHeader*>
               (pd->retain(sizeof(ArpHeader)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    SET_PROP(this->p_hw_addr_fmt_, hdr->hw_addr_fmt_);
    return Module::NONE;
  }
};

INIT_MODULE(ARP);

}   // namespace pm
