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

class ICMP : public Module {
 private:
  struct icmp_header {
    uint8_t type_;  // ICMP type
    uint8_t code_;  // ICMP code
    uint16_t chksum_;    // checksum
  } __attribute__((packed));

  const ParamDef* p_type_;
  const ParamDef* p_code_;
  const ParamDef* p_chksum_;

  class IcmpType : public Value {
   public:
    void repr(std::ostream &os) const {
      switch (this->uint()) {
        case  0: os << "Echo Reply"; break;
        case  3: os << "Destination Unreachable"; break;
        case  4: os << "Source Quench"; break;
        case  5: os << "Redirect"; break;
        case  8: os << "Echo Request"; break;
        case  9: os << "Router Advertisement"; break;
        case 10: os << "Router Solicitation"; break;
        case 11: os << "Time Exceeded"; break;
        case 12: os << "Parameter"; break;
        case 13: os << "Timestamp"; break;
        case 14: os << "Timestamp Reply"; break;
        case 15: os << "Information Request"; break;
        case 16: os << "Information Reply"; break;
        case 17: os << "Address Mask Request"; break;
        case 18: os << "Address Mask Reply"; break;
      }
    }
    static Value* new_value() { return new IcmpType(); }
  };

  class IcmpCode : public Value {
   private:
    uint8_t type_;

   public:
    void set_type(uint8_t type) {
      this->type_ = type;
    }
    void repr(std::ostream &os) const {
      uint64_t code = this->uint();

      if (this->type_ == 3) {   // Detination Unreachable
        switch (code) {
          case 0: os << "Net Unreachable"; break;
          case 1: os << "Host Unreachable"; break;
          case 2: os << "Protocol Unreachable"; break;
          case 3: os << "Port Unreachable"; break;
          case 4: os << "Fragmentation Needed and DF was Set"; break;
          case 5: os << "Source Route Failed"; break;
          case 6: os << "Destination Network Unknown"; break;
          case 7: os << "Destination Host Unknown"; break;
          case 8: os << "Source Host Isolated"; break;
          case 9: os << "Communication with Destination "
              "Network is Administratively Prohibited"; break;
          case 10: os << "Communication with Destinaltion "
              "Host is Administratively Prohibited"; break;
          case 11: os << "Destination Network Unreachable for ToS"; break;
          case 12: os << "Destination Host Unreachable for ToS"; break;
          case 13: os << "Communication Administratively Prohibited"; break;
          case 14: os << "Host Precedence Violation"; break;
          case 15: os << "Precedence cutoff in effect"; break;
        }
      } else if (this->type_ == 5) {   // Redirect
        switch (code) {
          case 0: os << "Redirect Datagram for the Network (or subnet)"; break;
          case 1: os << "Redirect Datagram for the Host"; break;
          case 2: os << "Redirect Datagram for the ToS and Network"; break;
          case 3: os << "Redirect Datagram for the ToS and Host"; break;
        }
      } else if (this->type_ == 9) {   // Router Advertisement
        switch (code) {
          case 0: os << "Normal router advertisement"; break;
          case 16: os << "Does not route common traffic"; break;
        }
      } else if (this->type_ == 11) {   // Time Exceeded
        switch (code) {
          case 0: os << "Time to Live exceeded in Transit"; break;
          case 1: os << "Fragment Reassembly Time Exceeded"; break;
        }
      } else if (this->type_ == 12) {   // Parameter Problem
        switch (code) {
          case 0: os << "Pointer indicates the error"; break;
          case 1: os << "Missing a Required Option"; break;
          case 2: os << "Bad Length"; break;
        }
      } else {
        os << code;
      }
    }
    static Value* new_value() { return new IcmpCode(); }
  };

 public:
  ICMP() {
    this->p_type_   = this->define_param("type", IcmpType::new_value);
    this->p_code_   = this->define_param("code", IcmpCode::new_value);
    this->p_chksum_ = this->define_param("chksum");
  }

  void setup(const Config& config) {
  }

#define SET_PROP(PARAM, DATA) \
  prop->retain_value(PARAM)->set(&(DATA), sizeof(DATA));

  mod_id decode(Payload* pd, Property* prop) {
    auto hdr = reinterpret_cast<const struct icmp_header*>
               (pd->retain(sizeof(struct icmp_header)));
    if (hdr == nullptr) {   // Not enough packet size.
      return Module::NONE;
    }

    SET_PROP(this->p_type_, hdr->type_);

    IcmpCode* code = dynamic_cast<IcmpCode*>
                     (prop->retain_value(this->p_code_));
    code->set(&hdr->code_, sizeof(hdr->code_));
    code->set_type(hdr->type_);

    SET_PROP(this->p_chksum_, hdr->chksum_);

    return Module::NONE;
  }
};

INIT_MODULE(ICMP);

}   // namespace pm
