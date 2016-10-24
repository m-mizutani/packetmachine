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
#include "./packetmachine.hpp"
#include "./capture.hpp"
#include "./packet.hpp"
#include "./channel.hpp"
#include "./kernel.hpp"

#include "./debug.hpp"

namespace pm {

class Input {
 private:
  Capture* cap_;
  Channel<Packet>* channel_;

 public:
  Input(Capture* cap, Channel<Packet>* channel) :
      cap_(cap), channel_(channel) {
  }
  ~Input() {
  }

  static void* thread(void* obj) {
    Input* p = static_cast<Input*>(obj);
    p->run();
    return nullptr;
  }

  void run() {
    Packet *pkt;
    int rc = 0;

    for (;;) {
      pkt = this->channel_->retain();

      while (0 == (rc = this->cap_->read(pkt))) {
        // timeout read packet data.
        usleep(1);
      }

      if (rc > 0) {
        this->channel_->push(pkt);
      } else {
        // rc < 0, error occurred
        this->channel_->close();
        break;
      }
    }
  }
};



Machine::Machine() : cap_(nullptr), input_(nullptr), kernel_(nullptr) {
  this->kernel_ = new Kernel();
}

Machine::~Machine() {
  delete this->kernel_;
  delete this->cap_;
}

void Machine::add_device(const std::string &dev_name) {
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

void Machine::start() {
  if (!this->cap_) {
    throw Exception::ConfigError("no input source is available");
  }

  this->input_  = new Input(this->cap_, this->kernel_->channel());

  pthread_create(&this->kernel_th_, nullptr, Kernel::thread, this->kernel_);
  pthread_create(&this->input_th_,  nullptr, Input::thread,  this->input_);
}

void Machine::join() {
  pthread_join(this->input_th_,  nullptr);
  pthread_join(this->kernel_th_, nullptr);
}

void Machine::shutdown() {
}

bool Machine::on(const std::string& event_name,
                 std::function<void(const Property&)>&& callback) {
  assert(this->kernel_);
  return this->kernel_->on(event_name, callback);
}

uint64_t Machine::recv_pkt() const {
  assert(this->kernel_);
  return this->kernel_->recv_pkt();
}

uint64_t Machine::recv_size() const {
  assert(this->kernel_);
  return this->kernel_->recv_size();
}

}   // namespace pm
