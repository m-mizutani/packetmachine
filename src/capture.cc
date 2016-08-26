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

#include "./capture.hpp"

namespace netdec {


bool Capture::start() {
  // TODO(m-mizutani): implement
  return false;
}

bool Capture::stop() {
  // TODO(m-mizutani): implement
  return false;
}


Device::Device(const std::string &dev_name) :
    dev_name_(dev_name) {
  // TODO(m-mizutani): implement
}

Device::~Device() {
  // TODO(m-mizutani): implement
}

int Device::read(byte_t *buf, int buf_len, int *cap_len) {
  // TODO(m-mizutani): implement
  return 0;
}

bool Device::ready() const {
  // TODO(m-mizutani): to be fix
  return false;
}



PcapFile::PcapFile(const std::string &file_path) :
    file_path_(file_path),
    pd_(nullptr) {
}

PcapFile::~PcapFile() {
  // TODO(m-mizutani): implement
}

int PcapFile::read(byte_t *buf, int buf_len, int *cap_len) {
  // TODO(m-mizutani): implement
  return 0;
}

bool PcapFile::ready() const {
  // TODO(m-mizutani): to be fix
  return false;
}


}  // namespace netdec
