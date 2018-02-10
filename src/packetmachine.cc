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

#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include "./packetmachine.hpp"
#include "./capture.hpp"
#include "./packet.hpp"
#include "./channel.hpp"
#include "./kernel.hpp"
#include "./thread.hpp"

#include "./debug.hpp"

namespace pm {

class Input : public Thread {
 private:
  Capture* cap_;
  PktChannel channel_;

 public:
  Input(Capture* cap, PktChannel channel) :
      cap_(cap), channel_(channel) {
  }
  ~Input() {
  }

  void thread_main() {
    Packet *pkt;
    Capture::Result rc;

    for (;;) {
      pkt = this->channel_->retain();

      while (Capture::NONE == (rc = this->cap_->read(pkt))) {
        // timeout read packet data.
        usleep(1);
      }

      if (rc == Capture::OK) {
        this->channel_->push(pkt);
      } else {
        this->channel_->close();

        if (rc == Capture::ERROR) {
          throw Exception::RunTimeError(this->cap_->error());
        }
        break;
      }
    }
  }
};



Handler::Handler(std::shared_ptr<HandlerEntity> ptr,
                 std::shared_ptr<Kernel> kernel) :
    ptr_(ptr), kernel_(kernel) {  
}
    
Handler::~Handler() {
}

bool Handler::is_active() const {
  if (auto p = this->ptr_.lock()) {
    return p->is_active();
  } else {
    return false;
  }
}

bool Handler::activate() {
  if (auto p = this->ptr_.lock()) {
    return p->activate();
  } else {
    return false;
  }
}

bool Handler::deactivate() {
  if (auto p = this->ptr_.lock()) {
    return p->deactivate();
  } else {
    return false;
  }
}

bool Handler::destroy() {
  if (auto p = this->ptr_.lock()) {
    if (auto k = this->kernel_.lock()) {
      return k->clear(p);
    }
  }

  return false;
}


Machine::Machine() :
    cap_(nullptr), input_(nullptr) {
  Config config;
  this->kernel_ = std::shared_ptr<Kernel>(new Kernel(config));
}

Machine::Machine(const Config& config) :
    cap_(nullptr), input_(nullptr) {
  this->kernel_ = std::shared_ptr<Kernel>(new Kernel(config));
}

Machine::~Machine() {
  delete this->cap_;
  delete this->input_;
}

void Machine::add_pcapdev(const std::string &dev_name) {
  if (this->cap_) {
    throw Exception::ConfigError("data source has been configured");
  }

  Capture *cap = new PcapDev(dev_name);
  if (!cap->ready()) {
    const std::string msg = cap->error();
    delete cap;
    throw Exception::ConfigError(msg);
  }

  this->cap_ = cap;
}

void Machine::add_pcapfile(const std::string &file_path) {
  if (this->cap_) {
    throw Exception::ConfigError("data source has been configured");
  }

  Capture *cap = new PcapFile(file_path);
  if (!cap->ready()) {
    const std::string msg = cap->error();
    delete cap;
    throw Exception::ConfigError(msg);
  }

  this->cap_ = cap;
}

const std::string& Machine::data_source_name() const {
  static const std::string none("");
  
  if (this->cap_) {
    return this->cap_->src_name();
  } else {
    return none;
  }
}

void Machine::loop() {
  this->start();
  this->join();
}


void Machine::start() {
  if (!this->cap_) {
    throw Exception::ConfigError("no input source is available");
  }

  this->kernel_->start();
  this->input_ = new Input(this->cap_, this->kernel_->pkt_channel());
  this->input_->start();
}

bool Machine::join(struct timespec* timeout) {
  if (timeout) {
    this->input_->join(*timeout);
  } else {
    this->input_->join();
  }
  
  this->kernel_->join();
  return true;
}



void Machine::halt() {
  this->input_->stop();
  this->kernel_->stop();
}


Handler Machine::on(const std::string& event_name,
                 std::function<void(const Property&)>&& callback) {
  assert(this->kernel_);
  HandlerPtr ptr = this->kernel_->on(event_name, std::move(callback));
  Handler hdlr(ptr, this->kernel_);
  return hdlr;
}

uint64_t Machine::recv_pkt() const {
  assert(this->kernel_);
  return this->kernel_->recv_pkt();
}

uint64_t Machine::recv_size() const {
  assert(this->kernel_);
  return this->kernel_->recv_size();
}


const ParamKey& Machine::lookup_param_key(const std::string& name) const {
  return this->kernel_->dec().lookup_param_key(name);
}

const std::string& Machine::lookup_param_name(const ParamKey& key) const {
  return this->kernel_->dec().lookup_param_name(key);
}


event_id Machine::lookup_event_id(const std::string& name) const {
  assert(this->kernel_);
  return this->kernel_->dec().lookup_event_id(name);
}
const std::string& Machine::lookup_event_name(event_id eid) const {
  assert(this->kernel_);
  return this->kernel_->dec().lookup_event_name(eid);
}

}   // namespace pm
