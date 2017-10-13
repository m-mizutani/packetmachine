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

#include <iostream>
#include "../src/packetmachine.hpp"

int main(int argc, char* argv[]) {
  pm::Machine m;

  if (argc != 2) {
    std::cout << "usage) pmfile <file_path>" << std::endl;
    return -1;
  }

  auto& ipv4_src = m.lookup_param_key("IPv4.src");
  auto& ipv4_dst = m.lookup_param_key("IPv4.dst");
  auto& tcp_src =  m.lookup_param_key("TCP.src_port");
  auto& tcp_dst =  m.lookup_param_key("TCP.dst_port");
  auto& udp_src =  m.lookup_param_key("UDP.src_port");
  auto& udp_dst =  m.lookup_param_key("UDP.dst_port");
  auto& icmp_type = m.lookup_param_key("ICMP.type");
  auto& icmp_code = m.lookup_param_key("ICMP.code");

  try {
    m.on("TCP", [&](const pm::Property& p) {
        printf("TCP: %s:%u > %s:%u\n",
               p[ipv4_src].repr().c_str(), p[tcp_src].uint(),
               p[ipv4_dst].repr().c_str(), p[tcp_dst].uint());
      });

    m.on("UDP", [&](const pm::Property& p) {
        printf("UDP: %s:%u > %s:%u\n",
               p[ipv4_src].repr().c_str(), p[udp_src].uint(),
               p[ipv4_dst].repr().c_str(), p[udp_dst].uint());
      });

    m.on("ICMP", [&](const pm::Property& p) {
        printf("ICMP: %s > %s (%s:%s)\n",
               p[ipv4_src].repr().c_str(), p[ipv4_dst].repr().c_str(),
               p[icmp_type].repr().c_str(), p[icmp_code].repr().c_str());
      });

    m.add_pcapfile(argv[1]);
    m.loop();
  } catch (pm::Exception::Error &e) {
    std::cerr << "PacketMachine Error: " << e.what() << std::endl;
  }
}
