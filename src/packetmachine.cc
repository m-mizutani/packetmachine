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

#include "./packetmachine.hpp"
#include "./capture.hpp"

namespace pm {

Process::Process() {
}

Process::~Process() {
}


Machine::Machine() : cap_(nullptr) {
}

Machine::~Machine() {
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
  this->cap_->start();
}

void Machine::stop() {
  this->cap_->stop();
}

bool Machine::bind(const std::string& event_name, ProcPtr ptr) {
  return false;
}


}   // namespace pm
