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

#include <pcap.h>

#include "./gtest/gtest.h"
#include "../src/decoder.hpp"
#include "../src/packet.hpp"
#include "../src/packetmachine/property.hpp"

#include "../src/debug.hpp"

namespace module_test {

TEST(Decoder, basic) {
  const std::string fpath = "./test/data1.pcap";
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* pcap = ::pcap_open_offline(fpath.c_str(), errbuf);

  struct pcap_pkthdr* pkthdr;
  const u_char* data;
  int rc;

  pm::Decoder dec;
  pm::Packet pkt;
  pm::Payload pd;
  pm::Property prop(&dec);

  int count_ipv4_saddr = 0;
  int count_mac_saddr = 0;

  while (0 <= (rc = ::pcap_next_ex(pcap, &pkthdr, &data))) {
    if (0 == rc) {
      continue;
    }

    ASSERT_TRUE(pkt.store(data, pkthdr->caplen));
    pkt.set_cap_len(pkthdr->caplen);
    pkt.set_tv(pkthdr->ts);

    pd.reset(&pkt);
    prop.init(&pkt);

    dec.decode(&pd, &prop);

    if ("50:a7:33:0c:5e:4c" == prop.value("Ethernet.src").mac()) {
      count_mac_saddr++;
    }
    // debug(true, "%s", prop.value("Ethernet.src").mac().c_str());

    if ("104.80.179.146" == prop.value("IPv4.src").ip4()) {
      count_ipv4_saddr++;
    }
  }

  EXPECT_EQ(22, count_mac_saddr);
  // EXPECT_EQ(491, count_ipv4_saddr);
}

}   // namespace module_test
