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
#include "./utils.hpp"
#include "./../debug.hpp"

namespace pm {

struct ns_header {
  uint16_t trans_id_;  // Transaction ID
  uint16_t flags_;     // Flags
  uint16_t qd_count_;  // Query Count
  uint16_t an_count_;  // Answer Count
  uint16_t ns_count_;  // Authory Count
  uint16_t ar_count_;  // Additional Record Count
} __attribute__((packed));

struct ns_rr_header {
  uint16_t type_;    // Resource type
  uint16_t class_;   // Class (basically 0x0001)
} __attribute__((packed));

struct ns_ans_header {
  uint32_t ttl_;     // Cache duration of resouce record
  uint16_t rd_len_;  // Resource data length
} __attribute__((packed));


NameService::NameService(const std::string& base_name)
    : base_name_(base_name) {
  this->p_tx_id_ = this->define_param("tx_id");
  this->p_is_query_ = this->define_param("is_query");
  this->p_records_ = this->define_param("records", NSRecord::new_value);
  this->p_name_ = this->define_param("name", NSName::new_value);
  this->p_data_ = this->define_param("data", NSData::new_value);
  this->p_type_ = this->define_param("type", NSType::new_value);

  std::vector<std::string> types = {
    "question",
    "answer",
    "authority",
    "additional",
  };


  for (size_t i = 0; i < types.size(); i++) {
    this->p_record_[i] = this->define_param(types[i], value::Array::new_value);
  }

  this->ev_query_ = this->define_event("query");
  this->ev_reply_ = this->define_event("reply");
}

NameService::~NameService() {
}

void NameService::setup(const Config& config) {
}

mod_id NameService::decode(Payload* pd, Property* prop) {
  this->ns_decode(pd, prop);
  return Module::NONE;
}

bool NameService::ns_decode(Payload* pd, Property* prop) {
  static const bool DEBUG = false;

  const size_t hdr_len = sizeof(struct ns_header);
  const byte_t *base_ptr = pd->retain(hdr_len);

  if (base_ptr == NULL) {
    return false;
  }

  const struct ns_header * hdr =
      reinterpret_cast<const struct ns_header*> (base_ptr);

  int rr_count[4], rr_delim[4];
  rr_count[RR_QD] = ntohs(hdr->qd_count_);
  rr_count[RR_AN] = ntohs(hdr->an_count_);
  rr_count[RR_NS] = ntohs(hdr->ns_count_);
  rr_count[RR_AR] = ntohs(hdr->ar_count_);
  int rr_total = rr_count[RR_QD] + rr_count[RR_AN] +
                 rr_count[RR_NS] + rr_count[RR_AR];
  for (int i = 0; i < 4; i++) {
    rr_delim[i] = (i == 0 ? 0 : (rr_delim[i - 1] + rr_count[i - 1]));
  }

  debug(DEBUG, "trans_id:0x%04X, flags:%04X, qd=%d, an=%d, ns=%d, ar=%d",
        hdr->trans_id_, hdr->flags_, rr_count[RR_QD], rr_count[RR_AN],
        rr_count[RR_NS], rr_count[RR_AR]);

  const size_t total_len = pd->length();
  const byte_t *ptr = pd->retain(total_len);
  assert(ptr != NULL);
  const byte_t * ep = base_ptr + hdr_len + total_len;

  prop->retain_value(this->p_tx_id_)->set(&(hdr->trans_id_),
                                        sizeof(hdr->trans_id_));
  /*
  printf("flag: %04X\n", hdr->flags_);
  printf("ntoh: %04X\n", ntohs(hdr->flags_));
  printf("mask: %04X\n", NS_FLAG_MASK_QUERY);
  */
  int is_q = 0;
  if ((ntohs(hdr->flags_) & NS_FLAG_MASK_QUERY) == 0) {
    is_q = 1;
    prop->push_event(this->ev_query_);
  } else {
    is_q = 0;
    prop->push_event(this->ev_reply_);
  }
  prop->retain_value(this->p_is_query_)->cpy(&(is_q), sizeof(is_q),
                                             pm::Value::LITTLE);

  value::Array* arr = nullptr;
  // parsing resource record
  int target = 0, rr_c = 0;
  for (int c = 0; c < rr_total; c++) {
    while (rr_c >= rr_count[target]) {
      rr_c = 0;
      target++;
      assert(target < RR_CNT);
      arr = nullptr;
    }
    rr_c++;

    auto remain = ep - ptr;
    // assert (ep - ptr > 0);
    if (ep <= ptr) {
      return false;
    }

    if (arr == nullptr) {
      arr = dynamic_cast<value::Array*>
        (prop->retain_value(this->p_record_[target]));
    }

    NSRecord *rec =
      dynamic_cast<NSRecord*>(prop->retain_value(this->p_records_));
    NSName* v_name = dynamic_cast<NSName*>(prop->retain_value(this->p_name_));
    Value* v_type = prop->retain_value(this->p_type_);

    rec->set_name(v_name);
    rec->set_type(v_type);
    assert(arr != nullptr);
    arr->push(rec);

    v_name->set_param(ptr, remain, base_ptr, total_len);

    if (NULL == (ptr = NameService::parse_label (ptr, remain, base_ptr,
                                                 total_len, NULL))) {
      debug(DEBUG, "label parse error");
      break;
    }

    // assert (ep - ptr);
    if (ep <= ptr) {
      return false;
    }

    if (ep - ptr < static_cast<int>(sizeof (struct ns_rr_header))) {
      debug(DEBUG, "not enough length: %ld", ep - ptr);
      break;
    }
    const struct ns_rr_header * rr_hdr =
        reinterpret_cast<const struct ns_rr_header*>(ptr);
    ptr += sizeof(struct ns_rr_header);
    v_type->set(&(rr_hdr->type_), sizeof (rr_hdr->type_));


    // has resource data field
    if (c >= rr_count[RR_QD]) {
      if (ep - ptr < static_cast<int>(sizeof (struct ns_ans_header))) {
        debug(DEBUG, "not enough length: %ld", ep - ptr);
        break;
      }
      const struct ns_ans_header * ans_hdr =
          reinterpret_cast<const struct ns_ans_header*> (ptr);
      ptr += sizeof (struct ns_ans_header);
      const size_t rd_len = ntohs(ans_hdr->rd_len_);

      if (ep - ptr < static_cast<int>(rd_len)) {
        debug(DEBUG, "not match resource record len(%zd) and remain (%zd)",
               rd_len, ep - ptr);
        break;
      }

      // set value
      // TODO(m-mizutani): retain map-type value and store data
      /*
      VarNameServiceData * v = dynamic_cast <VarNameServiceData*>
                               (p->retain (this->NS_DATA[target]));
      assert (v != NULL);
      v->set_data (ptr, rd_len, htons (rr_hdr->type_), base_ptr, total_len);
      */
      NSData* v_data = dynamic_cast<NSData*>(prop->retain_value(this->p_data_));
      rec->set_data(v_data);
      v_data->set_param(ptr, rd_len, htons(rr_hdr->type_), base_ptr,
                        total_len);

      // seek pointer
      ptr += rd_len;
    } else {
    }
  }

  if (ep != ptr) {
    debug(DEBUG, "fail to parse (remain:%ld)", ep - ptr);
  }

  return true;
}


const byte_t * NameService::parse_label(const byte_t * p, size_t remain,
                                        const byte_t * sp,
                                        const size_t total_len,
                                        std::string * s) {
  const size_t min_len = 1;
  const size_t dst_len = 2;
  const size_t max_len = 256;
  size_t len = 0;

  bool DEBUG = false;
  if (s) {
    s->erase();
  }

  const byte_t * rp = NULL;

  while (len < max_len) {
    if (remain < min_len) {
      debug(DEBUG, "not enough length: %zd", remain);
      return NULL;
    }

    // jump if needed
    if ((*p & 0xC0) == 0xC0) {
      if (remain < dst_len) {
        debug(DEBUG, "not enough jump destination length: %zd", remain);
        return NULL;
      }

      const uint16_t * h = reinterpret_cast<const uint16_t *>(p);
      uint16_t jmp = (ntohs(*h) & 0x3FFF);

      if (jmp >= total_len) {
        debug(DEBUG, "invalid jump point: %d", jmp);
        return NULL;
      }
      if (rp == NULL) {
        rp = p + dst_len;
      }
      p = const_cast<byte_t*>(&(sp[jmp]));
      remain = total_len - (jmp);
    }

    // retain payload
    int data_len = *p;
    if (data_len == 0) {
      return (rp == NULL ? p + 1 : rp);
    }
    if (data_len + min_len >= remain) {
      debug(DEBUG, "invalid data length: %d (remain:%zd)",
            data_len, remain);
      return NULL;
    }

    if (s) {
      s->append(reinterpret_cast<const char*>(p + 1), data_len);
      s->append(".", 1);
    }
    len += data_len;

    p += data_len + 1;
    remain -= data_len + 1;
  }

  // if exiting loop
  debug(DEBUG, "too long domain name (invalid)");
  return NULL;
}

NSRecord::NSRecord() {
  this->map_.insert(std::make_pair("name", nullptr));
  this->map_.insert(std::make_pair("data", nullptr));
  this->map_.insert(std::make_pair("type", nullptr));
  this->it_type_ = this->map_.find("type");
  this->it_name_ = this->map_.find("name");
  this->it_data_ = this->map_.find("data");
  this->it_data_->second = const_cast<value::NoneValue*>(&(value::NONE));
}

NSRecord::~NSRecord() {
}

void NSRecord::set_type(Value* val) {
  this->it_type_->second = val;
}
void NSRecord::set_name(NSName* name) {
  this->it_name_->second = name;
}
void NSRecord::set_data(NSData* data) {
  this->it_data_->second = data;
}

void NSRecord::clear() {
  this->it_data_->second = const_cast<value::NoneValue*>(&(value::NONE));
}

void NSName::set_param(const byte_t* ptr, size_t len,
                       const byte_t* base_ptr, size_t total_len) {
  this->set(ptr, len);
  this->base_ptr_ = base_ptr;
  this->total_len_ = total_len;
}


void NSName::repr(std::ostream &os) const {
  size_t len;
  const byte_t * ptr = this->raw(&len);
  std::string s;
  const byte_t* rp = NameService::parse_label(ptr, len, this->base_ptr_,
                                              this->total_len_, &s);
  os << ((rp != NULL) ? s : "?");
}

void NSData::set_param(const byte_t * ptr, size_t len, uint16_t type,
                       const byte_t * base_ptr, size_t total_len) {
  this->set(ptr, len);
  this->type_ = type;
  this->base_ptr_ = base_ptr;
  this->total_len_ = total_len;
}


void NSData::repr(std::ostream &os) const {
  switch (this->type_) {
    case  1: os << this->ip4(); break;  // A
    case 28: os << this->ip6(); break;  // AAAA
    case  2:  // NS
    case  5:  // CNAME
    case  6:  // SOA
    case 12:  // PTR
    case 15:  // MX
    case 33:  // SRV
      {
        std::string s;
        size_t len;
        const byte_t* ptr = this->raw(&len);
        const byte_t* rp = NameService::parse_label(ptr, len, this->base_ptr_,
                                                    this->total_len_, &s);
        if (rp == NULL) {
          s.clear();
        }

        os << s;
      }
      break;

    case 16:  // TXT
      {
        size_t len, d_len;
        const byte_t* start_ptr = this->raw(&len);
        debug(0, "len = %zd", len);
        for (const byte_t *p = start_ptr; p - start_ptr < len; p += d_len) {
          if (p > start_ptr) {
            os << ",";
          }
          d_len = *p;
          p += 1;
          debug(0, "d_len = %zu, p - s (%ld)", d_len, p - start_ptr);

          if (p - start_ptr + d_len > len) {
            break;
          }
          os << std::string(reinterpret_cast<const char *>(p), d_len);
        }
      }
      break;

    default:
      {
        size_t len;
        const byte_t *p, *start_ptr = this->raw(&len);
        debug(0, "unsupported name service type: %d", this->type_);
        for (p = start_ptr; p - start_ptr < len; p++) {
          char c = static_cast<char>(*p);
          os << (isprint(c) ? c : '.');
        }
      }
      break;
  }
}

void NSType::repr(std::ostream &os) const {
  auto type = this->uint();

  switch (type) {
    case  1: os << "A"; break;
    case  2: os << "NS"; break;
    case  5: os << "CNAME"; break;
    case  6: os << "SOA"; break;
    case 12: os << "PTR"; break;
    case 15: os << "MX"; break;
    case 16: os << "TXT"; break;
    case 28: os << "AAAA"; break;
    case 33: os << "SRV"; break;
    default: os << type; break;
  }
}

}   // namespace pm
