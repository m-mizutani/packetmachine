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
#include <sys/time.h>

#include "../module.hpp"
#include "../../external/cpp-toolbox/src/cache.hpp"

namespace pm {

class TCPSession : public Module {
 private:
  static const bool DBG = false;
  static const bool DBG_SEQ = false;
  static const bool DBG_STAT = false;
  const ParamDef* p_id_;;
  const EventDef *ev_new_, *ev_estb_;

  static const uint8_t FIN  = 0x01;
  static const uint8_t SYN  = 0x02;
  static const uint8_t RST  = 0x04;
  static const uint8_t ACK  = 0x10;
  static std::string flag2str(uint8_t f) {
    std::string s;
    s.append((f & FIN) > 0 ? "F" : "*");
    s.append((f & SYN) > 0 ? "S" : "*");
    s.append((f & RST) > 0 ? "R" : "*");
    s.append((f & ACK) > 0 ? "A" : "*");
    return s;
  }

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
    struct timeval ts_init_;
    struct timeval ts_estb_;
    struct timeval ts_rtt_;

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
      bool has_base_seq_;
      uint32_t base_seq_;
      uint32_t next_seq_;
      uint32_t win_size_;
      byte_t *addr_;
      size_t addr_len_;
      uint16_t port_;

     public:
      Peer(const byte_t* addr, size_t addr_len, uint16_t port) :
          has_base_seq_(false), base_seq_(0), next_seq_(0),
          addr_len_(addr_len), port_(port) {
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

      void set_base_seq(uint32_t seq, size_t seg_len) {
        this->has_base_seq_ = true;
        this->base_seq_ = seq;
        this->next_seq_ = 1 + seg_len;
      }

      void inc_seq(uint32_t step = 1) {
        this->next_seq_ += step;
      }

      inline bool match(const byte_t* addr, size_t addr_len, uint16_t port) {
        return (this->addr_len_ == addr_len && this->port_ == port &&
                ::memcmp(addr, this->addr_, addr_len) == 0);
      }

      bool send(uint8_t flags, uint32_t seq, uint32_t ack, size_t data_len) {
        if (!this->has_base_seq_) {
          return true;
        }

        auto f = flag2str(flags);
        const uint32_t rel_seq = seq - this->base_seq_;
        debug(DBG_SEQ, "(%p) %s seq: %u, next: %u > %zu", this,
              f.c_str(), rel_seq, this->next_seq_, data_len);

        if (this->next_seq_ == rel_seq) {
          this->next_seq_ += data_len;
        } else {
          debug(DBG_SEQ, "seq/ack mismatched");
        }

        return true;
      }

      void recv(uint32_t ack, uint32_t win_size) {
        this->win_size_ = win_size;
        return;
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

      if (!sender->send(flags, seq, ack, seg_len)) {
        return;  // Invalid sequence
      }
      recver->recv(ack, 0);

      static const bool DBG = true;

      switch (this->status_) {
        case UNKNOWN:
          if (flags == SYN && sender == this->client_) {
            debug(DBG_STAT, "%p: SYN", this);
            this->status_ = SYN_SENT;
            ::memcpy(&this->ts_init_, &(p->tv()), sizeof(this->ts_init_));
            sender->set_base_seq(seq, seg_len);
          }
          break;

        case SYN_SENT:
          if (flags == (SYN|ACK) && sender == this->server_) {
            debug(DBG_STAT, "%p: SYN-ACK", this);
            this->status_ = SYNACK_SENT;
            sender->set_base_seq(seq, seg_len);
          }
          break;

        case SYNACK_SENT:
          if (flags == ACK && sender == this->client_) {
            debug(DBG_STAT, "%p: ACK, ESTABLISHED", this);
            this->status_ = ESTABLISHED;
            ::memcpy(&this->ts_estb_, &(p->tv()), sizeof(this->ts_estb_));
            timersub(&this->ts_estb_, &this->ts_init_, &this->ts_rtt_);
          }
          break;

        case ESTABLISHED:
          if ((flags & FIN) > 0) {
            debug(DBG_STAT, "%p: FIN", this);
            this->status_ = CLOSING;
            this->closing_ = sender;
            sender->inc_seq();
          }
          break;

        case CLOSING:
          if ((flags & FIN) > 0 && this->closing_ != sender) {
            debug(DBG_STAT, "%p: CLOSED", this);
            this->status_ = CLOSED;
            sender->inc_seq();
          }
          break;

        case CLOSED:
        const std::string s = flag2str(flags);
        debug(false, "already closed: %p -> %s", this, s.c_str());
          break;  // pass
      }
    }

    static void make_key(const Property& p, tb::HashKey* key) {
      size_t src_len, dst_len;
      const byte_t* src_addr = p.src_addr(&src_len);
      const byte_t* dst_addr = p.dst_addr(&dst_len);
      const uint16_t src_port = p.src_port();
      const uint16_t dst_port = p.dst_port();
      debug(false, "port: %d -> %d", src_port, dst_port);
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

      key->finalize();
    }
  };

  static const time_t TIMEOUT = 300;
  param_id tcp_flags_, tcp_segment_, tcp_seq_, tcp_ack_;
  byte_t* keybuf_;
  static const size_t keybuf_len_ = 64;
  tb::LruHash<Session*> ssn_table_;
  uint64_t ssn_count_;
  time_t curr_ts_;
  bool init_ts_;



 public:
  TCPSession() : ssn_table_(3600, 0xffff), ssn_count_(0), curr_ts_(0),
                 init_ts_(false) {
    this->p_id_ = this->define_param("id");
    this->ev_new_ = this->define_event("new");
    this->ev_estb_ = this->define_event("established");

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

  /**
   *
   */
  mod_id decode(Payload* pd, Property* prop) {
    tb::HashKey key;

    time_t ts = prop->ts();
    if (this->curr_ts_ < ts) {
      time_t diff = ts - this->curr_ts_;
      this->curr_ts_ = ts;
      if (this->init_ts_) {
        this->ssn_table_.step(diff);
      } else {
        this->init_ts_ = true;
      }
    }

    uint8_t flags = prop->value(this->tcp_flags_).uint();
    flags &= (SYN|ACK|FIN|RST);
    uint32_t seq = static_cast<uint32_t>(prop->value(this->tcp_seq_).uint());
    uint32_t ack = static_cast<uint32_t>(prop->value(this->tcp_ack_).uint());
    size_t seg_len;
    const void* seg_ptr = prop->value(this->tcp_segment_).raw(&seg_len);

    if (seg_ptr == nullptr) {
      seg_len = 0;
    }

    Session::make_key(*prop, &key);
    auto node = this->ssn_table_.get(key);

    Session* ssn = nullptr;
    if (node.is_null()) {
      if (flags == SYN) {
        this->ssn_count_ += 1;
        ssn = new Session(*prop, this->ssn_count_);
        this->ssn_table_.put(300, key, ssn);
        debug(false, "new session: %p", ssn);
        prop->push_event(this->ev_new_);
      } else {
        debug(false, "new session, but not syn packet");
      }
    } else {
      ssn = node.data();
      debug(false, "existing session: %p", ssn);
    }

    if (ssn) {
      debug(DBG, "ssn = %p", ssn);
      const uint64_t ssn_id = ssn->id();
      prop->retain_value(this->p_id_)->cpy(&ssn_id, sizeof(ssn_id));
      ssn->decode(prop, flags, seq, ack, seg_len, seg_ptr);
    }

    return Module::NONE;
  }
};

INIT_MODULE(TCPSession);

}   // namespace pm
