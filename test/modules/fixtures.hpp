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

#ifndef __PACKETMACHINE_TEST_MODULES_FIXTURES_HPP__
#define __PACKETMACHINE_TEST_MODULES_FIXTURES_HPP__

#include <string>
#include <pcap.h>
#include "../gtest/gtest.h"
#include "../src/module.hpp"
#include "../src/decoder.hpp"
#include "../src/packet.hpp"
#include "../src/packetmachine/property.hpp"


class ModuleTesterData : public ::testing::Test {
 public:
  pm::Decoder dec;
  pm::Packet pkt;
  pm::Payload pd;
  pm::Property *prop_;
  pcap_t* pcap;

  virtual const std::string fpath() const = 0;

  virtual void SetUp() {
    const std::string fpath = this->fpath();
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap = ::pcap_open_offline(fpath.c_str(), errbuf);
    prop_ = new pm::Property(&dec);
  }

  virtual void TearDown() {
    delete prop_;
  }

  const pm::Property* get_property(size_t skip = 0) {
    struct pcap_pkthdr* pkthdr;
    const u_char* data;
    int rc;
    size_t idx = 0;
    while (0 <= (rc = ::pcap_next_ex(pcap, &pkthdr, &data))) {
      if (idx >= skip) {
        break;
      }
      idx++;
    }

    if (rc < 0) {
      return nullptr;
    } else {
      EXPECT_TRUE(pkt.store(data, pkthdr->caplen));
      pkt.set_cap_len(pkthdr->caplen);
      pkt.set_tv(pkthdr->ts);

      pd.reset(&pkt);
      prop_->init(&pkt);

      dec.decode(&pd, prop_);
      return prop_;
    }
  }
};

class ModuleTesterData1 : public ModuleTesterData {
  virtual const std::string fpath() const {
    return "./test/data1.pcap";
  }
};

class ModuleTesterData2 : public ModuleTesterData {
  virtual const std::string fpath() const {
    return "./test/data2.pcap";
  }
};

class ModuleTesterData3 : public ModuleTesterData {
  virtual const std::string fpath() const {
    return "./test/data3.pcap";
  }
};

#endif    // __PACKETMACHINE_TEST_MODULES_FIXTURES_HPP__
