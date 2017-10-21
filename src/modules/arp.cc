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

    uint16_t hw_type_;
    uint16_t pr_type_;
    uint8_t  hw_size_;
    uint8_t  pr_size_;
    uint16_t op_;
  } __attribute__((packed));

  const ParamDef* p_hw_type_;
  const ParamDef* p_pr_type_;
  const ParamDef* p_hw_size_;
  const ParamDef* p_pr_size_;
  const ParamDef* p_op_;
  const ParamDef* p_hw_src_;
  const ParamDef* p_hw_dst_;
  const ParamDef* p_pr_src_;
  const ParamDef* p_pr_dst_;

  const EventDef* ev_request_;
  const EventDef* ev_reply_;

 public:
  ARP() {
    this->p_hw_type_ = this->define_param("hw_type");
    this->p_pr_type_ = this->define_param("pr_type");
    this->p_hw_size_ = this->define_param("hw_size");
    this->p_pr_size_ = this->define_param("pr_size");
    this->p_op_      = this->define_param("op");
    this->p_hw_src_  = this->define_param("hw_src");
    this->p_hw_dst_  = this->define_param("hw_dst");
    this->p_pr_src_  = this->define_param("pr_src");
    this->p_pr_dst_  = this->define_param("pr_dst");

    this->ev_request_ = this->define_event("request");
    this->ev_reply_   = this->define_event("reply");
  }
  ~ARP() {
  }

  void setup(const Config& config) {
    // pass
  }

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const ArpHeader*>
               (pd->retain(sizeof(ArpHeader)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

#define SET_PROP(PARAM, DATA)                               \
    prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

    SET_PROP(this->p_hw_type_, hdr->hw_type_);
    SET_PROP(this->p_pr_type_, hdr->pr_type_);
    SET_PROP(this->p_hw_size_, hdr->hw_size_);
    SET_PROP(this->p_pr_size_, hdr->pr_size_);
    SET_PROP(this->p_op_,      hdr->op_);

    const size_t hw_len = static_cast<size_t>(hdr->hw_size_);
    const size_t pr_len = static_cast<size_t>(hdr->pr_size_);

    if (ntohs(hdr->op_) == ARPOP_REQUEST) {
      prop->push_event(this->ev_request_);
    } else if (ntohs(hdr->op_) == ARPOP_REPLY) {
      prop->push_event(this->ev_reply_);
    }

#define SET_ADDR(PARAM, LEN)                        \
    {                                               \
      const byte_t* ptr = pd->retain(LEN);          \
      if (ptr) {                                    \
        prop->retain_value(PARAM)->set(ptr, (LEN)); \
      } else {                                      \
        return Module::NONE;                        \
      }                                             \
    }

    SET_ADDR(this->p_hw_src_, hw_len);
    SET_ADDR(this->p_pr_src_, pr_len);
    SET_ADDR(this->p_hw_dst_, hw_len);
    SET_ADDR(this->p_pr_dst_, pr_len);

    return Module::NONE;
  }
};

INIT_MODULE(ARP);

}   // namespace pm
