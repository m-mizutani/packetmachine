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

#ifndef __PACKETMACHINE_SRC_MODULES_UTILS_HPP
#define __PACKETMACHINE_SRC_MODULES_UTILS_HPP

#include <string>
#include "../module.hpp"

namespace pm {

class NameService : public Module {
 private:
  const ParamDef* p_tx_id_;
  const ParamDef* p_record_[4];
  const EventDef* ev_query_;
  const EventDef* ev_reply_;
  const std::string base_name_;

  static const u_int16_t NS_FLAG_MASK_QUERY = 0x8000;
  static const u_int16_t RR_QD  = 0;
  static const u_int16_t RR_AN  = 1;
  static const u_int16_t RR_NS  = 2;
  static const u_int16_t RR_AR  = 3;
  static const u_int16_t RR_CNT = 4;

 public:
  explicit NameService(const std::string& base_name);
  virtual ~NameService();
  void setup();
  mod_id decode(Payload* pd, Property* prop);
  bool ns_decode(Payload* pd, Property* prop);
  static const byte_t * parse_label(const byte_t * p, size_t remain,
                                    const byte_t * sp,
                                    const size_t total_len,
                                    std::string * s);
};

}   // namespace pm


#endif   // __PACKETMACHINE_SRC_MODULES_UTILS_HPP
