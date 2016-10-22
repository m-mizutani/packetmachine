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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../module.hpp"
#include "../utils/lru.hpp"
#include "../utils/buffer.hpp"

namespace pm {

class TCPSession : public Module {
 private:
  const ParamDef* p_data_;

  static const u_int8_t FIN  = 0x01;
  static const u_int8_t SYN  = 0x02;
  static const u_int8_t RST  = 0x04;
  static const u_int8_t ACK  = 0x10;

  class Session {
   private:
    enum Status {
      SYN_SENT,
      SYNACK_SENT,
      ESTABLISHED,
      CLOSING,
      CLOSED,
    } status_;
    byte_t* key_;
    size_t keylen_;

    /* State Transition

      -- Client -------------- Server --
      [CLOSING]               [CLOSING]
          |       ---(SYN)--->    |      => SYN_SENT
      [SYN_SENT]              [LISTEN]
          |       <-(SYN|ACK)-    |      => SYNACK_SENT
      [SYN_SENT]              [SYN_RECV]
          |       ---(ACK)-->     |      => ESTABLISHED
      [ESTABLISH]             [SYN_RECV]
          |    <--(ACK or Data)-- |
    */

    class Client {
     private:
      uint32_t base_seq_;
      uint32_t next_ack_;
      byte_t *addr_;
      size_t addr_len_;
      uint16_t port_;

     public:
      Client(const byte_t* addr, size_t addr_len, uint16_t port) :
          base_seq_(0), next_ack_(0), addr_len_(addr_len), port_(port) {
        this->addr_ = static_cast<byte_t*>(::malloc(this->addr_len_));
        ::memcpy(this->addr_, addr, this->addr_len_);
      }
      ~Client() {
        free(this->addr_);
      }
      bool match(const byte_t* addr, size_t addr_len, uint16_t port) {
        return (this->addr_len_ == addr_len && this->port_ == port &&
                ::memcmp(addr, this->addr_, addr_len) == 0);
      }
      bool recv(uint8_t flags, uint32_t seq, uint32_t ack, size_t data_len) {
        return false;
      }
    };

   public:
    explicit Session(const Property& p) {
      // TODO(m-mizutani): create session key and store
    }
    ~Session() {
    }

    static void make_key(const Property &p, Buffer* key) {
      size_t src_len, dst_len;
      const byte_t* src_addr = p.src_addr(&src_len);
      const byte_t* dst_addr = p.dst_addr(&dst_len);
      const uint16_t src_port = p.src_port();
      const uint16_t dst_port = p.dst_port();

      assert(src_len == dst_len);
      const size_t keylen = src_len + dst_len +
                            sizeof(src_port) + sizeof(dst_port);
      key->clear();
      key->resize(keylen);

      int rc = ::memcmp(src_addr, dst_addr, src_len);
      if (rc > 0 || (rc == 0 && src_port > dst_port)) {
        key->append(src_addr, src_len);
        key->append(&src_port, sizeof(src_port));
        key->append(dst_addr, dst_len);
        key->append(&dst_port, sizeof(dst_port));
      } else {
        key->append(dst_addr, dst_len);
        key->append(&dst_port, sizeof(dst_port));
        key->append(src_addr, src_len);
        key->append(&src_port, sizeof(src_port));
      }
    }
  };

  static const time_t TIMEOUT = 300;
  param_id tcp_flags_, tcp_segment_, tcp_seq_, tcp_ack_;
  byte_t* keybuf_;
  static const size_t keybuf_len_ = 64;
  LruHash<Session*> ssn_table_;

 public:
  TCPSession() : ssn_table_(3600, 0xffff) {
    this->keybuf_ = static_cast<byte_t*>(::malloc(TCPSession::keybuf_len_));
  }
  ~TCPSession() {
    ::free(this->keybuf_);
  }

  void setup() {
    this->tcp_flags_   = this->lookup_param_id("TCP.flags");
    this->tcp_segment_ = this->lookup_param_id("TCP.segment");
    this->tcp_seq_     = this->lookup_param_id("TCP.seq");
    this->tcp_ack_     = this->lookup_param_id("TCP.ack");
  }


  mod_id decode(Payload* pd, Property* prop) {
    Buffer key;
    uint8_t flags = prop->value(this->tcp_flags_).uint();
    uint32_t seq = prop->value(this->tcp_seq_).uint();
    uint32_t ack = prop->value(this->tcp_ack_).uint();

    Session::make_key(&key);


    return Module::NONE;
  }
};

INIT_MODULE(TCPSession);

}   // namespace pm
