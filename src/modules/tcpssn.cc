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
  const ParamDef* p_id_;;

  static const u_int8_t FIN  = 0x01;
  static const u_int8_t SYN  = 0x02;
  static const u_int8_t RST  = 0x04;
  static const u_int8_t ACK  = 0x10;

  class Session {
   public:
    enum Status {
      UNKNOWN,
      SYN_SENT,
      SYNACK_SENT,
      ESTABLISHED,
      CLOSING,
      CLOSED,
    };

   private:
    uint64_t id_;
    Status status_;

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

    class Peer {
     private:
      uint32_t base_seq_;
      uint32_t next_ack_;
      byte_t *addr_;
      size_t addr_len_;
      uint16_t port_;

     public:
      Peer(const byte_t* addr, size_t addr_len, uint16_t port) :
          base_seq_(0), next_ack_(0), addr_len_(addr_len), port_(port) {
        this->addr_ = static_cast<byte_t*>(::malloc(this->addr_len_));
        ::memcpy(this->addr_, addr, this->addr_len_);
      }
      ~Peer() {
        free(this->addr_);
      }
      bool is_src(const Property& p) {
        size_t src_len;
        const byte_t* src_addr = p.src_addr(&src_len);
        const uint16_t src_port = p.src_port();
        return this->match(src_addr, src_len, src_port);
      }

      inline bool match(const byte_t* addr, size_t addr_len, uint16_t port) {
        return (this->addr_len_ == addr_len && this->port_ == port &&
                ::memcmp(addr, this->addr_, addr_len) == 0);
      }
      bool send(uint8_t flags, uint32_t seq, uint32_t ack, size_t data_len) {
        // TODO(m-mizutani): implement
        return true;
      }
      bool recv(uint8_t flags, uint32_t seq, uint32_t ack, size_t data_len) {
        // TODO(m-mizutani): implement
        return true;
      }
    } *client_, *server_, *closing_;

   public:
    explicit Session(const Property& p, uint64_t ssn_id) :
        id_(ssn_id), status_(UNKNOWN), closing_(nullptr) {
      size_t src_len, dst_len;
      const byte_t* src_addr = p.src_addr(&src_len);
      const byte_t* dst_addr = p.dst_addr(&dst_len);
      const uint16_t src_port = p.src_port();
      const uint16_t dst_port = p.dst_port();

      this->client_ = new Peer(src_addr, src_len, src_port);
      this->server_ = new Peer(dst_addr, dst_len, dst_port);
    }
    ~Session() {
      delete this->client_;
      delete this->server_;
    }

    uint64_t id() const { return this->id_; }
    Status status() const { return this->status_; }

    void decode(Property* p, uint8_t flags, uint32_t seq, uint32_t ack,
                size_t seg_len, const void* seg_ptr) {
      Peer *sender, *recver;
      if (this->client_->is_src(*p)) {
        sender = this->client_;
        recver = this->server_;
      } else {
        sender = this->server_;
        recver = this->client_;
      }

      if (!sender->send(flags, seq, ack, seg_len) ||
          !recver->recv(flags, seq, ack, seg_len)) {
        return;  // Invalid sequence
      }

      static const bool DBG = true;

      switch (this->status_) {
        case UNKNOWN:
          if (flags == SYN && sender == this->client_) {
            debug(DBG, "%p: SYN", this);
            this->status_ = SYN_SENT;
          }
          break;

        case SYN_SENT:
          if (flags == (SYN|ACK) && sender == this->server_) {
            debug(DBG, "%p: SYN-ACK", this);
            this->status_ = SYNACK_SENT;
          }
          break;

        case SYNACK_SENT:
          if (flags == ACK && sender == this->client_) {
            debug(DBG, "%p: ACK, ESTABLISHED", this);
            this->status_ = ESTABLISHED;
          }
          break;

        case ESTABLISHED:
          if ((flags & FIN) > 0) {
            debug(DBG, "%p: FIN", this);
            this->status_ = CLOSING;
            this->closing_ = sender;
          }
          break;

        case CLOSING:
          if ((flags & FIN) > 0 && this->closing_ != sender) {
            debug(DBG, "%p: CLOSED", this);
            this->status_ = CLOSED;
          }
          break;

        case CLOSED:
          break;  // pass
      }
    }

    static void make_key(const Property& p, Buffer* key) {
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
  uint64_t ssn_count_;

 public:
  TCPSession() : ssn_table_(3600, 0xffff), ssn_count_(0) {
    this->p_id_ = this->define_param("id");

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
    static Buffer key;   // memory can be reused, but not thread safe

    uint8_t flags = prop->value(this->tcp_flags_).uint();
    flags &= (SYN|ACK|FIN|RST);
    uint32_t seq = prop->value(this->tcp_seq_).uint();
    uint32_t ack = prop->value(this->tcp_ack_).uint();
    size_t seg_len;
    const void* seg_ptr = prop->value(this->tcp_segment_).raw(&seg_len);

    Session::make_key(*prop, &key);
    auto node = this->ssn_table_.get(key);

    Session* ssn = nullptr;
    if (node.is_null()) {
      if (flags == SYN) {
        this->ssn_count_ += 1;
        ssn = new Session(*prop, this->ssn_count_);
        this->ssn_table_.put(300, key, ssn);
        debug(false, "new session: %p", ssn);
      } else {
        debug(false, "new session, but not syn packet");
      }
    } else {
      ssn = node.data();
      debug(false, "existing session: %p", ssn);
    }

    if (ssn) {
      const uint64_t ssn_id = ssn->id();
      prop->retain_value(this->p_id_)->cpy(&ssn_id, sizeof(ssn_id));
      ssn->decode(prop, flags, seq, ack, seg_len, seg_ptr);
    }

    return Module::NONE;
  }
};

INIT_MODULE(TCPSession);

}   // namespace pm

