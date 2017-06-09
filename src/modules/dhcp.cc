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

class DHCP : public Module {
 private:
  struct dhcp_header {
    uint8_t msg_type_;
    uint8_t hw_type_;
    uint8_t hw_addr_len_;
    uint8_t hops_;
    uint32_t trans_id_;
    uint16_t seconds_;
    uint16_t flags_;
    uint32_t client_addr_;
    uint32_t your_client_addr_;
    uint32_t next_server_addr_;
    uint32_t relay_agent_addr_;
    uint8_t client_hw_addr_[16];
    uint8_t server_host_name_[64];
    uint8_t boot_file_name_[128];
    uint8_t magic_cookie_[4];    
  } __attribute__((packed));

  struct dhcp_opt {
    uint8_t length_;
    uint8_t opt_;
    uint8_t data_[128];
  } __attribute__((packed));

  const ParamDef *p_msg_type_;

 public:
  DHCP() {
    this->p_msg_type_ = this->define_param("msg_type");

    /*
    this->p_src_port_ = this->define_param("src_port",
                                        value::PortNumber::new_value);
    this->p_dst_port_ = this->define_param("dst_port",
                                        value::PortNumber::new_value);
    this->p_length_   = this->define_param("length");
    this->p_chksum_   = this->define_param("chksum");
    */
  }

  void setup() {
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct dhcp_header*>
               (pd->retain(sizeof(struct dhcp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    SET_PROP(this->p_msg_type_, hdr->msg_type_);

    mod_id next = Module::NONE;
    return next;
  }
};

INIT_MODULE(DHCP);

}   // namespace pm
