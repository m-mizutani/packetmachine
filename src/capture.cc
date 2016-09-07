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
#include "./capture.hpp"
#include "./packet.hpp"

#include "./debug.hpp"

namespace pm {

Capture::Capture() : ready_(false) {
}

Capture::~Capture() {
}



Device::Device(const std::string &dev_name) :
    dev_name_(dev_name) {
  // TODO(m-mizutani): implement
}

Device::~Device() {
  // TODO(m-mizutani): implement
}

int Device::read(Packet* pkt) {
  // TODO(m-mizutani): implement
  return 0;
}



PcapFile::PcapFile(const std::string &file_path) :
    file_path_(file_path),
    pd_(nullptr) {
  char errbuf[PCAP_ERRBUF_SIZE];
  this->pd_ = ::pcap_open_offline(this->file_path_.c_str(), errbuf);

  if (this->pd_ == nullptr) {
    this->set_error(errbuf);
  } else {
    this->set_ready(true);
  }
}

PcapFile::~PcapFile() {
  if (this->pd_) {
    pcap_close(this->pd_);
  }
}

int PcapFile::read(Packet* pkt) {
  struct pcap_pkthdr* pkthdr;
  const u_char* data;

  if (!this->ready()) {
    this->set_error("pcap is not ready");
    return -1;
  }

  int rc = ::pcap_next_ex(this->pd_, &pkthdr, &data);

  if (rc == 1) {
    // the packet was read without problems.
    if (pkt->store(data, pkthdr->caplen) == false) {
      this->set_error("memory allocation error");
      return -1;
    }

    pkt->set_cap_len(pkthdr->caplen);
    pkt->set_tv(pkthdr->ts);

    return 1;
  } else if (rc == 0) {
    // packets are being read from a live capture and the timeout expired.
    return 0;
  } else if (rc == -1) {
    // an error occurred.
    char* e = pcap_geterr(this->pd_);
    printf("%d, %s\n", rc, e);
    this->set_error(e);
    this->set_ready(false);
    return -1;
  } else if (rc == -2) {
    // exit normaly.
    this->set_ready(false);
    return -1;
  } else {
    assert(0);   // invalid return code from pcap_next_ex.
    return -3;
  }
}


}  // namespace pm
