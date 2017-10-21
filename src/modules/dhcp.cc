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
#include "../debug.hpp"

namespace pm {


class DHCP : public Module {
 public:
  class Option : public value::Map {
   private:
    std::map<std::string, Value*>::iterator it_type_;
    std::map<std::string, Value*>::iterator it_length_;
    std::map<std::string, Value*>::iterator it_data_;

   public:
    Option() {
      this->map_.insert(std::make_pair("type",   nullptr));
      this->map_.insert(std::make_pair("length", nullptr));
      this->map_.insert(std::make_pair("data",   nullptr));
      this->it_type_   = this->map_.find("type");
      this->it_length_ = this->map_.find("length");
      this->it_data_   = this->map_.find("data");
    }
    ~Option() = default;

    void set_type(Value* v)   { this->it_type_->second   = v; }
    void set_length(Value* v) { this->it_length_->second = v; }
    void set_data(Value* v)   { this->it_data_->second   = v; }
    void clear() {
      this->it_type_->second   = nullptr;
      this->it_length_->second = nullptr;
      this->it_data_->second   = nullptr;
    }
    static Value* new_value() { return new Option(); }
  };

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

  struct dhcp_opt_hdr {
    uint8_t opt_;
    uint8_t length_;
  } __attribute__((packed));

  const ParamDef* p_msg_type_;
  const ParamDef* p_hw_type_;
  const ParamDef* p_hw_addr_len_;
  const ParamDef* p_hops_;
  const ParamDef* p_trans_id_;
  const ParamDef* p_seconds_;
  const ParamDef* p_flags_;
  const ParamDef* p_client_addr_;
  const ParamDef* p_your_client_addr_;
  const ParamDef* p_next_server_addr_;
  const ParamDef* p_relay_agent_addr_;
  const ParamDef* p_client_hw_addr_;
  const ParamDef* p_server_host_name_;
  const ParamDef* p_boot_file_name_;
  const ParamDef* p_magic_cookie_;
  const ParamDef* p_options_;

  const ParamDef* p_option_;
  const ParamDef* p_opt_type_;
  const ParamDef* p_opt_length_;
  const ParamDef* p_opt_data_;

 public:
  DHCP() {
    this->p_msg_type_         = this->define_param("msg_type");
    this->p_hw_type_          = this->define_param("hw_type");
    this->p_hw_addr_len_      = this->define_param("hw_addr_len");
    this->p_hops_             = this->define_param("hops");
    this->p_trans_id_         = this->define_param("trans_id");
    this->p_seconds_          = this->define_param("seconds");
    this->p_flags_            = this->define_param("flags");
    this->p_client_addr_      = this->define_param("client_addr");
    this->p_your_client_addr_ = this->define_param("your_client_addr");
    this->p_next_server_addr_ = this->define_param("next_server_addr");
    this->p_relay_agent_addr_ = this->define_param("relay_agent_addr");
    this->p_client_hw_addr_   = this->define_param("client_hw_addr");
    this->p_server_host_name_ = this->define_param("server_host_name");
    this->p_boot_file_name_   = this->define_param("boot_file_name");
    this->p_magic_cookie_     = this->define_param("magic_cookie");
    this->p_options_          = this->define_param("options",
                                                   value::Array::new_value);

    // Internal values
    this->p_option_     = this->define_param("_option", Option::new_value);
    this->p_opt_type_   = this->define_param("_opt_type");
    this->p_opt_length_ = this->define_param("_opt_length");
    this->p_opt_data_   = this->define_param("_opt_data");
  }

  void setup(const Config& config) {
  }

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct dhcp_header*>
               (pd->retain(sizeof(struct dhcp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

#define SET_PROP(PARAM, DATA)                                   \
    prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

    SET_PROP(this->p_msg_type_,         hdr->msg_type_);
    SET_PROP(this->p_hw_type_,          hdr->hw_type_);
    SET_PROP(this->p_hw_addr_len_,      hdr->hw_addr_len_);
    SET_PROP(this->p_hops_,             hdr->hops_);
    SET_PROP(this->p_trans_id_,         hdr->trans_id_);
    SET_PROP(this->p_flags_,            hdr->flags_);
    SET_PROP(this->p_client_addr_,      hdr->client_addr_);
    SET_PROP(this->p_your_client_addr_, hdr->your_client_addr_);
    SET_PROP(this->p_next_server_addr_, hdr->next_server_addr_);
    SET_PROP(this->p_relay_agent_addr_, hdr->relay_agent_addr_);
    SET_PROP(this->p_magic_cookie_,     hdr->magic_cookie_);

    // Special values
    prop->retain_value(this->p_seconds_)->set(&hdr->seconds_,
                                              sizeof(hdr->seconds_),
                                              Value::LITTLE);
    prop->retain_value(this->p_client_hw_addr_)->set(&(hdr->client_hw_addr_),
                                                     hdr->hw_addr_len_);
    if (hdr->server_host_name_[0] != '\0') {
      SET_PROP(this->p_server_host_name_, hdr->server_host_name_);
    }
    if (hdr->boot_file_name_[0] != '\0') {
      SET_PROP(this->p_boot_file_name_,   hdr->boot_file_name_);
    }

    auto opt_arr = dynamic_cast<value::Array*>
                   (prop->retain_value(this->p_options_));
    for (;;) {
      auto opt_hdr = reinterpret_cast<const struct dhcp_opt_hdr*>
                     (pd->retain(sizeof(struct dhcp_opt_hdr)));
      if (opt_hdr == nullptr || opt_hdr->opt_ == 0xFF) {
        break;  // 0xFF means End of Option
      }

      auto opt_data = pd->retain(opt_hdr->length_);
      if (opt_data == nullptr) {
        break;
      }

      auto option = dynamic_cast<Option*>(prop->retain_value(this->p_option_));
      auto v_type   = prop->retain_value(this->p_opt_type_);
      auto v_length = prop->retain_value(this->p_opt_length_);
      auto v_data   = prop->retain_value(this->p_opt_data_);
      v_type->set(&(opt_hdr->opt_), sizeof(opt_hdr->opt_));
      v_length->set(&(opt_hdr->length_), sizeof(opt_hdr->length_));
      v_data->set(opt_data, opt_hdr->length_);
      option->set_type(v_type);
      option->set_length(v_length);
      option->set_data(v_data);

      opt_arr->push(option);
    }
    return Module::NONE;
  }
};

INIT_MODULE(DHCP);

}   // namespace pm
