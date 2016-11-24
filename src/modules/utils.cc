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
#include "./utils.hpp"

namespace pm {

struct ns_header {
  u_int16_t trans_id_;  // Transaction ID
  u_int16_t flags_;     // Flags
  u_int16_t qd_count_;  // Query Count
  u_int16_t an_count_;  // Answer Count
  u_int16_t ns_count_;  // Authory Count
  u_int16_t ar_count_;  // Additional Record Count
} __attribute__((packed));

struct ns_rr_header {
  u_int16_t type_;    // Resource type
  u_int16_t class_;   // Class (basically 0x0001)
} __attribute__((packed));

struct ns_ans_header {
  u_int32_t ttl_;     // Cache duration of resouce record
  u_int16_t rd_len_;  // Resource data length
} __attribute__((packed));


NameService::NameService(const std::string& base_name)
    : base_name_(base_name) {
  this->p_tx_id_ = this->define_param("tx_id");

  std::vector<std::string> types = {
    "question",
    "answer",
    "authority",
    "additional",
  };

  for (size_t i = 0; i < types.size(); i++) {
    this->p_record_[i] = this->define_param(types[i]);
  }

  this->ev_query_ = this->define_event("query");
  this->ev_reply_ = this->define_event("reply");
}

NameService::~NameService() {
}

void NameService::setup() {
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
  int rr_total =
      rr_count[RR_QD] + rr_count[RR_AN] + rr_count[RR_NS] + rr_count[RR_AR];

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

  if ((hdr->flags_ & NS_FLAG_MASK_QUERY) > 0) {
    prop->push_event(this->ev_query_);
  } else {
    prop->push_event(this->ev_reply_);
  }


  // parsing resource record
  int target = 0, rr_c = 0;
  for (int c = 0; c < rr_total; c++) {
    while (rr_c >= rr_count[target]) {
      rr_c = 0;
      target++;
      assert(target < RR_CNT);
    }
    rr_c++;

    int remain = ep - ptr;
    // assert (ep - ptr > 0);
    if (ep <= ptr) {
      return false;
    }

    // TODO(m-mizutani): retain map-type value and store data
    /*
    VarNameServiceName * vn =
        dynamic_cast <VarNameServiceName*> (p->retain (this->NS_NAME[target]));
    assert (vn != NULL);
    vn->set_data (ptr, remain, base_ptr, total_len);
    */

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

    // set value
    // TODO(m-mizutani): retain map-type value and store data
    /*
    p->set (this->NS_TYPE[target], &(rr_hdr->type_),
            sizeof (rr_hdr->type_));
    */

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

      // seek pointer
      ptr += rd_len;
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

      const u_int16_t * h = reinterpret_cast<const u_int16_t *>(p);
      u_int16_t jmp = (ntohs(*h) & 0x3FFF);

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

  // if exiting loop,
  debug(DEBUG, "too long domain name (invalid)");
  return NULL;
}


}   // namespace pm
