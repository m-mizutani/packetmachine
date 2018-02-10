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

#ifndef __PACKETMACHINE_CAPTURE_HPP__
#define __PACKETMACHINE_CAPTURE_HPP__

#include <pcap.h>
#include <string>

#include "./packetmachine/common.hpp"

namespace pm {

class Packet;

// Capture is wrapper of capturing packet from device and pcap file.
// In order to abstract data source.

class Capture {
 public:
  enum Result {
    OK,
    NONE,
    ERROR,
    EXIT,
  };

 private:
  std::string error_;
  bool ready_;

 protected:
  void set_error(const std::string& error) {
    this->error_ = error;
  }
  void set_ready(bool ready) {
    this->ready_ = ready;
  }

 public:
  Capture();
  virtual ~Capture();

  virtual Result read(Packet *pkt) = 0;
  virtual const std::string& src_name() const = 0;
  bool ready() const { return this->ready_; }
  const std::string& error() const { return this->error_; }
};

// Device is wrapper of libpcap packet capture because threre is a plan to
// replace libpcap with other packet capture mechanism such as netmap.

class PcapDev : public Capture {
 private:
  std::string dev_name_;
  pcap_t *pd_;

 public:
  explicit PcapDev(const std::string& dev_name);
  ~PcapDev();

  Result read(Packet *pkt);
  const std::string& src_name() const { return this->dev_name_; }
};


class PcapFile : public Capture {
 private:
  std::string file_path_;
  pcap_t *pd_;

 public:
  explicit PcapFile(const std::string& file_path);
  ~PcapFile();

  Result read(Packet *pkt);
  const std::string& src_name() const { return this->file_path_; }
};

}   // namespace pm

#endif   // __PACKETMACHINE_CAPTURE_HPP__
