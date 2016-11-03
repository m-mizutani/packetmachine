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
    std::cout << "usage) pmdump <dev>" << std::endl;
    return -1;
  }

  try {
    m.on("TCP", [&](const pm::Property& p) {
        std::cout << "TCP: " <<
            p.value("IPv4.src") << ":" << p.value("TCP.src_port") << " > " <<
            p.value("IPv4.dst") << ":" << p.value("TCP.dst_port") << std::endl;
      });

    m.on("UDP", [&](const pm::Property& p) {
        std::cout << "UDP: " <<
            p.value("IPv4.src") << ":" << p.value("UDP.src_port") << " > " <<
            p.value("IPv4.dst") << ":" << p.value("UDP.dst_port") << std::endl;
      });

    m.on("ICMP", [&](const pm::Property& p) {
        std::cout << "ICMP: " <<
            p.value("IPv4.src") << " > " << p.value("IPv4.dst") << " " <<
            p.value("ICMP.type") << ":" << p.value("ICMP.code") << std::endl;
      });

    m.add_pcapdev(argv[1]);
    m.loop();
  } catch (pm::Exception::Error &e) {
    std::cerr << "PacketMachine Error: " << e.what() << std::endl;
  }
}
